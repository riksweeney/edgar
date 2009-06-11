# The Legend of Edgar script

Name "The Legend of Edgar"
OutFile "edgar-0.1-1.installer.exe"
InstallDir "$PROGRAMFILES\Parallel Realities\Edgar"
Page Directory
Page InstFiles

Section "Main"
  SetOutPath $INSTDIR
  File edgar.exe
  File data.pak
  File edgar.ico
  File *.dll
  File /r locale
  File /r doc
SectionEnd

Section "Shortcuts"
	CreateDirectory "$SMPROGRAMS\Parallel Realities\Edgar"
	CreateShortCut "$SMPROGRAMS\Parallel Realities\Edgar\Edgar.lnk" "$INSTDIR\edgar.exe" "" $INSTDIR\edgar.ico 0 "" ""
	CreateShortCut "$SMPROGRAMS\Parallel Realities\Edgar\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
	CreateShortCut "$DESKTOP\Edgar.lnk" "$INSTDIR\edgar.exe" "" "$INSTDIR\edgar.ico" 0 "" ""
SectionEnd

Section "CreateUninstaller"
	WriteUninstaller "$INSTDIR\uninstaller.exe"
SectionEnd

Section "Uninstall"
  Delete "$SMPROGRAMS\Parallel Realities\Edgar\*.*"
  Delete "$SMPROGRAMS\Parallel Realities\Edgar"
  Delete "$DESKTOP\Edgar.lnk"
  RMDir /r "$INSTDIR"
SectionEnd
