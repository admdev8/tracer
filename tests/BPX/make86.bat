@echo off
rem MSVS 2010
ml /Foasm_part_86.obj /c asm_part.asm
cl 1.c /Zi /Ob0 /MD asm_part_86.obj /Fa1.asm
