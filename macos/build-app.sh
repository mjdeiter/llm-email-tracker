#!/usr/bin/env bash
# Build "Email Tracker.app" for macOS from source.
#
# Requires Homebrew with cmake and qt:  brew install cmake qt
# Usage:  macos/build-app.sh [output-dir]      (default output: ./dist)
#
# Builds out-of-tree in build-macos/ so the Linux binary tracked in build/ is untouched.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${1:-$ROOT/dist}"
BUILD="$ROOT/build-macos"
APP="$OUT/Email Tracker.app"

VERSION="$(sed -n 's/^## \[\([0-9][0-9.]*\)\].*/\1/p' "$ROOT/CHANGELOG.md" | head -1)"
if [ -z "$VERSION" ]; then
    echo "Could not read version from CHANGELOG.md" >&2
    exit 1
fi

cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build "$BUILD" --parallel

rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS"
cp "$BUILD/EmailTracker" "$APP/Contents/MacOS/EmailTracker"

cat > "$APP/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key><string>Email Tracker</string>
  <key>CFBundleDisplayName</key><string>Email Tracker</string>
  <key>CFBundleIdentifier</key><string>com.mjdeiter.emailtracker</string>
  <key>CFBundleExecutable</key><string>EmailTracker</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleShortVersionString</key><string>${VERSION}</string>
  <key>CFBundleVersion</key><string>${VERSION}</string>
  <key>NSHighResolutionCapable</key><true/>
  <key>LSMinimumSystemVersion</key><string>12.0</string>
</dict>
</plist>
PLIST

# Ad-hoc sign (required for arm64 binaries to launch)
codesign --force --deep -s - -i com.mjdeiter.emailtracker "$APP"

echo "Built: $APP (v$VERSION)"
