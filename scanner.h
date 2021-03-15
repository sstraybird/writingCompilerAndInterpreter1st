#ifndef scanner_h
#define scanner_h

#include "common.h"

/*-------------------------------------------------*
/*Token 编码
/*-------------------------------------------------*/
typedef enum{
    NO_TOKEN,WORD,NUMBER,PERIOD,END_OF_FILE,ERROR,
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