#pragma once

#include <KCModule>

class QCheckBox;
class QSpinBox;

// System Settings page for the currency runner (Search > Plugins > Currency
// Converter > Configure...). Registered via kurrencykonverter.json's
// "X-KDE-ConfigModule", and reads/writes the same krunnerrc group the
// runner itself uses.
class CurrencyRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit CurrencyRunnerConfig(QObject *parent);

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    QCheckBox *m_groupDigits;
    QSpinBox *m_decimalPlaces;
};
