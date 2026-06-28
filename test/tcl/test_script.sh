#!/bin/bash

OCC=$1
TEST_FOLDER=$2

$OCC -file $TEST_FOLDER/script.tcl > stdout.txt 2> stderr.txt

echo "Comparing standard output:"
diff -I "Executing file:" stdout.txt $TEST_FOLDER/stdout.txt
rv=$?
if [ "$rv" -ne 0 ];
then
    echo "Standard output not equal"
    exit $rv
fi

echo "Comparing error output:"
diff stderr.txt $TEST_FOLDER/stderr.txt
rv=$?
if [ "$rv" -ne 0 ];
then
    echo "Error output not equal"
    exit $rv
fi

exit 0