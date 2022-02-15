//------------------ main.cpp ------------------
#include "ourgetopt.cpp"
#include "main.h"

// Declare stuff from Flex that Bison needs to know about
extern FILE *yyin;          // input source file
extern int yylex();
extern int yyparse();       // Start Parser
extern int yydebug;         // yydebug flag
extern int errNum;          // number of errors
extern int warnNum;          // number of warnings
extern AST_Node* root;      // root of AST tree

// Open file and handle options
int main( int argc, char *argv[] )				
{								
	int c, args=0; 
    int Dflag=0, pflag=0, Pflag=0;				
	while ((c = ourGetopt(argc, argv, (char *)"dDpPh:")) != -1)
    {
        args++;
		switch ( c ) {	
        case 'd':                                     
			yydebug = 1; 	
            break;		
		case 'D':                                     
			++Dflag; 	
            break;
        case 'p':                                     
			++pflag; 	
            break;		
		case 'P':                                     
			++Pflag; 	
            break;		
        case 'h':
            printUsage();
            break;	
		default:					
		    return 1;		
        }                                                       
    }

    // open the file
    if (argc > 1) {
        int file = args + 1;
        if ((yyin = fopen(argv[file], "r"))) {
            // file open successful
        }
        else {
            // failed to open file
            printf("ERROR: failed to open \'%s\'\n", argv[file]);
            exit(1);
        }
    }

    yyparse();
    
    if (errNum == 0)
    {
        // check -p option
        if (pflag){
            // print only info for declarations
            printAST(root, -1, 0);
        }
        //symbolTable = new SymbolTable; // instantiate the symbol table
        // perform semantic analysis (may find errors when doing this)
        // semanticAnalysis(syntaxTree, symbolTable);
        
        // check -P option
        // if (Pflag)
            // print type info for all types
            //printTree(syntaxTree, TYPES);
        
        // code generation will eventually go here...
    }

    // report number of errors and warnings
    //printf("Number of errors: %d\n", errNum);
    //printf("Number of warnings: %d\n", warnNum);

    return 0;
}                                                               


/*int main(int argc, char *argv[])
{
    bool printTreeFlag = false;
    char option = '0';

    // check for options
    for(int i = 1; i < argc-1; i++) {
        int str_size = strlen(argv[i]);
        if(argv[i][0] == '-' && str_size == 2) {
            option = argv[i][1];
        }
    }

    switch(option) {
        case 'p': // print tree
            printTreeFlag = true;
            break;
        case 'd': // enable debugging
            yydebug = 1;
            break;
        case '0': // no option
            break;
        default:  // error
            printf("Error inputing option: %c\n", option);
    }

    if (argc > 1) {
        if ((yyin = fopen(argv[argc-1], "r"))) {
            // file open successful
        }
        else {
            // failed to open file
            printf("ERROR: failed to open \'%s\'\n", argv[1]);
            exit(1);
        }
    }
    yyparse();
    
    // print tree if true
    if(printTreeFlag) {
        printAST(root, -1, 0);
    }
}
*/