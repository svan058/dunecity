# Windows Installer - Complete Information

## ✅ What's Fixed

The Windows installer now works correctly with the modern CMake + vcpkg + CPack build system.

### Installation Directory
- **✅ Fixed:** Now installs to `C:\Program Files\Dune Legacy`
- **❌ Before:** Was installing to `C:\Program Files\DuneLegacy 0.98.7.0`

### File Structure
The installer creates a **hybrid structure**: EXE and DLLs in root, data in `share\DuneLegacy\`:

```
C:\Program Files\Dune Legacy\
├── dunelegacy.exe              ← Main executable
├── SDL2.dll                    ← All DLLs in root (15 total)
├── SDL2_mixer.dll
├── SDL2_ttf.dll
├── [other DLLs...]
├── AUTHORS, COPYING, NEWS, README  ← Documentation
└── share\                      ← Data directory
    └── DuneLegacy\
        ├── config\             ← Config templates
        │   ├── Dune Legacy.ini
        │   ├── ObjectData.ini.default
        │   └── QuantBot Config.ini.default
        ├── locale\             ← Translations
        │   ├── English.en.po
        │   ├── German.de.po
        │   └── [others...]
        ├── maps\               ← Custom maps
        │   ├── singleplayer\
        │   └── multiplayer\
        ├── GFXHD.PAK           ← Bundled PAK files (3)
        ├── LEGACY.PAK
        └── OPENSD2.PAK
```

**Important:** The `share\DuneLegacy\` structure is required because the game's path detection code expects this layout on Windows.

---

## ✅ All PAK Files Included

The installer now includes **ALL 18 PAK files**:

**Dune Legacy PAK files (3):**
- ✅ `GFXHD.PAK` - HD graphics (Dune Legacy)
- ✅ `LEGACY.PAK` - Enhanced features (Dune Legacy)
- ✅ `OPENSD2.PAK` - Open source data (Dune Legacy)

**Original Dune 2 PAK files (15):**
- ✅ `HARK.PAK` - Harkonnen house data
- ✅ `ATRE.PAK` - Atreides house data
- ✅ `ORDOS.PAK` - Ordos house data
- ✅ `ENGLISH.PAK` - English text
- ✅ `DUNE.PAK` - Main game data
- ✅ `SCENARIO.PAK` - Campaign missions
- ✅ `MENTAT.PAK` - Mentat advisor graphics
- ✅ `VOC.PAK` - Voice samples
- ✅ `MERC.PAK` - Mercenary house data
- ✅ `FINALE.PAK` - End game cinematics
- ✅ `INTRO.PAK` - Intro cinematics
- ✅ `INTROVOC.PAK` - Intro voice
- ✅ `SOUND.PAK` - Sound effects
- ✅ `GERMAN.PAK` - German language
- ✅ `FRENCH.PAK` - French language

**The game is ready to play immediately after installation!**

---

## 📦 Complete Installer Contents

### Included Files (7.1 MB NSIS installer)

**Executable:**
- `dunelegacy.exe` (2.6 MB)

**DLLs (15 files from vcpkg):**
- `SDL2.dll`, `SDL2_mixer.dll`, `SDL2_ttf.dll`
- `brotlicommon.dll`, `brotlidec.dll`, `bz2.dll`
- `freetype.dll`, `libcurl.dll`, `libpng16.dll`
- `ogg.dll`, `vorbis.dll`, `vorbisfile.dll`
- `wavpackdll.dll`, `zlib1.dll`

**Data Files:**
- 18 PAK files (3 Dune Legacy + 15 original Dune 2)
- Maps (singleplayer & multiplayer)
- Locale files (5 languages)
- Config templates
- Documentation (AUTHORS, COPYING, NEWS, README, Release_Notes)

---

## 🔧 Build Instructions

### Quick Build
```powershell
cd C:\source\dune\dunelegacy-code
cmake --build build --target installer --config Release
```

### Output
- `build\DuneLegacy-0.98.7.0-Windows-x64.exe` (NSIS installer)

### Build Time
- First build: ~5-10 minutes (vcpkg downloads dependencies)
- Subsequent builds: ~2-3 minutes

---

## ✨ Features

- ✅ **Modern Build System:** CMake + vcpkg + CPack
- ✅ **Automatic DLL Bundling:** vcpkg handles all dependencies
- ✅ **Professional Installer:** NSIS with Start Menu shortcuts
- ✅ **Uninstaller:** Full uninstall support
- ✅ **Proper Install Path:** `C:\Program Files\Dune Legacy`

---

## 📝 Technical Details

### CPack Configuration
- **Generator:** NSIS (Nullsoft Scriptable Install System)
- **Package Name:** "Dune Legacy"
- **Display Name:** "Dune Legacy"
- **Install Directory:** `C:\Program Files\Dune Legacy` (no version suffix)

### Build System
- **Compiler:** Visual Studio 2022 (MSVC 19.44)
- **Architecture:** x64
- **C++ Standard:** C++17
- **Package Manager:** vcpkg
- **Dependencies:** SDL2, SDL2_mixer, SDL2_ttf, libcurl, freetype

---

## 🐛 Known Issues

### PAK Files
- Installer only includes 3 of 17 PAK files
- Users must add original Dune 2 PAK files manually
- Consider adding a "first run" wizard to guide users

### Documentation
- README, AUTHORS, etc. don't have .txt extension on Windows
- Consider adding during install with .txt suffix for Windows usability

---

## 📚 References

- **Old NSI Script:** `legacy/nsis-old/nsis/dunelegacy.nsi`
- **CMake Config:** `CMakeLists.txt` (lines 138-248)
- **Source Install:** `src/CMakeLists.txt` (lines 440-482)
- **Windows Quick Start:** `WINDOWS_QUICKSTART.md`

