; Defines
!define PRODUCT_NAME            "OpenSCAM"
!define PRODUCT_EXE             "openscam.exe"
!define PRODUCT_ICON            "openscam.ico"
!define DISPLAY_NAME            "${PRODUCT_NAME}"
!define UNINSTALLER             "Uninstall.exe"
!define PRODUCT_LICENSE         "LICENSE"
!define PRODUCT_VENDOR          "%(vendor)s"
!define PRODUCT_TARGET          "%(package)s"
!define PRODUCT_VERSION         "%(version)s"
!define PRODUCT_WEBSITE         "http://openscam.com/"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_UNINST_KEY \
    "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_DIR_REGKEY \
    "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_NAME}"

!define MUI_ABORTWARNING
!define MUI_ICON "images\${PRODUCT_ICON}"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "images\header.bmp"
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH

; Includes
!include MUI2.nsh
!include nsDialogs.nsh
!include LogicLib.nsh


; Config
Name "${DISPLAY_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_TARGET}"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show


; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${PRODUCT_LICENSE}"
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${PRODUCT_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start ${PRODUCT_NAME}"
!insertmacro MUI_PAGE_FINISH

; !insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

; Sections
Section -Install
  ; Install files
  SetOutPath "$INSTDIR"
  File ${PRODUCT_EXE}
  File "oscamprobe.exe"
  File "oscamopt.exe"
  File "gcodetool.exe"
  File "tplang.exe"
  File "images\${PRODUCT_ICON}"
  File ${PRODUCT_LICENSE}
  File "README"
  File "ChangeLog"

  ; Qt DLLs
  File "$%%QT4DIR%%\bin\QtCore4.dll"
  File "$%%QT4DIR%%\bin\QtGui4.dll"
  File "$%%QT4DIR%%\bin\QtOpenGL4.dll"

  ; Examples
  SetOverwrite on
  SetOutPath "$INSTDIR\examples"
  File /r /x .svn "examples\*.*"

  ; Desktop
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_EXE}" \
    "" "$INSTDIR\${PRODUCT_ICON}"

  ; Start Menu
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" \
    "$INSTDIR\${PRODUCT_EXE}" "" "$INSTDIR\${PRODUCT_ICON}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" \
    "$INSTDIR\${UNINSTALLER}"

  ; Internet shortcut
  WriteIniStr "$INSTDIR\About ${PRODUCT_NAME}.url" "InternetShortcut" "URL" \
    "${PRODUCT_WEBSITE}"

  ; Write uninstaller
write_uninstaller:
  ClearErrors
  WriteUninstaller "$INSTDIR\${UNINSTALLER}"
  IfErrors 0 +2
    MessageBox MB_ABORTRETRYIGNORE "Failed to create uninstaller" \
      IDABORT abort IDRETRY write_uninstaller

  ; Save uninstall information
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "UninstallString" "$INSTDIR\${UNINSTALLER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "DisplayIcon" "$INSTDIR\${PRODUCT_ICON}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "URLInfoAbout" "${PRODUCT_WEBSITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "Publisher" "${PRODUCT_VENDOR}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" \
    "DisplayVersion" "${PRODUCT_VERSION}"

  Return

abort:
  Abort
SectionEnd


Section -un.Program
  ; Menu
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"

  ; Desktop
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"

  ; Registry
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"

  ; Program directory
  remove_dir:
  ClearErrors
  RMDir /r "$INSTDIR"
  IfErrors 0 +2
    MessageBox MB_RETRYCANCEL "Failed to remove $INSTDIR.  Please stop all \
      running ${PRODUCT_NAME} software." IDRETRY remove_dir
SectionEnd
