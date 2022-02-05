#!/bin/bash
# Automated test for compiler
SRC=c-
TST_BANK="testDataA2"

# save .c- files
cd testDataA2
files=(*.c-)
cd ..

#update build
make clean
make

# test each file
for filename in ${files[@]}; 
do
	echo $filename
	./$SRC -p $TST_BANK/$filename > test.out
	diff test.out $TST_BANK/${filename::-3}.out
done
