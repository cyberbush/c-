BIN=parser
EX=c-
CC=g++
STD=c++11

OBJS = lex.yy.c $(BIN).tab.c
OTHER = out.txt test.out

$(BIN) : $(OBJS)
	$(CC) -std=${STD} -g $(OBJS) -o $(EX) -O0

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(EX) lex.yy.c $(BIN).tar $(BIN).tab.h $(BIN).tab.c $(BIN).output $(OTHER) *.out

tar : 
	tar -cvf $(BIN).tar $(BIN).y $(BIN).l makefile *.cpp *.h

single :
	rm testfile.tm
	make
	./c- testDataA7/UnitTests/a07.c-
	cat testfile.tm

test :
	bash autotest.sh