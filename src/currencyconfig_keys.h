#pragma once

// Config keys shared between CurrencyRunner and CurrencyRunnerConfig, stored
// under krunnerrc's [Runners][kurrencykonverter] group.

static const char CONFIG_GROUP_DIGITS[] = "GroupDigits";
static const char CONFIG_DECIMAL_PLACES[] = "DecimalPlaces";

inline constexpr bool DEFAULT_GROUP_DIGITS = true;
inline constexpr int DEFAULT_DECIMAL_PLACES = 2;
