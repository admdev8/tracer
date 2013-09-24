#!/bin/sh

echo running $0 

source ../must_be.sh

if [[ "$1" == *tracer64* ]]
then
	TEST_EXE=1_x64.exe
	OPTIONS=options_x64
	LOG=tracer64.log
else
	TEST_EXE=1.exe
	OPTIONS=options
	LOG=tracer.log
fi

rm -f output

$1 -l:$TEST_EXE @$OPTIONS --no-new-console > /dev/null

# check all
diff_must_be_successful output output_mustbe

# check BPX at p1
must_be "FPU ST(0): 0.693147" $LOG
must_be "FPU ST(1): 0.301030" $LOG
must_be "FPU ST(2): 3.141593" $LOG
must_be "FPU ST(3): 1.442695" $LOG
must_be "FPU ST(4): 3.321928" $LOG
must_be "FPU ST(5): 1.000000" $LOG

# check BPF at f2
must_be "00000000:       22    44       77-" $LOG

rm $LOG
rm -f output

