# KurrencyKonverter

A [KRunner](https://userbase.kde.org/KRunner) plugin for KDE Plasma that converts currencies instantly from your launcher. Type `23 dollars inr` or `50 eur` for a quick conversion or a shortlist of common currencies — no browser, no separate app. Remembers your last-used currency, shows it first, and copies results to your clipboard. Live exchange rates, cached locally.

## Screenshots

Give a target currency and get a single result:

![Single conversion](screenshots/single-conversion.png)

Leave the target out and get a shortlist of common currencies — whichever one you picked last is remembered and shown first:

![Shortlist with a remembered currency](screenshots/shortlist.png)

## Features

- **Natural queries** — `23 dollars inr`, `23dollars inr`, `100 usd to eur`, `50 eur` all work.
- **Shortlist mode** — no target currency? See it converted to USD, EUR, GBP, INR, JPY, AUD, CAD, CHF, CNY and SGD at once.
- **Remembers your last pick** — select a currency from the shortlist once, and it's shown first (labeled "previously used") next time.
- **Copy to clipboard** — selecting a result copies the converted value.
- **Live exchange rates** — fetched from a free, keyless API and cached locally, so lookups are instant and still work offline once cached.

## Requirements

- KDE Plasma 6 with KRunner (KF6)
- CMake ≥ 3.16, a C++17 compiler
- Qt6 (Core, Gui, Network) and KDE Frameworks 6 (Runner, CoreAddons, Config)

On Arch Linux:

```sh
sudo pacman -S cmake qt6-base kcoreaddons krunner kconfig
```

(Package names will differ slightly on other distros — you need the `-dev`/headers packages for Qt6 and the KF6 Runner/CoreAddons/Config frameworks.)

## Install

```sh
git clone https://github.com/Vinayak-09/kurrencykonverter.git
cd kurrencykonverter
./install.sh
```

`install.sh` configures, builds, and installs the plugin system-wide (via `sudo`, falling back to `pkexec`), then restarts KRunner. Open KRunner (<kbd>Alt</kbd>+<kbd>Space</kbd> or <kbd>Alt</kbd>+<kbd>F2</kbd>) and try `23 dollars inr`.

### Manual build

```sh
cmake -B build -S .
cmake --build build
sudo cmake --install build
kquitapp6 krunner   # restart so it picks up the new plugin
```

## Usage

| Query               | Result                                          |
|---------------------|--------------------------------------------------|
| `23 dollars inr`     | Converts 23 USD to INR                          |
| `23dollars inr`      | Same — spacing doesn't matter                   |
| `100 usd to eur`     | Converts 100 USD to EUR                         |
| `50 eur`             | Shows 50 EUR converted to a shortlist of common currencies |

Supported currency words include full names, plurals, common slang, and symbols (`dollar`/`dollars`/`usd`/`$`, `euro`/`eur`/`€`, `rupee`/`inr`/`₹`, `pound`/`gbp`/`£`, `yen`/`jpy`/`¥`, and more — see `src/currencydata.h`), plus any raw 3-letter ISO 4217 code.

## Uninstall

```sh
./uninstall.sh
```

Removes the installed plugin and, if you confirm, the cached exchange rates and remembered currency too.

## How it works

- Exchange rates come from [Frankfurter](https://frankfurter.dev) (ECB reference rates), a free API that needs no key. They update once per business day, not intraday.
- Rates are cached at `~/.cache/kurrencykonverter/rates.json`; the plugin fetches fresh data in the background when the cache is older than 6 hours, and blocks briefly on the very first run if there's no cache yet.
- The last currency you picked from the shortlist is stored per-user in `krunnerrc` under `[Runners][kurrencykonverter]`.

## License

[GPL-3.0-or-later](LICENSE)

## Credits

- Exchange rate data from [Frankfurter](https://frankfurter.dev), based on European Central Bank reference rates.
