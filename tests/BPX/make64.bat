@echo off
rem MSVS 2010
ml64 /Foasm_part_64.obj /c asm_part.asm
cl 1.c /Ob0 /MD /Zi asm_part_64.obj /Fe1_x64.exe /Fa1_x64.asm
