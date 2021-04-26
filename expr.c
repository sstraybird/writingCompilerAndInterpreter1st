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

// extern SYMTAB_NODE_PTR symtab_root;
extern SYMTAB_NODE_PTR symtab_display[];
extern int level;

extern TYPE_STRUCT dummy_type;

extern LITERAL literal;

extern TYPE_STRUCT_PTR integer_typep, real_typep, boolean_typep, char_typep;

TOKEN_CODE rel_op_list[] = {LT, LE, EQUAL, NE, GE, GT, 0};

extern char       token_string[];

/*--------------------------------------------------------------*/
/*  Forwards                                                    */
/*--------------------------------------------------------------*/

TYPE_STRUCT_PTR expression(), simple_expression(), term(), factor(),
    function_call();

/*--------------------------------------------------------------*/
/*  integer_operands    TRUE if both operands are integer,      */
/*                      else FALSE.                             */
/*--------------------------------------------------------------*/
#define integer_operands(tp1, tp2) ((tp1 == integer_typep) && (tp2 == integer_typep))

/*--------------------------------------------------------------*/
/*  real_operands       TRUE if at least one or both operands   */
/*                      operands are real (and the other        */
/*                      integer), else FALSE.                   */
/*--------------------------------------------------------------*/
#define real_operands(tp1, tp2) (((tp1 == real_typep) && ((tp2 == real_typep) || (tp2 == integer_typep))) || \
                                 ((tp2 == real_typep) && ((tp1 == real_typep) || (tp1 == integer_typep))))

/*--------------------------------------------------------------*/
/*  boolean_operands    TRUE if both operands are boolean       */
/*                      else FALSE.                             */
/*--------------------------------------------------------------*/
#define boolean_operands(tp1, tp2) ((tp1 == boolean_typep) && (tp2 == boolean_typep))
TYPE_STRUCT_PTR expression()
{
    TOKEN_CODE op;
    TYPE_STRUCT_PTR result_tp, tp2;

    result_tp = simple_expression();

    if (token_in(rel_op_list))
    {
        op = token;
        result_tp = base_type(result_tp);
        get_token();
        tp2 = base_type(simple_expression());

        check_rel_op_types(result_tp, tp2);
        result_tp = boolean_typep;
    }
    return result_tp;
}

TOKEN_CODE add_op_list[] = {PLUS, MINUS, OR, 0};
TYPE_STRUCT_PTR simple_expression()
{
    TOKEN_CODE op;
    TOKEN_CODE unary_op = PLUS;

    BOOLEAN saw_unary_op = FALSE; /* TRUE iff unary operator */
    TYPE_STRUCT_PTR result_tp, tp2;

    if ((token == PLUS) || (token == MINUS))
    {
        unary_op = token;
        saw_unary_op = TRUE;
        get_token();
    }
    result_tp = term(); /* first term */

    if (saw_unary_op && (base_type(result_tp) != integer_typep) && (result_tp != real_typep))
        error(INCOMPATIBLE_TYPES);

    while (token_in(add_op_list))
    {
        op = token;
        result_tp = base_type(result_tp);

        get_token();
        tp2 = base_type(term());

        switch (op)
        {
        case PLUS:
        case MINUS:
        {
            if (integer_operands(result_tp, tp2))
                result_tp = integer_typep;
            else if (real_operands(result_tp, tp2))
                result_tp = real_typep;
            else
            {
                error(INCOMPATIBLE_TYPES);
                result_tp = &dummy_type;
            }
            break;
        }
        case OR:
        {
            if (!boolean_operands(result_tp, tp2))
                error(INCOMPATIBLE_TYPES);
            result_tp = boolean_typep;
            break;
        }
        }
    }
    return result_tp;
}

