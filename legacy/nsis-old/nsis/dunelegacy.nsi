SetCompressor /SOLID lzma

!include x64.nsh
!include MUI2.nsh

Name "Dune Legacy"
BrandingText " http://dunelegacy.sourceforge.net"
!define INSTALLATIONNAME "Dune Legacy"
!define VERSION "0.98.6.2"
OutFile "../build/installer/Dune Legacy ${VERSION} Setup.exe"
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
  File "../data\maps\singleplayer\*.*"

  SetOutPath $INSTDIR\maps\multiplayer
  File "../data\maps\multiplayer\*.ini"

  SetOutPath $INSTDIR\locale
  File "../data\locale\*.po"

  SetOutPath $INSTDIR\config
  File "../config\Dune Legacy.ini"
  File "../config\ObjectData.ini"
  File "../config\QuantBot Config.ini"
  File "../config\README.md"

  SetOutPath $INSTDIR
  ${If} ${RunningX64}
    File "../bin\Release-x64\DuneLegacy.exe"
    File "../bin\Release-x64\SDL2.dll"
    File "../bin\Release-x64\SDL2_mixer.dll"
    File "../bin\Release-x64\SDL2_ttf.dll"
    
    File "../bin\Release-x64\ATRE.PAK"
    File "../bin\Release-x64\DUNE.PAK"
    File "../bin\Release-x64\ENGLISH.PAK"
    File "../bin\Release-x64\FINALE.PAK"
    File "../bin\Release-x64\FRENCH.PAK"
    File "../bin\Release-x64\GERMAN.PAK"
    File "../bin\Release-x64\GFXHD.PAK"
    File "../bin\Release-x64\HARK.PAK"
    File "../bin\Release-x64\INTRO.PAK"
    File "../bin\Release-x64\INTROVOC.PAK"
    File "../bin\Release-x64\LEGACY.PAK"
    File "../bin\Release-x64\MENTAT.PAK"
    File "../bin\Release-x64\MERC.PAK"
    File "../bin\Release-x64\OPENSD2.PAK"
    File "../bin\Release-x64\ORDOS.PAK"
    File "../bin\Release-x64\SCENARIO.PAK"
    File "../bin\Release-x64\SOUND.PAK"
    File "../bin\Release-x64\VOC.PAK"
    
    ; Verify all required PAK files were copied successfully
    Call VerifyPakFiles
  ${EndIf}

  File "../COPYING"
  Push "$INSTDIR\COPYING"
  Push "$INSTDIR\License.txt"
  Call unix2dos

  File "../AUTHORS"
  Push "$INSTDIR\AUTHORS"
  Push "$INSTDIR\Authors.txt"
  Call unix2dos

  File "../README"
  Push "$INSTDIR\README"
  Push "$INSTDIR\Readme.txt"
  Call unix2dos

  WriteUninstaller $INSTDIR\uninstall.exe
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayName" "${INSTALLATIONNAME} ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayIcon" '"$INSTDIR\DuneLegacy.exe",0'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoRepair" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayVersion" "${VERSION}"
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${INSTALLATIONNAME}"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Dune Legacy.lnk" "$INSTDIR\DuneLegacy.exe" "" "$INSTDIR\DuneLegacy.exe" 0
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Readme.lnk" "$INSTDIR\Readme.txt"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\License.lnk" "$INSTDIR\License.txt"
  
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

