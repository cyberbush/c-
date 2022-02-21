#!/bin/bash
# Automated test for compiler
SRC=c-
TST_BANK="testDataA3"

# save all .c- files
# cd testDataA3
# files=(*.c-)  #*.c-
# cd ..

# test specific files
files=("allgood.c-")

#update build
make clean
make

# test each file
for filename in ${files[@]}; 
do
	echo $filename
	./$SRC -P $TST_BANK/$filename > test.out
	diff test.out $TST_BANK/${filename::-3}.out
done
