#!/bin/sh

echo running $0 

source ../must_be.sh

if [[ "$1" == *tracer64* ]]
then
	TEST_EXE=1_x64.exe
	OPTIONS=options_x64
	LOG=tracer64.log
	OFS1="+0x8"
	OFS2="+0x18"
else
	TEST_EXE=1.exe
	OPTIONS=options
	LOG=tracer.log
	OFS1="+0x4"
	OFS2="+0xC"
fi

$1 -l:$TEST_EXE @$OPTIONS --no-new-console > /dev/null

must_be "Argument 1/1 $OFS1: $TEST_EXE!global_var_1" $LOG
must_be "Argument 1/1 $OFS2: $TEST_EXE!global_var_2" $LOG
must_be "Argument 1/1 before $OFS2: $TEST_EXE!global_var_2" $LOG
must_be "Argument 1/1 after $OFS2: $TEST_EXE!global_var_3" $LOG

rm $LOG

