#include <stdio.h>
#include "common.h"
#include "scanner.h"

#define MAX_OUTPUT_RECORD_LENGTH    80


typedef enum{
    DELIMITER,NONDELIMITER,
}TOKEN_CLASS;


/*-------------------------------------------*/
/* Externals                                 */
/*-------------------------------------------*/
extern BOOLEAN print_flag;
extern TOKEN_CODE   token;
extern char     token_string[];

/*--------------------------------------------------------------*/
/*  全局变量                                                    */
/*--------------------------------------------------------------*/
int     record_length;          /* length of output record */
char*   recp;
char output_record[MAX_OUTPUT_RECORD_LENGTH] ;





int main(int argc, char const *argv[])
{
    TOKEN_CLASS class;                  //当前token class
    TOKEN_CLASS prev_class;             //previous token class
    TOKEN_CLASS token_class();

    print_flag = FALSE;
    init_scanner(argv[1]);

    //初始化
    prev_class = DELIMITER;
    recp = output_record;
    *recp = '\0';
    record_length = 0;

    
    do{
        get_token();
        if(token == END_OF_FILE)
            break;
        class = token_class();
        if((prev_class == NONDELIMITER) && (class = NONDELIMITER))
            append_blank();
        append_token();
        prev_class = class;
    }while (token != PERIOD);
    

    if(record_length > 0)
        flush_output_record();
    quit_scanner();
}

append_blank()
{
    if(++record_length == MAX_OUTPUT_RECORD_LENGTH -1)
        flush_output_record();
    else
        strcat(output_record," ");
}

append_token()
{
    int token_length; 
    token_length = strlen(token_string); 
    if(record_length + token_length >= MAX_OUTPUT_RECORD_LENGTH - 1)
        flush_output_record();
    strcat(output_record,token_string);
    record_length += token_length;         
}

TOKEN_CLASS token_class()
{
    /*
    --  Nondelimiters:	identifiers, numbers, and reserved words
    --  Delimiters:	strings and special symbols
    */
    switch (token)
    {
    case IDENTIFIER:
    case NUMBER:
        return NONDELIMITER;
        break;
    
    default:
        return (token <AND ? DELIMITER:NONDELIMITER) ;
    }
}

flush_output_record()
{
    printf("%s\n",output_record);
    recp = output_record;
    *recp = '\0';
    record_length = 0;
}