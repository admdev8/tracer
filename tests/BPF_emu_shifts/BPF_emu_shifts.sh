#!/bin/sh

echo running $0 

source ../must_be.sh

if [[ "$1" == *tracer64* ]]
then
	TEST_EXE=1_x64.exe
	OPTIONS1=options1_x64
	OPTIONS2=options2_x64
	LOG=tracer64.log
else
	TEST_EXE=1.exe
	OPTIONS1=options1
	OPTIONS2=options2
	LOG=tracer.log
fi

rm -f output

$1 -l:$TEST_EXE @$OPTIONS1 --no-new-console > /dev/null
diff_must_be_successful output output_mustbe

rm -f output

$1 -l:$TEST_EXE @$OPTIONS2 --no-new-console > /dev/null
diff_must_be_successful output output_mustbe

rm $LOG
rm -f output

