# Changelog

All notable changes to LLM Email Tracker are documented here.

## [1.1.1] - 2026-09-21

### Added
- macOS (Apple Silicon) support: `macos/build-app.sh` builds an ad-hoc signed `Email Tracker.app`, and a pre-built copy is included at `macos/Email-Tracker-macOS-arm64.zip`

### Fixed
- **Reset All** now saves immediately. Previously it only cleared the on-screen rows, so the old data came back after closing and reopening unless Save Config was clicked first

## [1.1.0] - 2026-09-17

### Added
- **Copy button** per row — copies the row's email address to the clipboard, with a brief "Copied!" confirmation on the button itself

## [1.0.0] - 2026-06-27

### Added
- Initial release
- Scrollable email row list with Add Row support
- Per-row "Used" status radio button (non-exclusive, togglable)
- Auto Timestamp column — captures current date and time when a row is marked Used
- Resets field — free-text input per row for manual reset notes
- **Reset All button** — clears Status, Auto Timestamp, and Resets across all rows simultaneously
- Persistent JSON storage at `~/.config/email_tracker_data.json`
- Auto-generated custom SVG app icon written to `~/.config/email_tracker_icon.svg` on first launch
- Built with Qt6 Widgets, CMake 3.16+, C++17
