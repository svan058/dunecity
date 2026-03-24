SetCompressor /SOLID lzma

!include x64.nsh
!include MUI2.nsh

Name "Dune Legacy"
BrandingText " http://dunelegacy.sourceforge.net"
!define INSTALLATIONNAME "Dune Legacy"
!define VERSION "0.98.6.2-optimized"
OutFile "../build/installer/Dune Legacy ${VERSION}-win64 Setup.exe"
InstallDir "$PROGRAMFILES\${INSTALLATIONNAME}"

RequestExecutionLevel admin

!define MUI_ICON "../dunelegacy.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "modern-wizard.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "modern-wizard.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "modern-header.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "../COPYING"

!define MUI_DIRECTORYPAGE_VARIABLE $INSTDIR
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"

LangString UNINSTALL_NAME ${LANG_ENGLISH} "Uninstall Dune Legacy"
LangString UNINSTALL_NAME ${LANG_GERMAN} "Dune Legacy deinstallieren"

Function .onInit
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    SetRegView 64
    StrCpy $INSTDIR "$PROGRAMFILES64\${INSTALLATIONNAME}"
  ${EndIf}
FunctionEnd

Function un.onInit
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    SetRegView 64
    StrCpy $INSTDIR "$PROGRAMFILES64\${INSTALLATIONNAME}"
  ${EndIf}
FunctionEnd

Section ""
  SetOutPath $INSTDIR\maps\singleplayer
  File "..\data\maps\singleplayer\*.*"

  SetOutPath $INSTDIR\maps\multiplayer
  File "..\data\maps\multiplayer\*.ini"

  SetOutPath $INSTDIR\locale
  File "..\data\locale\*.po"

  SetOutPath $INSTDIR
  ${If} ${RunningX64}
    File "..\build\bin\dunelegacy.exe"
    File "..\build\bin\SDL2.dll"
    File "..\build\bin\SDL2_mixer.dll"
    File "..\build\bin\SDL2_ttf.dll"
    
    ; Add MinGW runtime DLLs
    File "..\build\bin\libgcc_s_seh-1.dll"
    File "..\build\bin\libstdc++-6.dll"
    File "..\build\bin\libwinpthread-1.dll"
    
    File "..\build\bin\ATRE.PAK"
    File "..\build\bin\DUNE.PAK"
    File "..\build\bin\ENGLISH.PAK"
    File "..\build\bin\FINALE.PAK"
    File "..\build\bin\FRENCH.PAK"
    File "..\build\bin\GERMAN.PAK"
    File "..\build\bin\GFXHD.PAK"
    File "..\build\bin\HARK.PAK"
    File "..\build\bin\INTRO.PAK"
    File "..\build\bin\INTROVOC.PAK"
    File "..\build\bin\LEGACY.PAK"
    File "..\build\bin\MENTAT.PAK"
    File "..\build\bin\MERC.PAK"
    File "..\build\bin\OPENSD2.PAK"
    File "..\build\bin\ORDOS.PAK"
    File "..\build\bin\SCENARIO.PAK"
    File "..\build\bin\SOUND.PAK"
    File "..\build\bin\VOC.PAK"
    
    ; Verify all required PAK files were copied successfully
    Call VerifyPakFiles
  ${EndIf}

  File "..\COPYING"
  Push "$INSTDIR\COPYING"
  Push "$INSTDIR\License.txt"
  Call unix2dos

  File "..\AUTHORS"
  Push "$INSTDIR\AUTHORS"
  Push "$INSTDIR\Authors.txt"
  Call unix2dos

  File "..\README"
  Push "$INSTDIR\README"
  Push "$INSTDIR\Readme.txt"
  Call unix2dos

  ; Create a readme file for the optimizations
  FileOpen $0 "$INSTDIR\Optimizations.txt" w
  FileWrite $0 "Dune Legacy ${VERSION}$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "This build includes performance optimizations:$\r$\n"
  FileWrite $0 "- QuantBot AI has been optimized with a cached combat units list$\r$\n"
  FileWrite $0 "- Sandworm behavior optimized to use AMBUSH mode for better performance$\r$\n"
  FileWrite $0 "- Fixed sandworm aggression to make them less aggressive across the map$\r$\n"
  FileWrite $0 "- Improved CPU efficiency in large-scale battles$\r$\n"
  FileWrite $0 "- Reduced CPU load during AI decision making$\r$\n"
  FileClose $0

  WriteUninstaller $INSTDIR\uninstall.exe
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayName" "${INSTALLATIONNAME} ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayIcon" '"$INSTDIR\dunelegacy.exe",0'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoRepair" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayVersion" "${VERSION}"
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${INSTALLATIONNAME}"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Dune Legacy.lnk" "$INSTDIR\dunelegacy.exe" "" "$INSTDIR\dunelegacy.exe" 0
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Readme.lnk" "$INSTDIR\Readme.txt"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\License.lnk" "$INSTDIR\License.txt"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Optimizations.lnk" "$INSTDIR\Optimizations.txt"
  
  WriteINIStr "$INSTDIR\Dune Legacy Website.URL" "InternetShortcut" "URL" "http://dunelegacy.sourceforge.net/"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Dune Legacy Website.lnk" "$INSTDIR\Dune Legacy Website.URL"
  
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\$(UNINSTALL_NAME).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Uninstall"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}"
  RMDir /r "$INSTDIR"

  Delete "$SMPROGRAMS\${INSTALLATIONNAME}\*.*"
  RMDir "$SMPROGRAMS\${INSTALLATIONNAME}"
SectionEnd

Function unix2dos
    ; strips all CRs
    ; and then converts all LFs into CRLFs
    ; (this is roughly equivalent to "cat file | dos2unix | unix2dos")
    ;
    ; usage:
    ;    Push "infile"
    ;    Push "outfile"
    ;    Call unix2dos
    ;
    ; beware that this function destroys $0 $1 $2

    ClearErrors

    Pop $2
    FileOpen $1 $2 w 

    Pop $2
    FileOpen $0 $2 r

    Push $2 ; save name for deleting

    IfErrors unix2dos_done

    ; $0 = file input (opened for reading)
    ; $1 = file output (opened for writing)

unix2dos_loop:
    ; read a byte (stored in $2)
    FileReadByte $0 $2
    IfErrors unix2dos_done ; EOL
    ; skip CR
    StrCmp $2 13 unix2dos_loop
    ; if LF write an extra CR
    StrCmp $2 10 unix2dos_cr unix2dos_write

unix2dos_cr:
    FileWriteByte $1 13

unix2dos_write:
    ; write byte
    FileWriteByte $1 $2
    ; read next byte
    Goto unix2dos_loop

unix2dos_done:

    ; close files
    FileClose $0
    FileClose $1

    ; delete original
    Pop $0
    Delete $0

FunctionEnd

Function VerifyPakFiles
  IfFileExists "$INSTDIR\HARK.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\ATRE.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\ORDOS.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\ENGLISH.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\DUNE.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\SCENARIO.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\MENTAT.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\VOC.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\MERC.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\FINALE.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\INTRO.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\INTROVOC.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\SOUND.PAK" 0 FileNotFound
  Return

FileNotFound:
  MessageBox MB_OK "Error: One or more required PAK files could not be installed correctly."
  Abort
FunctionEnd 