cd %~dp0
cd build
rmdir /S /Q deploy
mkdir deploy
cd deploy
copy ..\32bit\release\MTGCards.exe .
set PATH=%SystemDrive%\Qt\5.4\msvc2013\bin\
windeployqt --verbose 8 --no-compiler-runtime MTGCards.exe
copy %SystemRoot%\SysWOW64\msvcp120.dll .
copy %SystemRoot%\SysWOW64\msvcr120.dll .
copy %SystemDrive%\OpenSSL-Win32\libeay32.dll .
copy %SystemDrive%\OpenSSL-Win32\ssleay32.dll .
copy %SystemDrive%\OpenSSL-Win32\license.txt license-openssl.txt
"%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe" ..\..\setup.iss
pause
