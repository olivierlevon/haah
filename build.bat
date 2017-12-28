set src=%~dp0

call "C:\Program Files\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

cd /D %src%

cl /DWIN32 haah.c ws2_32.lib /link /SUBSYSTEM:CONSOLE /MACHINE:X86 

pause