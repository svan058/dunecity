# Legacy NSIS Installer Scripts

**⚠️ DEPRECATED - These files are no longer used by the build system.**

## What's Here

This folder contains the old NSIS installer scripts that were manually maintained before the project switched to CMake + CPack.

### Files
- `nsis/` - Old NSIS script templates and resources
- `DuneLegacy-0.98.4-Setup.nsi` - Legacy installer for version 0.98.4
- `DuneLegacy-0.98.6-Setup.nsi` - Legacy installer for version 0.98.6

## Current Build System

The project now uses **CMake's CPack** with automatic NSIS generation:
- Configured in `CMakeLists.txt` (lines 194-219)
- Invoked via: `cmake --build build --target installer --config Release`
- Generates: `DuneLegacy-{VERSION}-Windows-x64.exe`

## Why Deprecated?

**Old approach:**
- Manual NSI scripts with hardcoded paths
- Required manual updates for every version
- Error-prone file management

**New approach:**
- Automatic file collection via CMake `install()` commands
- Version automatically pulled from `CMakeLists.txt`
- vcpkg handles DLL bundling automatically

## Historical Reference

These files are kept for:
1. Understanding the old build process
2. Migrating legacy installer features if needed
3. Historical documentation

---

**For current build instructions, see:** `WINDOWS_QUICKSTART.md`

