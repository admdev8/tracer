cd ..\octothorpe
call cmp_MSVC_win32.bat
cd ..\porg
call cmp_MSVC_win32.bat
cd ..\x86_disasm
call cmp_MSVC_win32.bat
cd ..\bolt
call cmp_MSVC_win32.bat
cd ..\tracer
nmake all /f Makefile.msvc