TOKEN_CODE mult_op_list[] = {STAR, SLASH, DIV, MOD, AND, 0};
TYPE_STRUCT_PTR term()
{
    TOKEN_CODE op;
    TYPE_STRUCT_PTR result_tp, tp2;

    result_tp = factor();

    while (token_in(mult_op_list))
    {
        op = token;
        result_tp = base_type(result_tp);

        get_token();
        tp2 = base_type(factor());
        switch (op)
        {
        case STAR:
        {
            if (integer_operands(result_tp, tp2))
                result_tp = integer_typep;
            else if (real_operands(result_tp, tp2))
                result_tp = real_typep;
            else
            {
                error(INCOMPATIBLE_TYPES);
                result_tp = &dummy_type;
            }
            break;
        }
        case SLASH:
        {
            if ((!real_operands(result_tp, tp2)) && (!integer_operands(result_tp, tp2)))
            {
                error(INCOMPATIBLE_TYPES);
            }
            result_tp = real_typep;
            break;
        }
        case DIV:
        case MOD:
        {
            if (!integer_operands(result_tp, tp2))
                error(INCOMPATIBLE_TYPES);
            result_tp = integer_typep;
            break;
        }
        case AND:
        {
            if (!boolean_operands(result_tp, tp2))
                error(INCOMPATIBLE_TYPES);
            result_tp = boolean_typep;
            break;
        }
        }
    }
    return result_tp;
}

TYPE_STRUCT_PTR factor()
{
    TYPE_STRUCT_PTR tp;
    switch (token)
    {
    case IDENTIFIER:
        SYMTAB_NODE_PTR idp;
        search_and_find_all_symtab(idp);
        // if (idp->defn.key == CONST_DEFN)
        // {
        //     get_token();
        //     tp = idp->typep;
        // }
        // else
        // {
        //     tp = variable(idp, EXPR_USE);
        // }
        switch (idp->defn.key)
        {
        case FUNC_DEFN:
            crunch_symtab_node_ptr(idp);
            get_token();
            tp = routine_call(idp, TRUE);
            break;
        case PROC_DEFN:
            error(INVALID_IDENTIFIER_USAGE);
            get_token();
            actual_parm_list(idp, FALSE);
            tp = &dummy_type;
            break;
        case CONST_DEFN:
            crunch_symtab_node_ptr(idp);
            get_token();
            tp = idp->typep;
            break;
        default:
            tp = variable(idp, EXPR_USE);
            break;
        }
        break;
    case NUMBER:{
        SYMTAB_NODE_PTR np;
        np = search_symtab(token_string,symtab_display[1]);
        if(np == NULL)
            np = enter_symtab(token_string,symtab_display[1]);
        // tp = literal.type == INTEGER_LIT ? integer_typep : real_typep;
        if(literal.type == INTEGER_LIT){
            tp = np->typep = integer_typep;
            np->defn.info.constant.value.integer = literal.value.integer ;
        }else {
            tp = np->typep = real_typep;
            np->defn.info.constant.value.real = literal.value.real;
        }

        crunch_symtab_node_ptr(np) ;
        get_token();
        break;
    }

    case STRING:{
        SYMTAB_NODE_PTR np;
        int length = strlen(literal.value.string);
        np = search_symtab(token_string,symtab_display[1]);
        if(np == NULL) 
            np = enter_symtab(token_string,symtab_display[1]);
        if(length == 1){
            np->defn.info.constant.value.character = literal.value.string[0];
            tp = char_typep ;
        }else {
            np->typep = tp = make_string_typep(length);
            np->info = alloc_bytes(length+1);
            strcpy(np->info,literal.value.string);
        }
        // tp = length == 1 ? char_typep : make_string_typep(length);

        crunch_symtab_node_ptr(np);
        get_token();
        break;
    }

    case NOT:
        get_token();
        tp = factor();
        break;
    case LPAREN:
        get_token();
        tp = expression();
        if_token_get_else_error(RPAREN, MISSING_RPAREN);
        break;
    default:
        error(INVALID_EXPRESSION);
        tp = &dummy_type;
        break;
    }

    return tp;
}

