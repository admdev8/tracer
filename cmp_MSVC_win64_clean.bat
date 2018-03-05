cd ..\octothorpe
call cmp_MSVC_win64_clean.bat
cd ..\porg
call cmp_MSVC_win64_clean.bat
cd ..\x86_disasm
call cmp_MSVC_win64_clean.bat
cd ..\bolt
call cmp_MSVC_win64_clean.bat
cd ..\tracer
nmake clean /f Makefile64.msvc
