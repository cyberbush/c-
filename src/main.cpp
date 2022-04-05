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
    int Dflag=0, pflag=0, Pflag=0, Mflag=0;				
    initErrorProcessing(); 
	while ((c = ourGetopt(argc, argv, (char *)"dDpPhM:")) != -1)
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
        case 'M':
            ++Pflag;
            ++Mflag;
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
            yyparse();
        }
        else {
            // failed to open file
            printf("ERROR(ARGLIST): source file \"%s\" could not be opened.\n", argv[file]);
            errNum++;
            //exit(1);
        }
    }

    
    if (errNum == 0)
    {
        // check -p option
        if (pflag) printAST(root, -1, 0, false);

        // perform semantic analysis (may find errors when doing this)
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.analyzeTree(root, Dflag);

        if(errNum == 0) { // memory allocation if no errors
        
            int goffset = 0;
            // add memory information

            if(Pflag) { // check -P option
                if(Mflag) { // check -M option
                    printASTAugmented(root, -1, 0); // print augmented tree
                    goffset = semanticAnalyzer.getgoffset();
                    printf("Offset for end of global space: %d\n", goffset);
                }
                else { // print annotated tree
                    printAST(root, -1, 0, true);
                }
            }
        }
        // code generation will eventually go here...
    }

    // report number of errors and warnings
 
    printf("Number of warnings: %d\n", warnNum);
    printf("Number of errors: %d\n", errNum);

    return 0;
}                                                               