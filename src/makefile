BIN = parser
EX = c-
CC = g++

SRCS = $(BIN).y  $(BIN).l main.cpp printTree.cpp utils.cpp treeUtil.cpp ourgetopt.cpp symbolTable.cpp
HDRS = main.h scanType.h AST_Node.h utils.h treeUtil.h printTree.h ourgetopt.h symbolTable.h
OBJS = lex.yy.c $(BIN).tab.c
OTHER = out.txt test.out

$(BIN) : $(OBJS)
	$(CC) -g $(OBJS) -o $(EX)

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(EX) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).output $(OTHER)

tar : $(HDR) $(SRCS) makefile
	tar -cvf $(BIN).tar $(HDRS) $(SRCS) makefile

test :
	bash autotest.sh