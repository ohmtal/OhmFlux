; SETUP RadioWana II
[Setup]
AppId={{0611F876-10EF-474C-8849-C876C66BD1C7}}
AppName=RadioWana 2
AppVerName=Auteria 0.260401

AppPublisher=Thomas Huehn
AppPublisherURL=http://www.ohmtal.com/
AppSupportURL=https://github.com/ohmtal/RadioWana
AppUpdatesURL=https://github.com/ohmtal/RadioWana

DefaultDirName={autopf}\RadioWana2
DefaultGroupName=RadioWana
AllowNoIcons=yes
OutputDir=C:\opt\OhmFlux\RadioWana\res
OutputBaseFilename=RadioWana_2_260401_Setup
Compression=lzma
SolidCompression=yes
; LicenseFile={#file "C:\torque\SDK\auteria_release_script\Auteria_Enduser_License.txt"}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{group}\RadioWana"; Filename: "{app}\RadioWana.exe"; WorkingDir: "{app}"
Name: "{commondesktop}\RadioWana.exe"; Filename: "{app}\RadioWana.exe"; Tasks: desktopicon; WorkingDir: "{app}"
[Run]
Filename: "{app}\RadioWana.exe"; Description: "{cm:LaunchProgram,RadioWana}"; Flags: nowait postinstall skipifsilent

[Files]
Source: "C:\opt\OhmFlux\RadioWana\assets\*"; DestDir: "{app}\assets"; Flags: ignoreversion recursesubdirs
Source: "C:\opt\OhmFlux\RadioWana\RadioWana.exe"; DestDir: "{app}"; Flags: ignoreversion 
Source: "C:\opt\OhmFlux\RadioWana\glew32.dll"; DestDir: "{app}"; Flags: ignoreversion 
Source: "C:\opt\OhmFlux\RadioWana\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion 
Source: "C:\opt\OhmFlux\RadioWana\SDL3.dll"; DestDir: "{app}"; Flags: ignoreversion 
Source: "C:\opt\OhmFlux\RadioWana\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion 