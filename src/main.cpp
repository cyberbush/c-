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
FILE *code;
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
            // int filesize = strlen(argv[file]);
            // char *filename = strdup(argv[file]);
            // filename[filesize-2] = 't';
            // filename[filesize-1] = 'm';
            // code = fopen(filename, "a"); // file for emitcode
            code = fopen("test.tm", "a"); // file for emitcode
            //printf("File Name: %s\tsize: %d\n", filename, filesize);
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

        int goffset = semanticAnalyzer.getgoffset();
;

        if(errNum == 0) { // memory allocation if no errors
        
            // add memory information

            if(Pflag) { // check -P option
                if(Mflag) { // check -M option
                    printASTAugmented(root, -1, 0); // print augmented tree
                    printf("Offset for end of global space: %d\n", goffset);
                }
                else { // print annotated tree
                    printAST(root, -1, 0, true);
                }
            }
        }
        if(errNum == 0) { // code generation 
            generateCode(root, goffset, semanticAnalyzer.getSymbolTable());

        }
    }

    // report number of errors and warnings
 
    printf("Number of warnings: %d\n", warnNum);
    printf("Number of errors: %d\n", errNum);

    return 0;
}                                                               