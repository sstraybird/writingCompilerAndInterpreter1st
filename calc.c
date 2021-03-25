#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "symtab.h"

#define STACK_SIZE 32

extern TOKEN_CODE       token;
extern char             token_string[];
extern char             word_string[];
extern SYMTAB_NODE_PTR  symtab_root;
extern LITERAL          literal;
extern BOOLEAN          print_flag;

/*--------------------------------------------------------------*/
/*  Globals							*/
/*--------------------------------------------------------------*/
float stack[STACK_SIZE]; /* evaluation stack */
float *tos = stack;      /* top of stack pointer */

/*--------------------------------------------------------------*/
/*  pop                 Return the value popped off the stack.  */
/*--------------------------------------------------------------*/
#define pop() *tos--

int main(int argc, char const *argv[])
{
    init_scanner(argv[1]);

    get_token();

    do
    {
        if (token == IDENTIFIER)
            assignment_statement();

        while ((token != SEMICOLON) && (token != PERIOD) && (token != END_OF_FILE))
        {
            error(INVALID_ASSIGNMENT);
            get_token();
        }
        while (token == SEMICOLON)
            get_token();
    } while ((token != PERIOD) && (token != END_OF_FILE));
}

assignment_statement()
{
    SYMTAB_NODE_PTR np;  /* ptr to symtab node */
    float *vp;           /* ptr to value */
    BOOLEAN output_flag; /* TRUE if assign to "output" */

    /*
    --  Look for the identifier.
    */
    if (token != IDENTIFIER)
    {
        error(MISSING_IDENTIFIER);
        return;
    }
    /*
    --  Enter the identifier into the symbol table
    --  unless it is "output".
    */
    output_flag = strcmp(word_string, "output") == 0;
    if (!output_flag)
    {
        if ((np = search_symtab(word_string, symtab_root)) == NULL)
        {
            np = enter_symtab(word_string, &symtab_root);
            np->defn.key = VAR_DEFN;
        }
    }

    /*
    --	Look for the := .
    */
    get_token();
    if (token != COLONEQUAL)
    {
        error(MISSING_COLONEQUAL);
        return;
    }
    get_token();

    /*
    --  Process the expression.
    */
    expression();

    /*
    --  Assign the expression value to the identifier.  If
    --  the identifer is "output", print the value instead.
    */
    if (output_flag)
    {

        printf(">> output: %0.6g \n", pop());
    }
    else
    {
        vp = alloc_struct(float);
        *vp = pop();
        np->info = (char *)vp;

    }
}

expression()
{
    TOKEN_CODE op;              /* an operator token */
    float operand_1, operand_2; /* operand values */

    simple_expression();

    if ((token == EQUAL) || (token == LT) || (token == GT) || (token == NE) || (token == LE) || (token == GE))
    {
        op = token;
        get_token();
        simple_expression();

        operand_2 = pop();
        operand_1 = pop();

        switch (op)
        {
        case EQUAL:
            push(operand_1 == operand_2 ? 1.0 : 0.0);
            break;
        case LT:
            push(operand_1 < operand_2 ? 1.0 : 0.0);
            break;
        case GT:
            push(operand_1 > operand_2 ? 1.0 : 0.0);
            break;
        case NE:
            push(operand_1 != operand_2 ? 1.0 : 0.0);
            break;
        case LE:
            push(operand_1 <= operand_2 ? 1.0 : 0.0);
            break;
        case GE:
            push(operand_1 >= operand_2 ? 1.0 : 0.0);
            break;
        }
    }
}

simple_expression()
{
    TOKEN_CODE op;              /* an operator token */
    TOKEN_CODE unary_op = PLUS; /* a unary operator token */
    float operand_1, operand_2; /* operand values */

    if ((token == PLUS) || (token == MINUS))
    {
        unary_op = token;
        get_token();
    }

    term();

    /*
    --  If there was a unary -, negate the top of stack value.
    */
    if (unary_op == MINUS)
        *tos = -(*tos);

    while ((token == PLUS) || (token == MINUS) || (token == OR))
    {
        op = token;
        get_token();
        term();

        operand_2 = pop();
        operand_1 = pop();

        /*
        --  ... and perform the operation, leaving the
        --  value on top of the stack.
        */
        switch (op)
        {
        case PLUS:
            push(operand_1 + operand_2);
            break;
        case MINUS:
            push(operand_1 - operand_2);
            break;
        case OR:
            push((operand_1 != 0.0) || (operand_2 != 0.0) ? 1.0 : 0.0);
            break;
        }
    }
}

term()
{
    TOKEN_CODE op;
    float operand_1, operand_2;

    factor();
    while ((token == STAR) || (token == SLASH) || (token == DIV) || (token == MOD) || (token == AND))
    {
        op = token;
        get_token();
        factor();

        operand_2 = pop();
        operand_1 = pop();

        switch (op)
        {
        case STAR:
            push(operand_1 * operand_2);
            break;
        case SLASH:
            if (operand_2 != 0.0)
                push(operand_1 / operand_2);
            else
            {
                printf("*** Warning: division by zero.\n");
                push(0.0);
            }
            break;
        case DIV:
            if (operand_2 != 0.0)
                push((float)(((int)operand_1) / ((int)operand_2)));
            else
            {
                printf("*** Warning: division by zero.\n");
                push(0.0);
            }
            break;
        case MOD:
            if (operand_2 != 0.0)
                push((float)(((int)operand_1) & ((int)operand_2)));
            else
            {
                printf("*** Warning: division by zero.\n");
                push(0.0);
            }
            break;
        case AND:
            push((operand_1 != 0.0) && (operand_2 != 0.0) ? 1.0 : 0.0);
            break;
        }
    }
}

factor()
{
    SYMTAB_NODE_PTR np;
    switch (token)
    {
    case IDENTIFIER:
        np = search_symtab(word_string, symtab_root);
        if (np != NULL)
            push(*((float *)np->info));
        else
        {
            error(UNDEFINED_IDENTIFIER);
            push(0.0);
        }
        get_token();
        break;
    case NUMBER:
        push(literal.type == INTEGER_LIT ? (float)literal.value.integer : literal.value.real);
        get_token();
        break;
    case NOT:
        get_token();
        factor();
        *tos = *tos == 0.0 ? 1.0 : 0.0; /* NOT tos */
        break;
    case LPAREN:
        get_token();
        expression();
        if (token == RPAREN)
            get_token();
        else
            error(MISSING_RPAREN);
        break;
    default:
        error(INVALID_EXPRESSION);
        break;
    }
}

push( value)
float value;            //如果写成push(float value)会有问题
{
    if (tos >= &stack[STACK_SIZE])
    {
        error(STACK_OVERFLOW);
        return;
    }

    *++tos = value;
}