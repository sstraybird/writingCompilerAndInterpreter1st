#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "parser.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/
extern TOKEN_CODE token;
extern char word_string[];

TOKEN_CODE rel_op_list[] = {LT, LE, EQUAL, NE, GE, GT, 0};
expression()
{
    TOKEN_CODE op;
    simple_expression();

    if (token_in(rel_op_list))
    {
        op = token;
        get_token();
        simple_expression();
    }
}

TOKEN_CODE add_op_list[] = {PLUS, MINUS, OR, 0};
simple_expression()
{
    TOKEN_CODE op;
    TOKEN_CODE unary_op = PLUS;

    if ((token == PLUS) || (token == MINUS))
    {
        unary_op = token;
        get_token();
    }
    term();

    while (token_in(add_op_list))
    {
        op = token;
        get_token();
        term();
    }
}

TOKEN_CODE mult_op_list[] = {STAR, SLASH, DIV, MOD, AND, 0};
term()
{
    TOKEN_CODE op;

    factor();

    while (token_in(mult_op_list))
    {
        op = token;
        get_token();
        factor();
    }
}

factor()
{
    switch (token)
    {
    case IDENTIFIER:
        get_token();
        break;
    case NUMBER:
        get_token();
        break;
    case STRING:
        get_token();
        break;
    case NOT:
        get_token();
        factor();
        break;
    case LPAREN:
        get_token();
        expression();
        if_token_get_else_error(RPAREN, MISSING_RPAREN);
        break;
    default:
        error(INVALID_EXPRESSION);
        break;
    }
}