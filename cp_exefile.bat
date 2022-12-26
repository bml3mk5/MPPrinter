md .\bin\vc2010\Win32\
copy /b ".\VC++2010\Release\Win32\mpprinter.exe" .\bin\vc2010\Win32\
md .\bin\vc2010\x64\
copy /b ".\VC++2010\Release\x64\mpprinter.exe" .\bin\vc2010\x64\
: md .\bin\mingw32\
: copy /b .\ReleaseM\mpprinter.exe .\bin\mingw32\

pause
