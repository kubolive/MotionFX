; Inno Setup Script for MotionFX OBS Studio Plugin
; Disclaimer: MotionFX is an independent open-source plugin and is not affiliated with, sponsored by, or endorsed by the OBS Project or OBS Studio.
#define MyAppName "MotionFX for OBS Studio"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "KuboLive"
#define MyAppURL "https://github.com/kubolive/MotionFX"

[Setup]
AppId={{D34F1A58-7D9A-4B6A-914A-C39B72F41A99}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={code:GetObsInstallDir}
DirExistsWarning=no
DisableDirPage=no
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=MotionFX-v{#MyAppVersion}-Windows-Installer
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64compatible
WizardStyle=modern
UninstallFilesDir={app}\obs-plugins\64bit\motionfx-uninstall

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
; Plugin binary
Source: "..\..\staging\obs-plugins\64bit\motionfx.dll"; DestDir: "{app}\obs-plugins\64bit"; Flags: ignoreversion restartreplace

; Data / Shaders / Locales
Source: "..\..\staging\data\obs-plugins\motionfx\locale\*"; DestDir: "{app}\data\obs-plugins\motionfx\locale"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\staging\data\obs-plugins\motionfx\motionfx_3d.effect"; DestDir: "{app}\data\obs-plugins\motionfx"; Flags: ignoreversion

[Code]
// Auto-detect OBS Studio install path from registry or fallback to Program Files
function GetObsInstallDir(Param: string): string;
var
  InstallPath: string;
begin
  if RegQueryStringValue(HKLM, 'SOFTWARE\OBS Studio', '', InstallPath) then
  begin
    Result := InstallPath;
  end
  else if RegQueryStringValue(HKLM64, 'SOFTWARE\OBS Studio', '', InstallPath) then
  begin
    Result := InstallPath;
  end
  else if RegQueryStringValue(HKCU, 'SOFTWARE\OBS Studio', '', InstallPath) then
  begin
    Result := InstallPath;
  end
  else
  begin
    Result := ExpandConstant('{commonpf64}\obs-studio');
  end;
end;
