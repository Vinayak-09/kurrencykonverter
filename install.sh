#!/usr/bin/env bash
# Builds and installs the KurrencyKonverter KRunner plugin system-wide.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "==> Configuring..."
cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR"

echo "==> Building..."
cmake --build "$BUILD_DIR"

echo "==> Installing (needs root)..."
if [ -t 0 ] && command -v sudo >/dev/null 2>&1; then
    # Interactive terminal: sudo can prompt for a password normally.
    sudo cmake --install "$BUILD_DIR"
elif command -v pkexec >/dev/null 2>&1; then
    # No terminal to prompt in (e.g. launched from a menu): use the graphical
    # polkit prompt instead.
    pkexec cmake --install "$BUILD_DIR"
elif command -v sudo >/dev/null 2>&1; then
    sudo cmake --install "$BUILD_DIR"
else
    echo "No sudo or pkexec found. Run as root manually:" >&2
    echo "  cmake --install \"$BUILD_DIR\"" >&2
    exit 1
fi

echo "==> Restarting krunner..."
kquitapp6 krunner >/dev/null 2>&1 || killall krunner >/dev/null 2>&1 || true

echo
echo "Done. Open KRunner (Alt+Space / Alt+F2) and try: 23 dollars inr"
