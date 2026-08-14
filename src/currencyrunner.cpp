#include "currencyrunner.h"
#include "currencyconfig_keys.h"
#include "currencydata.h"

#include <KConfigGroup>
#include <KRunner/QueryMatch>
#include <KRunner/RunnerContext>

#include <QClipboard>
#include <QGuiApplication>
#include <QRegularExpression>

K_PLUGIN_CLASS_WITH_JSON(CurrencyRunner, "kurrencykonverter.json")

namespace
{
const QString ConfigLastCurrencyKey = QStringLiteral("LastCurrency");

// Returns the multiplier for a "k"/"m"/"b" or "thousand"/"million"/"billion"
// suffix, or 1.0 if the token isn't a recognized multiplier word.
double suffixMultiplier(const QString &token)
{
    const QString t = token.trimmed().toLower();
    if (t == QLatin1String("k") || t == QLatin1String("thousand")) {
        return 1e3;
    }
    if (t == QLatin1String("m") || t == QLatin1String("million")) {
        return 1e6;
    }
    if (t == QLatin1String("b") || t == QLatin1String("billion")) {
        return 1e9;
    }
    return 1.0;
}
}

CurrencyRunner::CurrencyRunner(QObject *parent, const KPluginMetaData &data)
    : KRunner::AbstractRunner(parent, data)
    , m_rates(this)
{
    setMinLetterCount(3);
    addSyntax(QStringLiteral("23 dollars inr"), QStringLiteral("Convert 23 US dollars to Indian rupees"));
    addSyntax(QStringLiteral("50 eur"), QStringLiteral("Show 50 euros converted into several common currencies"));
    addSyntax(QStringLiteral("1.5k usd inr"), QStringLiteral("Convert 1,500 US dollars to Indian rupees"));
    addSyntax(QStringLiteral("5 million eur"), QStringLiteral("Convert 5,000,000 euros into several common currencies"));
}

void CurrencyRunner::init()
{
    m_rates.ensureLoaded();
}

QString CurrencyRunner::formatAmount(double value) const
{
    const KConfigGroup cfg = config();
    const int decimals = qBound(0, cfg.readEntry(CONFIG_DECIMAL_PLACES, DEFAULT_DECIMAL_PLACES), 6);
    const bool grouped = cfg.readEntry(CONFIG_GROUP_DIGITS, DEFAULT_GROUP_DIGITS);

    const QString formatted = QString::number(value, 'f', decimals);
    if (!grouped) {
        return formatted;
    }

    const int dotIndex = formatted.indexOf(QLatin1Char('.'));
    QString intPart = dotIndex >= 0 ? formatted.left(dotIndex) : formatted;
    const QString fracPart = dotIndex >= 0 ? formatted.mid(dotIndex) : QString();

    const bool negative = intPart.startsWith(QLatin1Char('-'));
    if (negative) {
        intPart.remove(0, 1);
    }
    for (int i = intPart.length() - 3; i > 0; i -= 3) {
        intPart.insert(i, QLatin1Char(','));
    }
    if (negative) {
        intPart.prepend(QLatin1Char('-'));
    }
    return intPart + fracPart;
}

KRunner::QueryMatch CurrencyRunner::makeMatch(double amount, const QString &fromCode, const QString &toCode, double result, bool preferred)
{
    KRunner::QueryMatch match(this);
    match.setId(fromCode + QLatin1Char('_') + toCode);
    match.setText(QStringLiteral("%1 %2 = %3 %4").arg(formatAmount(amount), fromCode, formatAmount(result), toCode));
    match.setSubtext(preferred ? QStringLiteral("Currency conversion · previously used") : QStringLiteral("Currency conversion"));
    match.setIconName(QStringLiteral("view-currency-list"));
    match.setCategoryRelevance(KRunner::QueryMatch::CategoryRelevance::High);
    match.setRelevance(preferred ? 1.0 : 0.9);
    match.setData(toCode);
    return match;
}

