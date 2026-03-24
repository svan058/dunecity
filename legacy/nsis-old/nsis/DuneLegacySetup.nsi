;NSIS Modern User Interface
;Dune Legacy Setup
;Written by Stefan van der Wel

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Dune Legacy"
  OutFile "Dune Legacy 0.98.6.2 Setup.exe"
  Unicode True

  ;Default installation folder
  InstallDir "$PROGRAMFILES64\Dune Legacy"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Dune Legacy" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ICON "DuneLegacy.exe"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "$PLUGINSDIR\modern-header.bmp" ; optional
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "Dune Legacy" GameFiles

  SetOutPath "$INSTDIR"

  ;ADD YOUR OWN FILES HERE...
  File dunelegacy.exe
  File SDL2.dll
  File SDL2_mixer.dll
  File SDL2_ttf.dll
  File LEGACY.PAK
  File OPENSD2.PAK
  File GFXHD.PAK
  File COPYING.txt
  File LICENSE.txt
  File AUTHORS.txt
  File README.txt
  ;Store installation folder
  WriteRegStr HKCU "Software\Dune Legacy" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_GameFiles ${LANG_ENGLISH} "A test section."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${GameFiles} $(DESC_GameFiles)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\dunelegacy.exe"
  Delete "$INSTDIR\SDL2.dll"
  Delete "$INSTDIR\SDL2_mixer.dll"
  Delete "$INSTDIR\SDL2_ttf.dll"
  Delete "$INSTDIR\LEGACY.PAK"
  Delete "$INSTDIR\OPENSD2.PAK"
  Delete "$INSTDIR\GFXHD.PAK"
  Delete "$INSTDIR\COPYING.txt"
  Delete "$INSTDIR\LICENSE.txt"
  Delete "$INSTDIR\AUTHORS.txt"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\Dune Legacy"

SectionEnd

; Supporting functions

; get the original PAK files
Function Browsesource
  nsDialogs::SelectFolderDialog "Select Source Folder" "c:\"
  ;pop $SOURCE
  ${NSD_SetText} $SOURCETEXT $SOURCE
FunctionEnd