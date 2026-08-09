#include "ratesprovider.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

namespace
{
// Frankfurter (ECB reference rates) - free, no API key required.
const auto RatesApiUrl = QStringLiteral("https://api.frankfurter.dev/v1/latest?from=EUR");
constexpr int CacheMaxAgeSecs = 6 * 60 * 60; // refresh in the background after 6h

QNetworkRequest ratesRequest()
{
    QNetworkRequest request{QUrl(RatesApiUrl)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return request;
}
}

RatesProvider::RatesProvider(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

QString RatesProvider::cacheFilePath() const
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/kurrencykonverter");
    QDir().mkpath(dir);
    return dir + QStringLiteral("/rates.json");
}

void RatesProvider::ensureLoaded()
{
    if (!m_rates.isEmpty()) {
        if (m_lastUpdated.secsTo(QDateTime::currentDateTimeUtc()) > CacheMaxAgeSecs) {
            fetchAsync();
        }
        return;
    }

    if (loadFromDisk()) {
        if (m_lastUpdated.secsTo(QDateTime::currentDateTimeUtc()) > CacheMaxAgeSecs) {
            fetchAsync();
        }
        return;
    }

    // No cache at all yet: block briefly so the very first query already
    // has data. Safe here because AbstractRunner::init() runs off the GUI thread.
    fetchBlocking(4000);
}

bool RatesProvider::isAvailable() const
{
    return !m_rates.isEmpty();
}

bool RatesProvider::isKnownCurrency(const QString &isoCode) const
{
    return isoCode == QLatin1String("EUR") || m_rates.contains(isoCode);
}

bool RatesProvider::convert(const QString &from, const QString &to, double amount, double *result) const
{
    if (!result || !isKnownCurrency(from) || !isKnownCurrency(to)) {
        return false;
    }

    const double fromRate = (from == QLatin1String("EUR")) ? 1.0 : m_rates.value(from);
    const double toRate = (to == QLatin1String("EUR")) ? 1.0 : m_rates.value(to);
    if (fromRate <= 0.0 || toRate <= 0.0) {
        return false;
    }

    *result = amount / fromRate * toRate;
    return true;
}

bool RatesProvider::loadFromDisk()
{
    QFile f(cacheFilePath());
    if (!f.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QByteArray data = f.readAll();
    f.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonObject rates = root.value(QStringLiteral("rates")).toObject();
    if (rates.isEmpty()) {
        return false;
    }

    QHash<QString, double> parsed;
    for (auto it = rates.constBegin(); it != rates.constEnd(); ++it) {
        parsed.insert(it.key(), it.value().toDouble());
    }

    m_rates = parsed;
    m_lastUpdated = QDateTime::fromString(root.value(QStringLiteral("cachedAt")).toString(), Qt::ISODate);
    return !m_rates.isEmpty();
}

void RatesProvider::saveToDisk() const
{
    QJsonObject rates;
    for (auto it = m_rates.constBegin(); it != m_rates.constEnd(); ++it) {
        rates.insert(it.key(), it.value());
    }

    QJsonObject root;
    root.insert(QStringLiteral("rates"), rates);
    root.insert(QStringLiteral("cachedAt"), m_lastUpdated.toString(Qt::ISODate));

    QFile f(cacheFilePath());
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }
}

void RatesProvider::applyRatesDocument(const QByteArray &data)
{
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject rates = doc.object().value(QStringLiteral("rates")).toObject();
    if (rates.isEmpty()) {
        return;
    }

    QHash<QString, double> parsed;
    for (auto it = rates.constBegin(); it != rates.constEnd(); ++it) {
        parsed.insert(it.key(), it.value().toDouble());
    }

    m_rates = parsed;
    m_lastUpdated = QDateTime::currentDateTimeUtc();
    saveToDisk();
}

void RatesProvider::fetchBlocking(int timeoutMs)
{
    QNetworkReply *reply = m_manager->get(ratesRequest());

    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timeoutTimer.start(timeoutMs);
    loop.exec();

    if (reply->isFinished() && reply->error() == QNetworkReply::NoError) {
        applyRatesDocument(reply->readAll());
    } else {
        reply->abort();
    }
    reply->deleteLater();
}

void RatesProvider::fetchAsync()
{
    if (m_fetchInProgress) {
        return;
    }
    m_fetchInProgress = true;

    QNetworkReply *reply = m_manager->get(ratesRequest());
    connect(reply, &QNetworkReply::finished, this, &RatesProvider::onReplyFinished);
}

void RatesProvider::onReplyFinished()
{
    m_fetchInProgress = false;

    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        applyRatesDocument(reply->readAll());
    }
    reply->deleteLater();
}
