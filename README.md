# LLM Email Tracker

A lightweight Qt6 desktop utility for tracking email addresses used in LLM workflows — logs status, auto-timestamps when marked used, and supports manual reset notes.

![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-blue)
![Qt](https://img.shields.io/badge/Qt-6-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

## Features

- **Email list** — add as many rows as needed via scrollable UI
- **Status toggle** — "Used" radio button per row (non-exclusive, togglable)
- **Auto Timestamp** — automatically captures date and time when a row is marked Used
- **Resets field** — manual text field per row for tracking reset events or notes
- **Reset All button** — clears all Status toggles, Auto Timestamps, and Resets fields in one click
- **Copy button** — copies a row's email address to the clipboard, with a "Copied!" confirmation
- **Persistent storage** — saves/loads state from `~/.config/email_tracker_data.json`

## Requirements

- Qt 6 (Widgets module)
- CMake 3.16+
- C++17-compatible compiler (GCC, Clang)

Tested on CachyOS (Arch Linux) with Qt 6.x. Should work on any modern Linux with Qt6 installed.

## Build

```bash
git clone https://github.com/mjdeiter/llm-email-tracker.git
cd llm-email-tracker
mkdir -p build && cd build
cmake ..
cmake --build . --parallel
```

The binary will be at `build/EmailTracker`.

A pre-built x86-64 Linux binary is included at `build/EmailTracker` for convenience.  
It is dynamically linked and requires Qt6 libraries to be installed on the host system.

## macOS

Tested on Apple Silicon (arm64) with Qt 6 from Homebrew.

**Build from source** (recommended):

```bash
brew install cmake qt
git clone https://github.com/mjdeiter/llm-email-tracker.git
cd llm-email-tracker
macos/build-app.sh
```

This produces `dist/Email Tracker.app` (ad-hoc signed). Drag it to `/Applications`, or run it from anywhere.

**Pre-built:** `macos/Email-Tracker-macOS-arm64.zip` contains the same app built from this repo. It links dynamically against Homebrew's Qt (`brew install qt`), so it only runs on Apple Silicon Macs with Qt installed. It is not notarized; on first launch, right-click the app and choose Open, or run `xattr -dr com.apple.quarantine "Email Tracker.app"`.

Data is stored in the same place as on Linux: `~/.config/email_tracker_data.json`.

## Usage

Run the binary:

```bash
./build/EmailTracker
```

- **Add Row** — appends a new empty row
- **Save Config** — persists all rows to `~/.config/email_tracker_data.json`
- **Reset All** — clears Status, Auto Timestamp, and Resets on every row (does not affect email addresses)

State is saved manually with **Save Config** (except **Reset All**, which saves immediately); the app reloads your last saved state on next launch.

## Data Storage

Config is stored as JSON at:

```
~/.config/email_tracker_data.json
```

Example entry:

```json
[
  {
    "email": "user@example.com",
    "used": true,
    "timestamp": "06/27/2026 11:51:00",
    "resets": "reset after campaign 1"
  }
]
```

## License

MIT
