# Changelog

All notable changes to LLM Email Tracker are documented here.

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
