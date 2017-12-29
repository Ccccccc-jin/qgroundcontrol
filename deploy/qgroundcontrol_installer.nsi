!include "MUI2.nsh"
!include LogicLib.nsh
!include Win\COM.nsh
!include Win\Propkey.nsh
!include x64.nsh

!macro DemoteShortCut target
    !insertmacro ComHlpr_CreateInProcInstance ${CLSID_ShellLink} ${IID_IShellLink} r0 ""
    ${If} $0 <> 0
            ${IUnknown::QueryInterface} $0 '("${IID_IPersistFile}",.r1)'
            ${If} $1 P<> 0
                    ${IPersistFile::Load} $1 '("${target}",1)'
                    ${IUnknown::Release} $1 ""
            ${EndIf}
            ${IUnknown::QueryInterface} $0 '("${IID_IPropertyStore}",.r1)'
            ${If} $1 P<> 0
                    System::Call '*${SYSSTRUCT_PROPERTYKEY}(${PKEY_AppUserModel_StartPinOption})p.r2'
                    System::Call '*${SYSSTRUCT_PROPVARIANT}(${VT_UI4},,&i4 ${APPUSERMODEL_STARTPINOPTION_NOPINONINSTALL})p.r3'
                    ${IPropertyStore::SetValue} $1 '($2,$3)'

                    ; Reuse the PROPERTYKEY & PROPVARIANT buffers to set another property
                    System::Call '*$2${SYSSTRUCT_PROPERTYKEY}(${PKEY_AppUserModel_ExcludeFromShowInNewInstall})'
                    System::Call '*$3${SYSSTRUCT_PROPVARIANT}(${VT_BOOL},,&i2 ${VARIANT_TRUE})'
                    ${IPropertyStore::SetValue} $1 '($2,$3)'

                    System::Free $2
                    System::Free $3
                    ${IPropertyStore::Commit} $1 ""
                    ${IUnknown::Release} $1 ""
            ${EndIf}
            ${IUnknown::QueryInterface} $0 '("${IID_IPersistFile}",.r1)'
            ${If} $1 P<> 0
                    ${IPersistFile::Save} $1 '("${target}",1)'
                    ${IUnknown::Release} $1 ""
            ${EndIf}
            ${IUnknown::Release} $0 ""
    ${EndIf}
!macroend

Name "${APPNAME}"
Var StartMenuFolder

InstallDir "$PROGRAMFILES\${APPNAME}"

SetCompressor /SOLID /FINAL lzma

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${HEADER_BITMAP}";
!define MUI_ICON "${INSTALLER_ICON}";
!define MUI_UNICON "${INSTALLER_ICON}";

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section
  ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString"
  StrCmp $R0 "" doinstall

  ExecWait "$R0 /S _?=$INSTDIR"
  IntCmp $0 0 doinstall

  MessageBox MB_OK|MB_ICONEXCLAMATION \
        "Could not remove a previously installed ${APPNAME} version.$\n$\nPlease remove it before continuing."
  Abort

doinstall:
  SetOutPath $INSTDIR
  File /r /x ${EXENAME}.pdb /x ${EXENAME}.lib /x ${EXENAME}.exp ${DESTDIR}\*.*
  File deploy\px4driver.msi
  File deploy\wdi-simple.exe

  ExpandEnvStrings $1 %COMSPEC%

  DetailPrint "Checking for BCM2708 driver..."
  ExpandEnvStrings $1 %COMSPEC%
  ${DisableX64FSRedirection}
  ExecWait '"$1" /c "pnputil.exe -e | findstr $\"USB\VID_0A5C&PID_2763$\""' $0
  DetailPrint "Driver check returned $0"
  ${EnableX64FSRedirection}

  ${If} $0 != 0
    DetailPrint "Driver not found. Installing BCM2708 driver..."
    ExecWait '"$INSTDIR\wdi-simple.exe" -n "Raspberry Pi USB boot" -v 0x0a5c -p 0x2763 -t 0' $0 
    DetailPrint "Driver install returned $0"
  ${Else}
    DetailPrint "Driver found. Skipping install."
  ${EndIf}

  DetailPrint "Checking for BCM2708 driver..."
  ${DisableX64FSRedirection}
  ExecWait '"$1" /c "pnputil.exe -e | findstr $\"USB\VID_0A5C&PID_2764$\""' $0
  DetailPrint "Driver check returned $0"
  ${EnableX64FSRedirection}

  ${If} $0 != 0
    DetailPrint "Driver not found. Installing BCM2710 driver..."
    ExecWait '"$INSTDIR\wdi-simple.exe" -n "Raspberry Pi USB boot" -v 0x0a5c -p 0x2764 -t 0' $0
    DetailPrint "Driver install returned $0"
  ${Else}
    DetailPrint "Driver found. Skipping install"
  ${EndIf}

  WriteUninstaller $INSTDIR\${EXENAME}-Uninstall.exe
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$\"$INSTDIR\${EXENAME}-Uninstall.exe$\""

  ; Only attempt to install the PX4 driver if the version isn't present
  !define ROOTKEY "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\434608CF2B6E31F0DDBA5C511053F957B55F098E"

  SetRegView 64
  ReadRegStr $0 HKLM "${ROOTKEY}" "Publisher"
  StrCmp     $0 "3D Robotics" found_provider notfound

found_provider:
  ReadRegStr $0 HKLM "${ROOTKEY}" "DisplayVersion"
  DetailPrint "Checking USB driver version... $0"
  StrCmp     $0 "04/11/2013 2.0.0.4" skip_driver notfound

notfound:
  DetailPrint "USB Driver not found... installing"
  ExecWait '"msiexec" /i "px4driver.msi"'
  goto done

skip_driver:
  DetailPrint "USB Driver found... skipping install"
done:
  SetRegView lastused
SectionEnd 

Section "Uninstall"
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  SetShellVarContext all
  RMDir /r /REBOOTOK $INSTDIR
  RMDir /r /REBOOTOK "$SMPROGRAMS\$StartMenuFolder\"
  SetShellVarContext current
  RMDir /r /REBOOTOK "$APPDATA\${ORGNAME}\"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
SectionEnd

Section "create Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME}.lnk" "$INSTDIR\${EXENAME}.exe" "" "$INSTDIR\${EXENAME}.exe" 0
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (GPU Compatibility Mode).lnk" "$INSTDIR\${EXENAME}.exe" "-angle" "$INSTDIR\${EXENAME}.exe" 0
  !insertmacro DemoteShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (GPU Compatibility Mode).lnk"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (GPU Safe Mode).lnk" "$INSTDIR\${EXENAME}.exe" "-swrast" "$INSTDIR\${EXENAME}.exe" 0
  !insertmacro DemoteShortCut "$SMPROGRAMS\$StartMenuFolder\${APPNAME} (GPU Safe Mode).lnk"
SectionEnd

