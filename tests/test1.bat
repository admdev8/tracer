..\tracer -l:test1_BPF.exe bpf=msvcrt.dll!printf,args:2
rem ..\tracer -l:test1_BPF.exe bpf=msvcrt.dll!printf,args:2 -dont-run-thread-b
rem ..\tracer -l:test1_BPF.exe bpf=msvcrt.dll!printf,args:2,trace
rem ..\tracer -l:test1_BPF.exe bpf=test1_BPF.exe!OEP,trace:cc
..\tracer -l:test1_BPF.exe bpf=msvcrt.dll!printf,args:2 bpf=test1_BPF.exe!0x004013B0,args:1,dump_args:0x80
