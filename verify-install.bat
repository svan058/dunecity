@echo off
echo ================================================
echo    Dune Legacy Installation Verification
echo ================================================
echo.

set INSTALL_DIR=C:\Program Files\DuneLegacy

echo Checking installation at: %INSTALL_DIR%
echo.

if not exist "%INSTALL_DIR%" (
    echo [ERROR] Dune Legacy is not installed!
    echo Please run the installer first.
    pause
    exit /b 1
)

echo [1/5] Checking executable...
if exist "%INSTALL_DIR%\dunelegacy.exe" (
    echo   [OK] dunelegacy.exe found
) else (
    echo   [ERROR] dunelegacy.exe NOT FOUND!
)

echo [2/5] Checking DLLs...
if exist "%INSTALL_DIR%\SDL2.dll" (
    echo   [OK] SDL2.dll found
) else (
    echo   [ERROR] SDL2.dll NOT FOUND!
)

echo [3/5] Checking data directory structure...
if exist "%INSTALL_DIR%\share\DuneLegacy" (
    echo   [OK] share\DuneLegacy directory exists
) else (
    echo   [ERROR] share\DuneLegacy directory NOT FOUND!
    echo   You may be running an old installation.
    echo   Please uninstall and reinstall.
)

echo [4/5] Checking config files...
if exist "%INSTALL_DIR%\share\DuneLegacy\config\ObjectData.ini.default" (
    echo   [OK] ObjectData.ini.default found
) else (
    echo   [ERROR] ObjectData.ini.default NOT FOUND!
)
if exist "%INSTALL_DIR%\share\DuneLegacy\config\QuantBot Config.ini.default" (
    echo   [OK] QuantBot Config.ini.default found
) else (
    echo   [ERROR] QuantBot Config.ini.default NOT FOUND!
)

echo [5/5] Checking locale files...
if exist "%INSTALL_DIR%\share\DuneLegacy\locale\English.en.po" (
    echo   [OK] English.en.po found
) else (
    echo   [ERROR] English.en.po NOT FOUND!
)

echo.
echo ================================================
echo    Verification Complete
echo ================================================
echo.
echo If all checks passed, the game should work correctly.
echo If any checks failed, reinstall using the latest installer.
echo.
pause

