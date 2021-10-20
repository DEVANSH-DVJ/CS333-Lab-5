#!/bin/bash
if [ "$#" -ne 2 ]; then 
    echo "Usage: ./autograder.sh testcases expected_output"
    exit
fi
echo "Testcases: $1"
echo "Expected output: $2"
TESTDIR=$1
EXPECTED=$2

#generate output
OUTDIR=myoutput
rm -rf $OUTDIR
mkdir $OUTDIR
for filename in $TESTDIR/*.c; do
    name=`basename $filename`
    name="${name%.*}"
    outfile=$OUTDIR/$name.out
    echo "Running testcase $filename: Output stored in $outfile"
    cp $filename testcase.c
    gcc testcase.c simplefs-ops.c simplefs-disk.c
    ./a.out > $outfile
    rm -f testcase.c
    rm -f a.out
done



#compare
correct=0
total=0
rm -rf myout
for expected in $EXPECTED/*.out; do
    name=`basename $expected`
    actual=$OUTDIR/$name
    total=$((total+1))
    echo "Comparing $expected and $actual"
    diff -w -B $expected $actual
		if [ "$?" == "0" ]; then
			echo "Test Case Passed"
			correct=$((correct+1))
		else
			echo "Test Case Failed"
		fi
		echo ""
		echo ""
done
echo "Test Cases Passed: $correct"
echo "Test Cases Total: $total"

