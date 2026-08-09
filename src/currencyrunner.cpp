#include "currencyrunner.h"
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
}

CurrencyRunner::CurrencyRunner(QObject *parent, const KPluginMetaData &data)
    : KRunner::AbstractRunner(parent, data)
    , m_rates(this)
{
    setMinLetterCount(3);
    addSyntax(QStringLiteral("23 dollars inr"), QStringLiteral("Convert 23 US dollars to Indian rupees"));
    addSyntax(QStringLiteral("50 eur"), QStringLiteral("Show 50 euros converted into several common currencies"));
}

void CurrencyRunner::init()
{
    m_rates.ensureLoaded();
}

QString CurrencyRunner::formatAmount(double value)
{
    return QString::number(value, 'f', 2);
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

    // number, source currency word, optional ("to"/"in") + target currency word.
    // Matches e.g. "23dollars inr", "23 dollars in usd", "100 usd to eur", "50 eur".
    static const QRegularExpression re(
        QStringLiteral(R"(^([0-9]+(?:[.,][0-9]+)?)\s*([a-zA-Z€£¥₹$]+)\s*(?:(?:to|in)\s+)?([a-zA-Z]{2,10})?$)"));

    const QRegularExpressionMatch m = re.match(query);
    if (!m.hasMatch() || !m_rates.isAvailable()) {
        return;
    }

    QString numberStr = m.captured(1);
    numberStr.remove(QLatin1Char(','));
    bool ok = false;
    const double amount = numberStr.toDouble(&ok);
    if (!ok) {
        return;
    }

    const QString fromCode = CurrencyData::resolveCurrency(m.captured(2));
    if (fromCode.isEmpty() || !m_rates.isKnownCurrency(fromCode)) {
        return;
    }

    const QString explicitTarget = m.captured(3);

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
