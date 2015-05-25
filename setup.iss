; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{399B5FDC-84F4-4E33-8EE2-85FE31F1F6B2}
AppName=MTGCards
AppVersion=0.1.0
;AppVerName=MTGCards 0.1.0
AppPublisher=stijnvsoftware
AppPublisherURL=http://www.stijn-v.be
AppSupportURL=http://www.stijn-v.be
AppUpdatesURL=http://www.stijn-v.be
DefaultDirName={pf}\MTGCards
DefaultGroupName=MTGCards
AllowNoIcons=yes
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\MTGCards.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\icudt53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\icuin53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\icuuc53.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\libEGL.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\libGLESV2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\opengl32sw.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_ca.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_cs.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_de.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_fi.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_hu.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_it.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_ja.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_ru.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_sk.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\qt_uk.qm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\Qt5Svg.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\iconengines\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\imageformats\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Users\Stijn\git\mtgcards\build\deploy\platforms\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\MTGCards"; Filename: "{app}\MTGCards.exe"
Name: "{group}\{cm:UninstallProgram,MTGCards}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\MTGCards"; Filename: "{app}\MTGCards.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\MTGCards"; Filename: "{app}\MTGCards.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\MTGCards.exe"; Description: "{cm:LaunchProgram,MTGCards}"; Flags: nowait postinstall skipifsilent

