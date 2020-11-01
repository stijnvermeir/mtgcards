cd %~dp0
rmdir /S /Q deploy
mkdir deploy
cd deploy
copy ..\build-release\release\MTGCards.exe .
set PATH=%SystemDrive%\Qt\5.15.0\msvc2019\bin\
windeployqt --verbose 8 MTGCards.exe
"%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" ..\setup.iss
pause
