#!/usr/bin/env bash
# Removes the KurrencyKonverter KRunner plugin.
set -euo pipefail

QT_PLUGIN_DIR="$(qmake6 -query QT_INSTALL_PLUGINS 2>/dev/null || qmake -query QT_INSTALL_PLUGINS)"
PLUGIN_FILE="$QT_PLUGIN_DIR/kf6/krunner/kurrencykonverter.so"

if [[ -f "$PLUGIN_FILE" ]]; then
    echo "==> Removing $PLUGIN_FILE (needs root)..."
    if [ -t 0 ] && command -v sudo >/dev/null 2>&1; then
        sudo rm -f "$PLUGIN_FILE"
    elif command -v pkexec >/dev/null 2>&1; then
        pkexec rm -f "$PLUGIN_FILE"
    elif command -v sudo >/dev/null 2>&1; then
        sudo rm -f "$PLUGIN_FILE"
    else
        echo "No sudo or pkexec found. Run as root manually:" >&2
        echo "  rm -f \"$PLUGIN_FILE\"" >&2
        exit 1
    fi
else
    echo "Plugin not found at $PLUGIN_FILE (already uninstalled?)"
fi

echo "==> Restarting krunner..."
kquitapp6 krunner >/dev/null 2>&1 || killall krunner >/dev/null 2>&1 || true

reply="n"
if [ -t 0 ]; then
    read -r -p "Also remove the cached exchange rates and remembered currency? [y/N] " reply
fi
if [[ "$reply" =~ ^[Yy]$ ]]; then
    rm -rf "$HOME/.cache/kurrencykonverter"
    kwriteconfig6 --file krunnerrc --group Runners --group kurrencykonverter --key LastCurrency --delete 2>/dev/null || true
    echo "Removed cached rates and remembered currency."
fi

echo
echo "Done."
