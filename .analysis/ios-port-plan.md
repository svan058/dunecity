# Dune Legacy → iPhone Port Plan

## 1. Clarify Goals & Constraints
- Target hardware/iOS versions: align with SDL2 support matrix (iOS 13+ recommended) and Apple Silicon build hosts.
- Determine app distribution path (App Store vs TestFlight/sideload). Drives entitlement, signing, and legal review for requiring original Dune 2 data files.
- Decide if multiplayer, map editor, and mods are in-scope for the first release.
- Confirm acceptable solutions for obtaining the original PAK files on iOS (bundle legal status vs user-provided import flow).

## 2. Developer Environment Bring-Up
1. Install and validate Xcode + iOS SDKs, including command line tools. Enable code signing certificates/profiles for debug/test.
2. Bring SDL2, SDL2_mixer, SDL2_ttf, and libcurl builds for iOS (prefer vcpkg or fetchContent). If vcpkg isn’t practical on iOS, create a `cmake/toolchains/ios.cmake` that points to locally built static frameworks.
3. Verify third-party code in `external/` for desktop-only assumptions (e.g., ENet HTTP, adlib player); document replacements or stubs required on iOS.

## 3. Build System & Project Structure
1. Extend `CMakeLists.txt` to recognize an `IOS` option. Key work:
   - Set `CMAKE_SYSTEM_NAME iOS`, `CMAKE_OSX_SYSROOT iphoneos`, `CMAKE_OSX_ARCHITECTURES arm64`.
   - Use `CMAKE_XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY` (1 for iPhone, 1,2 for iPhone/iPad).
   - Switch bundle generation from macOS settings to iOS-specific plist, icons, and launch screen assets (new files under `IDE/iOS/`).
2. Add a dedicated CMake preset or script (`buildios.sh`) that produces an Xcode project capable of running in Simulator + device.
3. Introduce an iOS-specific `Info.plist`, app icon set, and entitlements. Ensure App Store-compliant versioning and privacy usage strings (camera/microphone not needed?).
4. Automate packaging to an `.ipa` via `xcodebuild -scheme dunelegacy archive` + `-exportArchive`. Document signing expectations in `BUILD.md`.

## 4. Platform Abstraction Layer
1. Audit the existing platform-specific code paths in `src/CrashHandler.cpp`, `SoundPlayer.cpp`, filesystem helpers, etc. Create an `src/platform/ios/` module if needed for code isolated behind `#if TARGET_OS_IPHONE`.
2. Replace macOS bundle path lookups with iOS equivalents:
   - Read-only assets from `[[NSBundle mainBundle] resourcePath]` (exposed via SDL).
   - Writable paths via `NSDocumentDirectory` or `NSApplicationSupportDirectory`, accessible through SDL’s `SDL_SysWMinfo` or Objective‑C++ bridge file.
3. Handle system events: respond to background/foreground notifications (`UIApplicationWillResignActiveNotification`) to pause the game gracefully.
4. Review `libcurl` usage for multiplayer/meta-server. Confirm Apple’s networking requirements (ATS) and add plist whitelists if HTTP endpoints are non-HTTPS.

## 5. Input, UI, and UX Adaptation
1. Replace or supplement mouse/keyboard controls in `GUI/*` and `GameInterface.cpp` with touch gestures:
   - Tap → select, double tap → confirm, long press → context menu.
   - Two-finger drag for scrolling, pinch for zoom (wire up via SDL touch APIs hooking into `Game` camera logic).
2. Provide on-screen buttons for essential keyboard shortcuts (pause, menu, structure build, unit groups). Implement as SDL overlays or platform-native UI layered above SDL view.
3. Update text input (chat, naming saves) to use iOS virtual keyboard via SDL_StartTextInput.
4. Ensure menus scale to small screens: audit `GUIStyle` constants, add DPI-aware scaling, and test on 4.7"–6.7" displays.
5. Provide controller support (MFi, DualShock, Xbox) mapping to existing keybinds for a premium experience.

## 6. Audio, Performance, and Memory
1. Confirm SDL_mixer backends compile for iOS; disable unsupported codecs or ship prebuilt codec libs as needed.
2. Profile rendering on device (Instruments + Metal frame capture) to ensure 60fps; optimize surfaces/blits or migrate to SDL_Renderer/Metal if current code relies on software rendering.
3. Audit memory usage—old PCs tolerated >512 MB but iPhones have watchdog limits. Add texture streaming/unload logic for large maps if necessary.
4. Ensure background music playback meets iOS audio session rules (set category to `AVAudioSessionCategorySoloAmbient`).

## 7. Data & Save-Game Handling
1. Decide how players supply original PAK files:
   - Option A: ship free demo data only (verify licensing). 
   - Option B: add import UI using UIDocumentPicker / Files app, copying to `Documents/data/`.
2. Implement sandbox-safe save locations for campaigns, skirmishes, and settings. Update configuration paths used in `config/Dune Legacy.ini` accordingly.
3. Provide migration/backup instructions and iTunes File Sharing support to extract/import saves.

## 8. Testing Strategy
1. Unit/integration tests already in `tests/` should be cross-compiled (where feasible) or run on desktop CI to prevent regressions before deploying to iOS.
2. Manual QA matrix:
   - Devices: iPhone 8, iPhone 11, iPhone 14 Pro, iPad (if supported).
   - Scenarios: campaign missions, skirmish with AI, multiplayer host/join, save/load, backgrounding, low-memory kill.
3. Automate smoke tests via XCTest + `simctl` to launch, wait for SDL window, and verify no crashes on boot.

## 9. Compliance & Release
1. Review licensing of bundled data/codecs to ensure App Store compatibility (GPL + static linking restrictions). Evaluate whether the app must be open-source-distributed or if LGPL libs require dynamic linking.
2. Update documentation (`README`, `BUILD.md`, website) with iOS instructions, screenshots, and data import steps.
3. Prepare marketing assets (App Store screenshots, preview video). Ensure localization files cover supported languages.
4. Set up CI job to build signed `.ipa` artifacts for TestFlight via GitHub Actions or local scripts.

## 10. Timeline (rough)
1. Weeks 1-2: Research + build toolchain + dependency proof-of-concept (SDL sample on device).
2. Weeks 3-5: Platform layer + filesystem + rendering/input adaptation.
3. Weeks 6-7: Touch UI polish, audio/network fixes, data import UX.
4. Weeks 8-9: Optimization pass, QA, and TestFlight builds.
5. Weeks 10+: App Store submission, post-launch bugfixes.
