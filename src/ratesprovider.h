#pragma once

#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

// Fetches and caches currency exchange rates (base currency: EUR) from a
// free, keyless web API, with an on-disk cache so most queries never need
// the network.
class RatesProvider : public QObject
{
    Q_OBJECT

public:
    explicit RatesProvider(QObject *parent = nullptr);

    // Makes sure rates are available for querying. On the very first run
    // (no cache on disk yet) this blocks briefly on a network request; this
    // is safe because KRunner runs each plugin's init() off the GUI thread.
    // Otherwise it returns immediately, refreshing stale data in the background.
    void ensureLoaded();

    bool isAvailable() const;
    bool isKnownCurrency(const QString &isoCode) const;

    // Converts `amount` from `from` to `to`. Returns false if either currency
    // is unknown.
    bool convert(const QString &from, const QString &to, double amount, double *result) const;

private Q_SLOTS:
    void onReplyFinished();

private:
    QString cacheFilePath() const;
    bool loadFromDisk();
    void saveToDisk() const;
    void applyRatesDocument(const QByteArray &data);
    void fetchBlocking(int timeoutMs);
    void fetchAsync();

    QNetworkAccessManager *m_manager;
    QHash<QString, double> m_rates; // ISO code -> value of 1 EUR in that currency
    QDateTime m_lastUpdated;
    bool m_fetchInProgress = false;
};
