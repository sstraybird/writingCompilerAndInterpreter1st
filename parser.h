#ifndef parser_h
#define parser_h

#include    "common.h"
#include    "symtab.h"


#define if_token_get_else_error(token_code,error_code)  \
    if(token == token_code)     \
        get_token();            \
    else                        \
        error(error_code)       \

#endif