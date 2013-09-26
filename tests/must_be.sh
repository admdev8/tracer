#!/bin/sh

function must_be
{
	if grep --quiet "$1" $2 > /dev/null; then
		true
	else
		echo [$1] not found in $2. exiting
		exit 1;
	fi
}

function must_not_be
{
	if grep --quiet "$1" $2 > /dev/null; then
		echo [$1] found in $2 but should not. exiting
		exit 1;
	else
		true
	fi
}

function diff_must_be_successful
{
	diff $1 $2

	if [[ $? -ne 0 ]]
	then
		echo results are different in [$1] and [$2]. exiting.
		exit 0;
	fi
}
