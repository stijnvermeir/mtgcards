cd %~dp0
rmdir /S /Q deploy
mkdir deploy
cd deploy
copy ..\mtgcards\build\Desktop_Qt_6_9_0_MSVC2022_64bit-Release\release\MTGCards.exe .
set PATH=%SystemDrive%\Qt\6.9.0\msvc2022_64\bin
windeployqt --verbose 8 MTGCards.exe
REM "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" ..\setup.iss
pause
