cd ..\octothorpe
call cmp_MSVC_win64.bat
cd ..\porg
call cmp_MSVC_win64.bat
cd ..\x86_disasm
call cmp_MSVC_win64.bat
cd ..\bolt
call cmp_MSVC_win64.bat
cd ..\tracer
nmake all /f Makefile64.msvc
