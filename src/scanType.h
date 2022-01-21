#ifndef _SCANTYPE_H_
#define _SCANTYPE_H_

#include <string>
#include <stdbool.h>

struct TokenData {
	int tokenClass; // integer representing id, assigned by bison
	int line; // line number associated with token
	char *tokenString; // full string of token

	int nValue; // any numeric or bool value
	char cValue; // any character value
	std::string sValue; // any string value
};

#endif
