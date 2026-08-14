#include "currencyrunner_config.h"
#include "currencyconfig_keys.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>

#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS(CurrencyRunnerConfig)

CurrencyRunnerConfig::CurrencyRunnerConfig(QObject *parent)
    : KCModule(parent)
{
    auto *layout = new QVBoxLayout(widget());
    layout->setContentsMargins(0, 0, 0, 0);

    auto *group = new QGroupBox(QStringLiteral("Number Formatting"), widget());
    auto *form = new QFormLayout(group);

    m_groupDigits = new QCheckBox(QStringLiteral("Group large numbers with separators (e.g. 1,000,000)"), group);
    form->addRow(m_groupDigits);

    m_decimalPlaces = new QSpinBox(group);
    m_decimalPlaces->setRange(0, 6);
    form->addRow(QStringLiteral("Decimal places:"), m_decimalPlaces);

    layout->addWidget(group);
    layout->addStretch();

    connect(m_groupDigits, &QCheckBox::checkStateChanged, this, &CurrencyRunnerConfig::markAsChanged);
    connect(m_decimalPlaces, &QSpinBox::valueChanged, this, &CurrencyRunnerConfig::markAsChanged);
}

void CurrencyRunnerConfig::load()
{
    KCModule::load();

    const KConfigGroup grp = KSharedConfig::openConfig(u"krunnerrc"_s)->group(u"Runners"_s).group(QStringLiteral(KRUNNER_PLUGIN_NAME));
    m_groupDigits->setChecked(grp.readEntry(CONFIG_GROUP_DIGITS, DEFAULT_GROUP_DIGITS));
    m_decimalPlaces->setValue(grp.readEntry(CONFIG_DECIMAL_PLACES, DEFAULT_DECIMAL_PLACES));
}

void CurrencyRunnerConfig::save()
{
    KCModule::save();

    KConfigGroup grp = KSharedConfig::openConfig(u"krunnerrc"_s)->group(u"Runners"_s).group(QStringLiteral(KRUNNER_PLUGIN_NAME));
    grp.writeEntry(CONFIG_GROUP_DIGITS, m_groupDigits->isChecked());
    grp.writeEntry(CONFIG_DECIMAL_PLACES, m_decimalPlaces->value());
    grp.sync();
}

void CurrencyRunnerConfig::defaults()
{
    KCModule::defaults();

    m_groupDigits->setChecked(DEFAULT_GROUP_DIGITS);
    m_decimalPlaces->setValue(DEFAULT_DECIMAL_PLACES);

    setNeedsSave(true);
}

#include "currencyrunner_config.moc"
