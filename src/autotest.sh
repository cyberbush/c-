#!/bin/bash
# Automated test for compiler
SRC=c-
TST_BANK="testDataA7/BroadTests"

# save all .c- files
# cd testDataA7/UnitTests
# files=(i*.c-)
# cd ..
# cd ..

# test specific files
files=("factor2.c-")

#update build
make clean
make

# test each file
for filename in ${files[@]}; 
do
	echo $filename
	#./$SRC -M $TST_BANK/$filename > test.out
	#diff -y <(sort test.out) <(sort $TST_BANK/${filename::-3}.out) -W 210 # > result.out
	./$SRC $TST_BANK/$filename
	diff -y "test.tm" $TST_BANK/${filename::-2}tm
	rm test.tm 
done