TYPE_STRUCT_PTR variable(SYMTAB_NODE_PTR var_idp, USE use)
{
    TYPE_STRUCT_PTR tp = var_idp->typep;
    DEFN_KEY defn_key = var_idp->defn.key;
    TYPE_STRUCT_PTR array_subscript_list();
    TYPE_STRUCT_PTR record_field();

    crunch_symtab_node_ptr(var_idp);

    switch (defn_key)
    {
    case VAR_DEFN:
    case VALPARM_DEFN:
    case VARPARM_DEFN:
    case FUNC_DEFN:
    case UNDEFINED:
        break; /* OK */

    default: /* error */
    {
        tp = &dummy_type;
        error(INVALID_IDENTIFIER_USAGE);
    }
    }

    get_token();

    /*
    --  There must not be a parameter list, but if there is one,
    --  parse it anyway for error recovery.
    */
    if(token == LPAREN){
        error(UNEXPECTED_TOKEN);
        actual_parm_list(var_idp,FALSE);
        return tp;
    }
    while ((token == LBRACKET) || (token == PERIOD))
    {
        tp = token == LBRACKET ? array_subscript_list(tp) : record_field(tp);
    }

    return tp;
}

TYPE_STRUCT_PTR array_subscript_list(TYPE_STRUCT_PTR tp)
{
    TYPE_STRUCT_PTR index_tp, elmt_tp, ss_tp;
    extern TOKEN_CODE statement_end_list[];

    do
    {
        if (tp->form == ARRAY_FORM)
        {
            index_tp = tp->info.array.index_typep;
            elmt_tp = tp->info.array.elmt_typep;

            get_token();
            ss_tp = expression();
            /*
            --  The subscript expression must be assignment type
            --  compatible with the corresponding subscript type.
            */
            if (!is_assign_type_compatible(index_tp, ss_tp))
                error(INCOMPATIBLE_TYPES);
            tp = elmt_tp;
        }
        else
        {
            error(TOO_MANY_SUBSCRIPTS);
            while ((token != RBRACKET) && (!token_in(statement_end_list)))
                get_token();
        }
    } while (token == COMMA);

    if_token_get_else_error(RBRACKET, MISSING_RPAREN);
    return (tp);
}

TYPE_STRUCT_PTR record_field(TYPE_STRUCT_PTR tp)
{
    SYMTAB_NODE_PTR field_idp;
    get_token();
    if ((token == IDENTIFIER) && (tp->form == RECORD_FORM))
    {
        search_this_symtab(field_idp, tp->info.record.field_symtab);
        get_token();
        if (field_idp != NULL)
            return field_idp->typep;
        else
        {
            error(INVALID_FIELD);
            return (&dummy_type);
        }
    }
    else
    {
        get_token();
        error(INVALID_FIELD);
        return (&dummy_type);
    }
}

BOOLEAN is_assign_type_compatible(TYPE_STRUCT_PTR tp1, TYPE_STRUCT_PTR tp2)
{
    tp1 = base_type(tp1);
    tp2 = base_type(tp2);
    if (tp1 == tp2)
        return TRUE;
    /*
    --  real := integer
    */
    if ((tp1 == real_typep) && (tp2 == integer_typep))
        return TRUE;

    /*
    --  string1 := string2 of the same length
    */
    if ((tp1->form = ARRAY_FORM) && (tp2->form == ARRAY_FORM) && (tp1->info.array.elmt_typep == char_typep) && (tp2->info.array.elmt_typep == char_typep) && (tp1->info.array.elmt_count == tp2->info.array.elmt_count))
        return TRUE;
    return FALSE;
}

TYPE_STRUCT_PTR base_type(TYPE_STRUCT_PTR tp)
{
    return ((tp->form == SUBRANGE_FORM) ? tp->info.subrange.range_typep : tp);
}

check_rel_op_types(TYPE_STRUCT_PTR tp1, TYPE_STRUCT_PTR tp2)
{
    /*
    --  Two identical scalar or enumeration types.
    */
    if ((tp1 == tp2) && ((tp1->form == SCALAR_FORM) || (tp1->form == ENUM_FORM)))
        return;
    /*
    --  One integer and one real.
    */
    if (((tp1 == integer_typep) && (tp2 == real_typep)) || ((tp2 == integer_typep) && (tp1 == real_typep)))
        return;
    /*
    --  Two strings of the same length.
    */
    if ((tp1->form = ARRAY_FORM) && (tp2->form == ARRAY_FORM) && (tp1->info.array.elmt_typep == char_typep) && (tp2->info.array.elmt_typep == char_typep) && (tp1->info.array.elmt_count == tp2->info.array.elmt_count))
        return;
    error(INCOMPATIBLE_TYPES);
}