void CurrencyRunner::match(KRunner::RunnerContext &context)
{
    const QString query = context.query().trimmed();

    // number (optionally grouped with commas every 2 or 3 digits, e.g.
    // "1,234.56" or the Indian "10,00,000"), optional "k"/"m"/"b" glued
    // directly to the number or a "thousand"/"million"/"billion" word,
    // source currency word, optional ("to"/"in") + target currency word.
    // Matches e.g. "23dollars inr", "23 dollars in usd", "100 usd to eur",
    // "50 eur", "1.5k usd inr", "5 million eur".
    static const QRegularExpression re(
        QStringLiteral(
            R"(^([0-9][0-9,]*(?:\.[0-9]+)?)([kKmMbB](?![a-zA-Z]))?\s*(?:(thousand|million|billion)\s+)?([a-zA-Z€£¥₹$]+)\s*(?:(?:to|in)\s+)?([a-zA-Z]{2,10})?$)"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch m = re.match(query);
    if (!m.hasMatch() || !m_rates.isAvailable()) {
        return;
    }

    QString numberStr = m.captured(1);
    numberStr.remove(QLatin1Char(','));
    bool ok = false;
    double amount = numberStr.toDouble(&ok);
    if (!ok) {
        return;
    }

    double multiplier = 1.0;
    const QString gluedSuffix = m.captured(2);
    const QString wordSuffix = m.captured(3);
    if (!gluedSuffix.isEmpty()) {
        multiplier = suffixMultiplier(gluedSuffix);
    } else if (!wordSuffix.isEmpty()) {
        multiplier = suffixMultiplier(wordSuffix);
    }

    const QString fromToken = m.captured(4);
    QString fromCode = CurrencyData::resolveCurrency(fromToken);
    if (fromCode.isEmpty() && multiplier == 1.0 && fromToken.length() > 1) {
        // No standalone suffix matched (e.g. "1k usd"), but the currency
        // word itself might start with a glued multiplier, e.g. "1kusd".
        const QChar prefix = fromToken.at(0).toLower();
        if (prefix == QLatin1Char('k') || prefix == QLatin1Char('m') || prefix == QLatin1Char('b')) {
            const QString restCode = CurrencyData::resolveCurrency(fromToken.mid(1));
            if (!restCode.isEmpty()) {
                fromCode = restCode;
                multiplier = suffixMultiplier(QString(prefix));
            }
        }
    }
    if (fromCode.isEmpty() || !m_rates.isKnownCurrency(fromCode)) {
        return;
    }

    amount *= multiplier;

    const QString explicitTarget = m.captured(5);

    if (!explicitTarget.isEmpty()) {
        const QString toCode = CurrencyData::resolveCurrency(explicitTarget);
        if (toCode.isEmpty() || toCode == fromCode || !m_rates.isKnownCurrency(toCode)) {
            return;
        }
        double result = 0.0;
        if (!m_rates.convert(fromCode, toCode, amount, &result)) {
            return;
        }
        context.addMatch(makeMatch(amount, fromCode, toCode, result, /*preferred=*/false));
        return;
    }

    // No explicit target currency: show a shortlist, with any previously used
    // target currency (for this source query) shown first.
    QStringList targets = CurrencyData::defaultCurrencies();
    targets.removeAll(fromCode);

    const QString preferred = config().readEntry(ConfigLastCurrencyKey, QString());
    if (!preferred.isEmpty() && preferred != fromCode && m_rates.isKnownCurrency(preferred)) {
        targets.removeAll(preferred);
        targets.prepend(preferred);
    }

    QList<KRunner::QueryMatch> matches;
    matches.reserve(targets.size());
    for (const QString &toCode : std::as_const(targets)) {
        double result = 0.0;
        if (!m_rates.convert(fromCode, toCode, amount, &result)) {
            continue;
        }
        matches << makeMatch(amount, fromCode, toCode, result, toCode == preferred);
    }
    context.addMatches(matches);
}

void CurrencyRunner::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context)

    const QString toCode = match.data().toString();
    if (!toCode.isEmpty()) {
        KConfigGroup cfg = config();
        cfg.writeEntry(ConfigLastCurrencyKey, toCode);
        cfg.sync();
    }

    if (qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
        if (auto *clipboard = QGuiApplication::clipboard()) {
            const int eq = match.text().indexOf(QLatin1Char('='));
            const QString converted = eq >= 0 ? match.text().mid(eq + 1).trimmed() : match.text();
            clipboard->setText(converted);
        }
    }
}

#include "currencyrunner.moc"
