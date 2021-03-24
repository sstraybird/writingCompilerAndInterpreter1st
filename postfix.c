#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/
extern TOKEN_CODE token;
extern char token_string[];
extern BOOLEAN print_flag;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/
char postfix[MAX_PRINT_LINE_LENGTH]; /* buffer for postfix */
char *pp;                            /* ptr into postfix */

int main(int argc, char const *argv[])
{
    init_scanner(argv[1]);
    do
    {
        strcpy(postfix, ">> ");
        pp = postfix + strlen(postfix);
        get_token();
        expression();

        output_postfix("\n");
        print_line(postfix);

        while ((token != SEMICOLON) && (token != PERIOD) && (token != END_OF_FILE))
        {
            error(INVALID_EXPRESSION);
            get_token();
        }
    } while ((token != PERIOD) && (token != END_OF_FILE));
    return 0;
}

expression()
{
    simple_expression();
}

simple_expression()
{
    TOKEN_CODE op;   /* an operator token */
    char *op_string; /* an operator token string */

    term();

    while ((token == PLUS) || (token == MINUS))
    {
        op = token;
        get_token();
        term();
        switch (op)
        {
        case PLUS:
            op_string = "+";
            break;
        case MINUS:
            op_string = "-";
            break;
        }
        output_postfix(op_string);
    }
}

term()
{
    TOKEN_CODE op;   /* an operator token */
    char *op_string; /* an operator token string */

    factor();
    while ((token == STAR) || (token == SLASH))
    {
        op = token;
        get_token();
        factor();
        switch (op)
        {
        case STAR:
            op_string = "*";
            break;
        case SLASH:
            op_string = "/";
            break;
        }
        output_postfix(op_string);
    }
}

factor()
{
    if ((token == IDENTIFIER) || (token == NUMBER))
    {
        output_postfix(token_string);
        get_token();
    }
    else if (token == LPAREN)
    {
        get_token();
        expression();
        if(token == RPAREN)
            get_token();
        else
            error(MISSING_RPAREN);

    }
    else
        error(INVALID_EXPRESSION);
}

output_postfix(char *string)
{
    *pp++ = ' ';
    *pp = '\0';
    strcat(pp,string);
    pp += strlen(string);
}
