# AGENTS.md

Guidance for AI coding agents working in this repository.

## Development

- Always keep code generalized.
- Always add or update existing unit tests for any changes made in the code.
- For comments: state facts and use simple langauge with short sentences. Follow softened ASD-STE100 standard and use light caveman mode.
- Add diagnostic logging to facilitate error logging.

## Building

Always build through the root PowerShell script instead of invoking CMake or MSVC
directly:

```powershell
.\mediaflyouts.ps1 build
```

This handles Qt/MSVC environment setup, CMake configure, compile, and Qt runtime
deployment. Useful variants:

- `.\mediaflyouts.ps1 build -Config release` — release build
- `.\mediaflyouts.ps1 build -Clean` — full rebuild
- `.\mediaflyouts.ps1 build -RunTests` — build then run the test suite
- `.\mediaflyouts.ps1 test` — run the unit tests
- `.\mediaflyouts.ps1 run` — launch the app
- `.\mediaflyouts.ps1 help` — list all commands

## Version Control

Never commit or push agent made changes automatically.
