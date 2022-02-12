BIN = parser
EX = c-
CC = g++

SRCS = $(BIN).y  $(BIN).l printTree.cpp utils.cpp treeUtil.cpp
HDRS = scanType.h AST_Node.h utils.h treeUtil.h printTree.h
OBJS = lex.yy.c $(BIN).tab.c

$(BIN) : $(OBJS)
	$(CC) -g $(OBJS) -o $(EX)

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(EX) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).output out.txt

tar : $(HDR) $(SRCS) makefile
	tar -cvf $(BIN).tar $(HDRS) $(SRCS) makefile

test :
	bash autotest.sh