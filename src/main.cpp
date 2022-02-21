//------------------ main.cpp ------------------
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
        if (pflag) printAST(root, -1, 0, false);

        // perform semantic analysis (may find errors when doing this)
        SemanticAnalyzer semanticAnalyzer;
        // check -D option
        if(Dflag) semanticAnalyzer.analyzeTree(root, true);
        else semanticAnalyzer.analyzeTree(root, false);
        // check -P option
        if (Pflag && errNum == 0)
            //print type info for all types
            printAST(root, -1, 0, true);
        
        // code generation will eventually go here...
    }

    // report number of errors and warnings
 
    printf("Number of warnings: %d\n", warnNum);
    printf("Number of errors: %d\n", errNum);

    return 0;
}                                                               