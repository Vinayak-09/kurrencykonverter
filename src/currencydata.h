#pragma once

#include <QHash>
#include <QString>
#include <QStringList>

// Static data and helpers for recognizing currencies from free-form text.
namespace CurrencyData
{

// Maps lower-case words/symbols to ISO 4217 currency codes.
inline const QHash<QString, QString> &aliasMap()
{
    static const QHash<QString, QString> map = {
        {QStringLiteral("usd"), QStringLiteral("USD")},
        {QStringLiteral("dollar"), QStringLiteral("USD")},
        {QStringLiteral("dollars"), QStringLiteral("USD")},
        {QStringLiteral("buck"), QStringLiteral("USD")},
        {QStringLiteral("bucks"), QStringLiteral("USD")},
        {QStringLiteral("$"), QStringLiteral("USD")},

        {QStringLiteral("eur"), QStringLiteral("EUR")},
        {QStringLiteral("euro"), QStringLiteral("EUR")},
        {QStringLiteral("euros"), QStringLiteral("EUR")},
        {QStringLiteral("€"), QStringLiteral("EUR")},

        {QStringLiteral("gbp"), QStringLiteral("GBP")},
        {QStringLiteral("pound"), QStringLiteral("GBP")},
        {QStringLiteral("pounds"), QStringLiteral("GBP")},
        {QStringLiteral("quid"), QStringLiteral("GBP")},
        {QStringLiteral("£"), QStringLiteral("GBP")},

        {QStringLiteral("inr"), QStringLiteral("INR")},
        {QStringLiteral("rupee"), QStringLiteral("INR")},
        {QStringLiteral("rupees"), QStringLiteral("INR")},
        {QStringLiteral("rs"), QStringLiteral("INR")},
        {QStringLiteral("₹"), QStringLiteral("INR")},

        {QStringLiteral("jpy"), QStringLiteral("JPY")},
        {QStringLiteral("yen"), QStringLiteral("JPY")},
        {QStringLiteral("¥"), QStringLiteral("JPY")},

        {QStringLiteral("aud"), QStringLiteral("AUD")},
        {QStringLiteral("cad"), QStringLiteral("CAD")},

        {QStringLiteral("chf"), QStringLiteral("CHF")},
        {QStringLiteral("franc"), QStringLiteral("CHF")},
        {QStringLiteral("francs"), QStringLiteral("CHF")},

        {QStringLiteral("cny"), QStringLiteral("CNY")},
        {QStringLiteral("yuan"), QStringLiteral("CNY")},
        {QStringLiteral("rmb"), QStringLiteral("CNY")},

        {QStringLiteral("krw"), QStringLiteral("KRW")},
        {QStringLiteral("won"), QStringLiteral("KRW")},

        {QStringLiteral("nzd"), QStringLiteral("NZD")},
        {QStringLiteral("sgd"), QStringLiteral("SGD")},
        {QStringLiteral("hkd"), QStringLiteral("HKD")},
        {QStringLiteral("zar"), QStringLiteral("ZAR")},

        {QStringLiteral("sek"), QStringLiteral("SEK")},
        {QStringLiteral("nok"), QStringLiteral("NOK")},
        {QStringLiteral("dkk"), QStringLiteral("DKK")},

        {QStringLiteral("mxn"), QStringLiteral("MXN")},
        {QStringLiteral("brl"), QStringLiteral("BRL")},
        {QStringLiteral("real"), QStringLiteral("BRL")},
        {QStringLiteral("reais"), QStringLiteral("BRL")},

        {QStringLiteral("try"), QStringLiteral("TRY")},
        {QStringLiteral("lira"), QStringLiteral("TRY")},

        {QStringLiteral("pln"), QStringLiteral("PLN")},
        {QStringLiteral("thb"), QStringLiteral("THB")},
        {QStringLiteral("baht"), QStringLiteral("THB")},
        {QStringLiteral("idr"), QStringLiteral("IDR")},
        {QStringLiteral("ils"), QStringLiteral("ILS")},
        {QStringLiteral("php"), QStringLiteral("PHP")},
        {QStringLiteral("myr"), QStringLiteral("MYR")},
        {QStringLiteral("czk"), QStringLiteral("CZK")},
        {QStringLiteral("huf"), QStringLiteral("HUF")},
        {QStringLiteral("ron"), QStringLiteral("RON")},
        {QStringLiteral("bgn"), QStringLiteral("BGN")},
        {QStringLiteral("isk"), QStringLiteral("ISK")},
    };
    return map;
}

// A sensible shortlist shown when the user doesn't specify a target currency.
inline const QStringList &defaultCurrencies()
{
    static const QStringList list = {
        QStringLiteral("USD"),
        QStringLiteral("EUR"),
        QStringLiteral("GBP"),
        QStringLiteral("INR"),
        QStringLiteral("JPY"),
        QStringLiteral("AUD"),
        QStringLiteral("CAD"),
        QStringLiteral("CHF"),
        QStringLiteral("CNY"),
        QStringLiteral("SGD"),
    };
    return list;
}

// Resolves a free-form word (alias, symbol, or ISO code) to an ISO 4217 code.
// Does not validate that the code is actually known to the rates provider.
inline QString resolveCurrency(const QString &token)
{
    const QString t = token.trimmed().toLower();
    if (t.isEmpty()) {
        return QString();
    }

    const auto &aliases = aliasMap();
    const auto it = aliases.constFind(t);
    if (it != aliases.constEnd()) {
        return it.value();
    }

    // Fall back to treating a 3-letter token as a literal ISO 4217 code.
    if (t.length() == 3) {
        return t.toUpper();
    }

    return QString();
}

}
