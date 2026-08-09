#pragma once

#include <KRunner/AbstractRunner>

#include "ratesprovider.h"

class CurrencyRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    CurrencyRunner(QObject *parent, const KPluginMetaData &data);

    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

protected:
    void init() override;

private:
    KRunner::QueryMatch makeMatch(double amount, const QString &fromCode, const QString &toCode, double result, bool preferred);
    static QString formatAmount(double value);

    RatesProvider m_rates;
};
