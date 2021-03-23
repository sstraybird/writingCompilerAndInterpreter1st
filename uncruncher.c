#include <stdio.h>
#include "common.h"
#include "scanner.h"

#define MAX_OUTPUT_RECORD_LENGTH 80

typedef enum
{
    DELIMITER,
    NONDELIMITER,
} TOKEN_CLASS;

/*--------------------------------------------------------------*/
/*  Globals							*/
/*--------------------------------------------------------------*/
FILE *crunch_file;
char token_string[MAX_TOKEN_STRING_LENGTH];
char output_record[MAX_OUTPUT_RECORD_LENGTH];

TOKEN_CODE ctoken;
int record_length;
char *recp;
char **symtab_strings;

char *symbol_strings[] = {
    "<no token>",
    "<IDENTIFIER>",
    "<NUMBER>",
    "<STRING>",
    "^",
    "*",
    "(",
    ")",
    "-",
    "+",
    "=",
    "[",
    "]",
    ":",
    ";",
    "<",
    ">",
    ",",
    ".",
    "/",
    ":=",
    "<=",
    ">=",
    "<>",
    "..",
    "<END OF FILE>",
    "<ERROR>",
    "AND",
    "ARRAY",
    "BEGIN",
    "CASE",
    "CONST",
    "DIV",
    "DO",
    "DOWNTO",
    "ELSE",
    "END",
    "FILE",
    "FOR",
    "FUNCTION",
    "GOTO",
    "IF",
    "IN",
    "LABEL",
    "MOD",
    "NIL",
    "NOT",
    "OF",
    "OR",
    "PACKED",
    "PROCEDURE",
    "PROGRAM",
    "RECORD",
    "REPEAT",
    "SET",
    "THEN",
    "TO",
    "TYPE",
    "UNTIL",
    "VAR",
    "WHILE",
    "WITH",
};

TOKEN_CLASS token_class();
int main(int argc, char const *argv[])
{
    TOKEN_CLASS class;
    TOKEN_CLASS prev_class;

    //打开crunch文件
    crunch_file = fopen(argv[1], "rb");
    if (crunch_file == NULL)
    {
        printf("*** Error: Failed to open crunch file.\n");
        exit(-2);
    }

    //初始化uncruncher
    prev_class = DELIMITER;
    recp = output_record;
    *recp = '\0';
    record_length = 0;

    //读取crunched symbol table
    read_crunched_symtab();
    do
    {
        get_ctoken();
        if (ctoken == END_OF_FILE)
            break;
        class = token_class();
        if ((prev_class == NONDELIMITER) && (class == NONDELIMITER))
            append_blank();
        append_token();
        prev_class = class;
    } while (ctoken != PERIOD);

    if (record_length > 0)
        flush_output_record();
    return 0;
}

read_crunched_symtab()
{
    short count;
    short index;
    char length;

    //读取symbol table entries的个数
    //并且分配
    fread(&count, sizeof(short), 1, crunch_file);
    symtab_strings = (char **)alloc_bytes(count * sizeof(char *));

    do
    {
        fread(&index, sizeof(short), 1, crunch_file);
        fread(&length, sizeof(char), 1, crunch_file);
        symtab_strings[index] = alloc_bytes(length);
        fread(symtab_strings[index], length, 1, crunch_file);
    } while (--count > 0);
}

get_ctoken()
{
    fread(&ctoken, sizeof(char), 1, crunch_file);
    switch (ctoken)
    {
    case IDENTIFIER:
    case NUMBER:
    case STRING:
    {
        short index;
        fread(&index, sizeof(short), 1, crunch_file);
        strcpy(token_string, symtab_strings[index]);
        break;
    }

    default:
        strcpy(token_string, symbol_strings[ctoken]);
        break;
    }
}

TOKEN_CLASS token_class()
{
    switch (ctoken)
    {
    case IDENTIFIER:
    case NUMBER:
        return (NONDELIMITER);

    default:
        return (ctoken < AND ? DELIMITER : NONDELIMITER);
    }
}

append_blank()

{
    if (++record_length == MAX_OUTPUT_RECORD_LENGTH - 1)
        flush_output_record();
    else
        strcat(output_record," ");
}

append_token()
{
    int token_length; /* length of token string */

    token_length = strlen(token_string);
    if (record_length + token_length >= MAX_OUTPUT_RECORD_LENGTH - 1)
        flush_output_record();

    strcat(output_record, token_string);
    record_length += token_length;
}

flush_output_record()
{
    printf("%s\n", output_record);
    recp = output_record;
    *recp = '\0';
    record_length = 0;
}
