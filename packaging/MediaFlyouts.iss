; Inno Setup script for the MediaFlyouts installer.
; Build with: ISCC.exe packaging\MediaFlyouts.iss
; The version is read from the top-level VERSION file (override with /DMyAppVersion=x.y.z).
; It packages the windeployqt output in <root>\bin, so build + deploy first.

#ifndef MyAppVersion
  #define VerFile FileOpen("..\VERSION.txt")
  #define MyAppVersion Trim(FileRead(VerFile))
  #expr FileClose(VerFile)
  #undef VerFile
#endif
#define MyAppName "MediaFlyouts"
#define MyAppPublisher "MediaFlyouts"
#define MyAppURL "https://github.com/scleaverzer0ne/MediaFlyouts"
#define MyAppExeName "MediaFlyouts.exe"

[Setup]
; NOTE: keep this AppId stable across versions so upgrades replace in place.
AppId={{B7E6F4B2-2C1A-4E9E-9A2E-6D0A1C3F5A10}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=MediaFlyouts-{#MyAppVersion}-setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}
SetupIconFile=assets\MediaFlyouts.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; the entire deployed app (exe + Qt runtime staged by windeployqt)
Source: "..\bin\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
