#ifndef scanner_h
#define scanner_h

#include "common.h"

/*-------------------------------------------------*
/*Token 编码
/*-------------------------------------------------*/
typedef enum{
    NO_TOKEN, IDENTIFIER, NUMBER, STRING,
    UPARROW, STAR, LPAREN, RPAREN, MINUS, PLUS, EQUAL,
    LBRACKET, RBRACKET, COLON, SEMICOLON, LT, GT, COMMA, PERIOD,
    SLASH, COLONEQUAL, LE, GE, NE, DOTDOT, END_OF_FILE, ERROR,
    AND, ARRAY, BEGIN, CASE, CONST, DIV, DO, DOWNTO, ELSE, END,
    FFILE, FOR, FUNCTION, GOTO, IF, IN, LABEL, MOD, NIL, NOT,
    OF, OR, PACKED, PROCEDURE, PROGRAM, RECORD, REPEAT, SET,
    THEN, TO, TYPE, UNTIL, VAR, WHILE, WITH,
}TOKEN_CODE;

/*-------------------------------------------------*
/*字面类型
/*-------------------------------------------------*/
typedef enum{
    INTEGER_LIT,REAL_LIT,STRING_LIT,
} LITERAL_TYPE;

typedef struct 
{
    LITERAL_TYPE    type;
    union{
        int integer;
        float real;
        char string[MAX_SOURCE_LINE_LENGTH];    
    }value;
}LITERAL;


#endif