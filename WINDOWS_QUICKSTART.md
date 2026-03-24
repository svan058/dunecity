# Windows Quick Start Guide

Get up and running with Dune Legacy on Windows in 5 minutes!

## Prerequisites

- **Windows 10/11** (64-bit)
- **Visual Studio 2019 or 2022** (Community Edition is free)
  - Download: https://visualstudio.microsoft.com/downloads/
  - Install the "Desktop development with C++" workload
- **Git for Windows**
  - Download: https://git-scm.com/download/win
- **CMake** (usually included with Visual Studio)

---

## Step 1: Install vcpkg (One-Time Setup)

Open **PowerShell** or **Command Prompt**:

```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```

✅ **vcpkg is now installed at `C:\vcpkg`**

---

## Step 2: Clone Dune Legacy

```powershell
cd C:\dev  # or your preferred location
git clone https://github.com/svan058/dunelegacy.git
cd dunelegacy
```

---

## Step 3: Open in Cursor/VS Code

1. **Open Cursor/VS Code**
2. **File → Open Folder** → Select `C:\dev\dunelegacy`
3. **Trust the workspace** when prompted

The `.vscode` folder contains pre-configured build tasks!

---

## Step 4: Configure Project (First Time Only)

**Option A: Using VS Code Tasks (Recommended)**
1. Press **`Ctrl+Shift+P`**
2. Type: **"Tasks: Run Task"**
3. Select: **"🔧 Configure CMake (vcpkg)"**

**Option B: Using Terminal**
```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
```

**⏱️ First run takes 5-10 minutes** as vcpkg downloads and builds SDL2, SDL2_mixer, SDL2_ttf, libcurl, etc.

> **📦 Note:** CPack will automatically generate a Windows installer using NSIS.
> CMake will install NSIS if needed (no manual installation required!).

---

## Step 5: Build the Game + Create Installer

**Option A: Using VS Code (Easiest)**
1. Press **`Ctrl+Shift+B`** (or **Cmd+Shift+B**)
2. Select: **"🚀 Build + Create Installer (RECOMMENDED)"**

**Option B: Using Terminal**
```powershell
cmake --build build --target installer --config Release
```

**⏱️ Build takes 2-3 minutes**

**What This Does:**
- ✅ Compiles the game with Visual Studio
- ✅ Copies all data files and DLLs
- ✅ Creates staging directory with `cmake --install`
- ✅ Generates **NSIS installer** via CPack: `DuneLegacy-0.98.7.0-Windows-x64.exe`
- ✅ Also creates **ZIP archive**: `DuneLegacy-0.98.7.0-Windows-x64.zip`

Both packages are found in `build/` folder!

---

## Step 6: Run the Game

**Option A: Using VS Code Task**
1. Press **`Ctrl+Shift+P`**
2. Type: **"Tasks: Run Task"**
3. Select: **"▶️ Run Game"**

**Option B: Using Terminal**
```powershell
.\build\bin\Release\dunelegacy.exe
```

**Option C: Open Visual Studio Solution**
```powershell
start build\DuneLegacy.sln
```
Then press **F5** to build and run with debugger.

---

## Installer Technology (CPack + NSIS)

The build system uses **CMake's CPack** to automatically generate Windows installers.

**Old Approach (Deprecated):**
- Custom `.nsi` scripts in `nsis/` folder
- Manual `makensis` commands
- Hardcoded file paths

**New Approach (Current):**
- CPack automatically generates NSIS installer
- File paths managed by CMake's `install()` commands
- Automatic DLL bundling via vcpkg
- Generates both `.exe` installer and `.zip` archive

> **Note:** The old NSI files in `nsis/` are kept for reference only and are NOT used by the build system.

---

## Troubleshooting

### "vcpkg not found"
- Make sure vcpkg is at `C:\vcpkg`
- Or update the path in `.vscode\tasks.json` line 53

### "SDL2 not found"
- vcpkg didn't finish installing
- Run configure step again

### "MSBuild not found"
- Open **Visual Studio Installer**
- Modify your installation
- Ensure **"Desktop development with C++"** is checked

### "CPack: Create package" error
- NSIS installer will be downloaded automatically by CPack
- If it fails, install NSIS manually: https://nsis.sourceforge.io/Download
- Then run configure again

### Installer is missing DLLs
- vcpkg automatically copies DLLs to `build/bin/Release/`
- The installer packages everything from `cmake --install`
- Check `build/install/` folder to verify files

### Build is slow
- **First build**: 5-10 minutes (vcpkg downloads SDL2, libcurl, etc.)
- **Subsequent builds**: 2-3 minutes
- Use **Release** configuration, not Debug

---

## Next Steps

### Update Game Version
When you pull new changes:
```powershell
git pull origin release-0.98.6
cmake --build build --target installer --config Release
```

### Clean Rebuild
If things break:
1. Press **`Ctrl+Shift+P`**
2. Select: **"🧹 Clean Build"**
3. Then build again with **`Ctrl+Shift+B`**

---

## File Locations

| Item | Location |
|------|----------|
| **Source Code** | `C:\dev\dunelegacy\` (or `C:\source\dune\dunelegacy-code\`) |
| **Build Output** | `C:\dev\dunelegacy\build\` |
| **Executable** | `C:\dev\dunelegacy\build\bin\Release\dunelegacy.exe` |
| **NSIS Installer** | `C:\dev\dunelegacy\build\DuneLegacy-0.98.7.0-Windows-x64.exe` |
| **ZIP Archive** | `C:\dev\dunelegacy\build\DuneLegacy-0.98.7.0-Windows-x64.zip` |
| **Install Staging** | `C:\dev\dunelegacy\build\install\` (temporary) |
| **vcpkg** | `C:\vcpkg\` |

---

## VS Code Tips

### Quick Actions
- **`Ctrl+Shift+B`** - Build with installer (default task)
- **`Ctrl+Shift+P`** → "Tasks" - See all build tasks
- **`Ctrl+\``** - Open terminal

### Available Tasks
- 🚀 **Build + Create Installer** - Always use this!
- 🔧 **Configure CMake** - First-time setup
- 🧹 **Clean Build** - Delete build/ and reconfigure
- ▶️ **Run Game** - Launch the game
- 🎯 **Build and Run** - Build then run automatically

### Build Output
Watch the **TERMINAL** panel for:
- ✅ Build progress
- ❌ Compiler errors
- 📦 Installer creation

---

## Need Help?

- **Documentation**: See `BUILD.md` for detailed instructions
- **GitHub Issues**: https://github.com/svan058/dunelegacy/issues
- **Discord**: https://discord.gg/6sAcZr6y3B

---

**You're all set! Press `Ctrl+Shift+B` to build and start developing!** 🎮

