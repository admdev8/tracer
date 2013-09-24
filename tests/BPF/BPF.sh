#!/bin/sh

echo running $0 

source ../must_be.sh

if [[ "$1" == *tracer64* ]]
then
	TEST_EXE=test1_x64.exe
	OPTIONS1=options1_x64
	OPTIONS2=options2_x64
	LOG=tracer64.log
else
	TEST_EXE=test1.exe
	OPTIONS1=options1
	OPTIONS2=options2
	LOG=tracer.log
fi

$1 -l:$TEST_EXE @$OPTIONS1 --no-new-console > /dev/null
must_be "(0) $TEST_EXE!f1(\"asd\", \"def\") (called from $TEST_EXE!main" $LOG
must_be "(1) $TEST_EXE!f2(\"widestring1\", \"widestring2\") (called from $TEST_EXE!main" $LOG
must_be "61 73 64" $LOG
must_be "64 65 66" $LOG
must_be "(at the end) global=666" test1.output

$1 -l:$TEST_EXE @$OPTIONS2 --no-new-console > /dev/null
must_be "main - 12345" test1.output

rm $LOG
rm test1.output

