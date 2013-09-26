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

$1 -l:$TEST_EXE @$OPTIONS --no-new-console > /dev/null
must_be "Setting one-time INT3 breakpoint on msvcr100.dll!free" $LOG
must_be "Setting one-time INT3 breakpoint on msvcr100.dll!fwrite" $LOG
must_be "One-time INT3 breakpoint: MSVCR100.dll!fopen" $LOG
must_be "One-time INT3 breakpoint: MSVCR100.dll!fprintf" $LOG
must_be "One-time INT3 breakpoint: MSVCR100.dll!fclose" $LOG
must_be "called from $TEST_EXE!main" $LOG

must_not_be "!printf" $LOG

rm $LOG
rm output

