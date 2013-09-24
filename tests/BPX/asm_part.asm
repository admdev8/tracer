IFDEF RAX ; --------------------------- x86-64

EXTRN	_fltused:DWORD

PUBLIC	f

_TEXT	SEGMENT

f PROC

	finit
	fld1
	fldl2t
	fldl2e
	fldpi
	fldlg2
	fldln2 ; ln(2)
	;fsave [rcx]
	;fld1
	fst qword ptr [rdx]

	ret	0

f ENDP

ELSE ; --------------------------- x86

	.686P
	.XMM

_TEXT	SEGMENT

EXTRN	__fltused:DWORD

PUBLIC	_f

_f PROC

	finit
	fld1
	fldl2t
	fldl2e
	fldpi
	fldlg2
	fldln2 ; ln(2)
	;mov ebx, [esp+4]
	;fsave [ebx]
	;fld1
	mov ebx, [esp+8]
	fst qword ptr [ebx]

	ret	0
_f ENDP

ENDIF

_TEXT	ENDS
	END
