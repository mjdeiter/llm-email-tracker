# LLM Email Tracker

A lightweight Qt6 desktop utility for tracking email addresses used in LLM workflows — logs status, auto-timestamps when marked used, and supports manual reset notes.

![Platform](https://img.shields.io/badge/platform-Linux-blue)
![Qt](https://img.shields.io/badge/Qt-6-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

## Features

- **Email list** — add as many rows as needed via scrollable UI
- **Status toggle** — "Used" radio button per row (non-exclusive, togglable)
- **Auto Timestamp** — automatically captures date and time when a row is marked Used
- **Resets field** — manual text field per row for tracking reset events or notes
- **Reset All button** — clears all Status toggles, Auto Timestamps, and Resets fields in one click
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

## Usage

Run the binary:

```bash
./build/EmailTracker
```

- **Add Row** — appends a new empty row
- **Save Config** — persists all rows to `~/.config/email_tracker_data.json`
- **Reset All** — clears Status, Auto Timestamp, and Resets on every row (does not affect email addresses)

State is saved manually; the app will reload your last saved state on next launch.

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
