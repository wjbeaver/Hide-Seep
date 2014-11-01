; bioView installation script
; Author: Dmitry V. Fedorov <www.dimin.net>
; ver: 1.1.18

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------


; The name of the installer
Name "bioView"

; The file to write
OutFile "..\distr\bioview_win32_1-1-18.exe"

; The default installation directory
InstallDir $PROGRAMFILES\CBI\bioView
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\CBI "bioView_Install_Dir"

ShowInstDetails show
CRCCheck on

; The text to prompt the user to enter a directory
ComponentText "This will install bioView 1.1.18 on your computer. Select which optional things you want installed."
; The text to prompt the user to enter a directory
DirText "Choose a directory to install in to:"


;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  ;!define MUI_COMPONENTSPAGE_NODESC
  !define MUI_COMPONENTSPAGE_SMALLDESC
  
  ;!define MUI_UI "install\modern.exe"
  
  !define MUI_HEADERIMAGE 
  ;!define MUI_HEADERIMAGE_BITMAP "install\div5_install_v2.bmp"
  !define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
  
  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_FINISHPAGE_RUN $INSTDIR\wv.exe 

  
;--------------------------------
;Pages

  ;!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------


; The stuff to install
Section "-bioView (required)"

  SetOutPath $INSTDIR
  File /r "..\distr\win32\*.*"
        
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\CBI "bioView_Install_Dir" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\bioView" "DisplayName" "bioView (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\bioView" "UninstallString" '"$INSTDIR\uninstall.exe"'
  
  WriteUninstaller "uninstall.exe"
SectionEnd

; optional section
Section "Start Menu Shortcuts" SecStartMu
  CreateDirectory "$SMPROGRAMS\Center for Bio-Image Informatics\bioView"
  CreateShortCut "$SMPROGRAMS\Center for Bio-Image Informatics\bioView\bioView.lnk" "$INSTDIR\wv.exe" "" "$INSTDIR\wv.exe" 0
  CreateShortCut "$SMPROGRAMS\Center for Bio-Image Informatics\bioView\HELP.lnk" "$INSTDIR\doc\bioview.html" "" "$INSTDIR\doc\bioview.html" 0  
  CreateShortCut "$SMPROGRAMS\Center for Bio-Image Informatics\bioView\LICENCE.lnk" "$INSTDIR\LICENCE.txt" "" "$INSTDIR\LICENCE.txt" 0  
  CreateShortCut "$SMPROGRAMS\Center for Bio-Image Informatics\bioView\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

; optional section
Section "Web browser integration" SecSysPopU
  WriteRegStr HKCR "bioView" "" "URL:bioView Protocol"
  WriteRegStr HKCR "bioView" "URL Protocol" ''
  WriteRegStr HKCR "bioView\shell\open\command" "" '"$INSTDIR\wv.exe" "%1"'
SectionEnd

; optional section
Section "Desktop Shortcut" SecDeskTop
  CreateShortCut "$DESKTOP\bioView.lnk" "$INSTDIR\wv.exe" "" "$INSTDIR\wv.exe" 0
SectionEnd

; optional section
Section "Quick Launch Shortcut" SecQuickLu
  CreateShortCut "$QUICKLAUNCH\bioView.lnk" "$INSTDIR\wv.exe" "" "$INSTDIR\wv.exe" 0
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecStartMu ${LANG_ENGLISH} "Create Start Menu folder with main shortcuts."  
  LangString DESC_SecSysPopU ${LANG_ENGLISH} "Associate with bioView protocol for web browser integration" 
  LangString DESC_SecDeskTop ${LANG_ENGLISH} "Add shortcut to your desktop."     
  LangString DESC_SecQuickLu ${LANG_ENGLISH} "Add shortcut to your quick launch bar."     
      
  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMu} $(DESC_SecStartMu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSysPopU} $(DESC_SecSysPopU)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDeskTop} $(DESC_SecDeskTop)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecQuickLu} $(DESC_SecQuickLu)             
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------
; uninstall stuff

UninstallText "This will uninstall bioView. Hit next to continue."

; special uninstall section.
Section "Uninstall"
  
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\bioView"
  DeleteRegKey HKLM "SOFTWARE\CBI\bioView_Install_Dir"
  DeleteRegKey HKCR "bioView"  
  ;DeleteRegKey /ifempty HKCU "Software\CBI" 

  ; remove Start Menu shortcuts, if any.
  ;Delete "$SMPROGRAMS\Center for Bio-Image Informatics\bioView\*.*"
  RMDir /r "$SMPROGRAMS\Center for Bio-Image Informatics\bioView"  
  RMDir "$SMPROGRAMS\Center for Bio-Image Informatics"  

  Delete "$DESKTOP\bioView.lnk"
  Delete "$QUICKLAUNCH\bioView.lnk"
        
  ; remove files
  RMDir /r $INSTDIR

  
SectionEnd

; eof