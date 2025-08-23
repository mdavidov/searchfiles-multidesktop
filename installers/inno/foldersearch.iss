
#include "../../src/version.hpp"

#define APP_VERSION     OvSk_FsOp_APP_VERSION_STR
#define APP_PUBLISHER   OvSk_FsOp_COPMANY_NAME_TXT
#define APP_COPYRIGHT   OvSk_FsOp_COPYRIGHT_TXT
#define APP_NAME        OvSk_FsOp_APP_NAME_TXT
#define APP_NM          OvSk_FsOp_APP_NM_TXT
#define APP_EXE         OvSk_FsOp_APP_EXE_TXT

#define APP_INPUT_DIR   "C:\build\foldersearch\desktop-qt5.5.1-vs2013-x64\release"
#define INPUT_3RDP_DIR  "C:\qt\qt5.5.1-vs2013-x64\5.5\msvc2013_64"

[Setup]
AppPublisher={#APP_PUBLISHER}
AppPublisherURL=OvSk_FsOp_COPMANY_URL_TXT
AppName={#APP_NAME}
AppVersion={#APP_VERSION}
DefaultDirName={pf}\{#APP_PUBLISHER}\{#APP_NAME} {#APP_VERSION}
DefaultGroupName={#APP_PUBLISHER}
UninstallDisplayIcon={app}\{#APP_EXE}
OutputDir=.
OutputBaseFilename={#APP_NM}-{#APP_VERSION}-setup
Compression=lzma2
SolidCompression=yes
VersionInfoCompany={#APP_PUBLISHER}
VersionInfoProductName={#APP_NAME}
VersionInfoProductVersion={#APP_VERSION}
VersionInfoProductTextVersion={#APP_VERSION}
VersionInfoVersion={#APP_VERSION}
VersionInfoTextVersion={#APP_VERSION}
VersionInfoDescription={#APP_NAME} {#APP_VERSION} Setup
VersionInfoCopyright={#APP_COPYRIGHT}

[Files]
Source: "{#APP_INPUT_DIR}\{#APP_EXE}"; DestDir: "{app}"

Source: "{#INPUT_3RDP_DIR}\bin\Qt5Core.dll"; DestDir: "{app}"
Source: "{#INPUT_3RDP_DIR}\bin\Qt5Gui.dll"; DestDir: "{app}"
Source: "{#INPUT_3RDP_DIR}\bin\Qt5Widgets.dll"; DestDir: "{app}"
Source: "{#INPUT_3RDP_DIR}\plugins\platforms\qwindows.dll"; DestDir: "{app}\platforms"
Source: "{#INPUT_3RDP_DIR}\bin\icudt54.dll"; DestDir: "{app}"
Source: "{#INPUT_3RDP_DIR}\bin\icuin54.dll"; DestDir: "{app}"
Source: "{#INPUT_3RDP_DIR}\bin\icuuc54.dll"; DestDir: "{app}"

; --- Source: "ginfo.chm"; DestDir: "{app}"
; --- Source: "Readme.txt"; DestDir: "{app}"; Flags: isreadme

[Icons]
Name: "{group}\{#APP_NAME} {#APP_VERSION}"; Filename: "{app}\{#APP_EXE}"; WorkingDir: "{app}"
Name: "{userdesktop}\{#APP_NAME} {#APP_VERSION}"; Filename: "{app}\{#APP_EXE}"; WorkingDir: "{app}"
Name: "{group}\Uninstall {#APP_NAME} {#APP_VERSION}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\{#APP_EXE}"; Description: "Launch Application"; Flags: postinstall nowait skipifsilent
; --- Filename: "{app}\README.TXT"; Description: "View the README file"; Flags: postinstall shellexec skipifsilent
