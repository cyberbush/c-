#ifndef _YYERROR_H_
#define _YYERROR_H_

// write a nice error message
#define YYERROR_VERBOSE

extern int lineNum;        // line number of last token scanned in your scanner (.l)
extern char *lastToken; // last token scanned in your scanner (connect to your .l)
extern int errNum;   // number of errors

void initErrorProcessing();    // WARNING: MUST be called before any errors occur (near top of main)!
void yyerror(const char *msg); // error routine called by Bison

#endif
