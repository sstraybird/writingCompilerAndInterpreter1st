#include <stdio.h>
#include "symtab.h"
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "symtab.h"
#include "parser.h"

extern TOKEN_CODE token;
extern char word_string[];
extern SYMTAB_NODE_PTR symtab_root;
extern LITERAL literal;
extern TYPE_STRUCT_PTR integer_typep, real_typep, boolean_typep, char_typep;

extern TYPE_STRUCT dummy_type;

extern TOKEN_CODE declaration_start_list[], statement_start_list[];

/*--------------------------------------------------------------*/
/*  Forwards							*/
/*--------------------------------------------------------------*/
TYPE_STRUCT_PTR do_type(), identifier_type(), enumeration_type(), subrange_type(), array_type(), record_type();

declarations(SYMTAB_NODE_PTR rtn_idp)
{
    if (token == CONST)
    {
        get_token();
        const_definitions();
    }

    if (token == TYPE)
    {
        get_token();
        type_definitions();
    }

    if (token == VAR)
    {
        get_token();
        var_declarations(rtn_idp);
    }
}

TOKEN_CODE follow_declaration_list[] = {SEMICOLON, IDENTIFIER,
                                        END_OF_FILE, 0};
const_definitions()
{
    SYMTAB_NODE_PTR const_idp; /* constant id */
    while (token == IDENTIFIER)
    {
        search_and_enter_local_symtab(const_idp);
        const_idp->defn.key = CONST_DEFN;

        get_token();
        if_token_get_else_error(EQUAL, MISSING_EQUAL);

        /*
        --  Process the constant.
        */
        do_const(const_idp);
        analyze_const_defn(const_idp);

        /*
        --  Error synchronization:  Should be ;
        */
        synchronize(follow_declaration_list, declaration_start_list, statement_start_list);
        if_token_get(SEMICOLON);
        else if (token_in(declaration_start_list) || token_in(statement_start_list))
            error(MISSING_SEMICOLON);
    }
}

/*--------------------------------------------------------------*/
/*  do_const            Process the constant of a constant      */
/*                      definition.                             */
/*--------------------------------------------------------------*/
do_const(SYMTAB_NODE_PTR const_idp)
{
    TOKEN_CODE sign = PLUS;   /* unary + or - sign */
    BOOLEAN saw_sign = FALSE; /* TRUE iff unary sign */

    /*
    --  Unary + or - sign.
    */
    if ((token == PLUS) || (token == MINUS))
    {
        sign = token;
        saw_sign = TRUE;
        get_token();
    }

    if (token == NUMBER)
    {
        if (literal.type == INTEGER_LIT)
        {
            const_idp->defn.info.constant.value.integer = sign == PLUS ? literal.value.integer : -literal.value.integer;
            const_idp->typep = integer_typep;
        }
        else
        {
            const_idp->defn.info.constant.value.real = sign == PLUS ? literal.value.real : -literal.value.real;
            const_idp->typep = real_typep;
        }
    }
    /*
    --  Identifier constant:  Integer, real, character, enumeration,
    --                        or string (character array) type.
    */
    else if (token == IDENTIFIER)
    {
        SYMTAB_NODE_PTR idp;
        search_all_symtab(idp);
        if (idp == NULL)
            error(UNDEFINED_IDENTIFIER);
        else if (idp->defn.key != CONST_DEFN)
            error(NOT_A_CONSTANT_IDENTIFIER);
        else if (idp->typep == integer_typep)
        {
            const_idp->defn.info.constant.value.integer =
                sign == PLUS ? idp->defn.info.constant.value.integer
                             : -idp->defn.info.constant.value.integer;
            const_idp->typep = integer_typep;
        }
        else if (idp->typep == real_typep)
        {
            const_idp->defn.info.constant.value.real =
                sign == PLUS ? idp->defn.info.constant.value.real
                             : -idp->defn.info.constant.value.real;
        }
        else if (idp->typep == char_typep)
        {
            if (saw_sign)
                error(INVALID_CONSTANT);
            const_idp->defn.info.constant.value.character =
                idp->defn.info.constant.value.character;
            const_idp->typep = char_typep;
        }
        else if (idp->typep->form == ENUM_FORM)
        {
            if (saw_sign)
                error(INVALID_CONSTANT);
            const_idp->defn.info.constant.value.integer = idp->defn.info.constant.value.integer;
            const_idp->typep = idp->typep;
        }
        else if (idp->typep->form == ARRAY_FORM)
        {
            if (saw_sign)
                error(INVALID_CONSTANT);
            const_idp->defn.info.constant.value.stringp = idp->defn.info.constant.value.stringp;
            const_idp->typep = idp->typep;
        }
    }
    /*
    --  String constant:  Character or string (character array) type.
    */
    else if (token == STRING)
    {
        if (saw_sign)
            error(INVALID_CONSTANT);
        if (strlen(literal.value.string) == 1)
        {
            const_idp->defn.info.constant.value.character = literal.value.string[0];
            const_idp->typep = char_typep;
        }
        else
        {
            int length = strlen(literal.value.string);
            const_idp->defn.info.constant.value.stringp = alloc_bytes(length + 1);
            strcpy(const_idp->defn.info.constant.value.stringp, literal.value.string);
            const_idp->typep = make_string_typep(length);
        }
    }
    else
    {
        const_idp->typep = &dummy_type;
        error(INVALID_CONSTANT);
    }
    get_token();
}

/*--------------------------------------------------------------*/
/*  make_string_typep   Make a type structure for a string of   */
/*                      the given length, and return a pointer  */
/*                      to it.                                  */
/*--------------------------------------------------------------*/
TYPE_STRUCT_PTR make_string_typep(int length)
{
    TYPE_STRUCT_PTR string_tp = alloc_struct(TYPE_STRUCT);
    TYPE_STRUCT_PTR index_tp = alloc_struct(TYPE_STRUCT);

    /*
    --  Array type.
    */
    string_tp->form = ARRAY_FORM;
    string_tp->size = length;
    string_tp->type_idp = NULL;
    string_tp->info.array.index_typep = index_tp;
    string_tp->info.array.elmt_typep = char_typep;
    string_tp->info.array.elmt_count = length;

    /*
    --  Subrange index type.
    */
    index_tp->form = SUBRANGE_FORM;
    index_tp->size = sizeof(int);
    index_tp->type_idp = NULL;
    index_tp->info.subrange.range_typep = integer_typep;
    index_tp->info.subrange.min = 1;
    index_tp->info.subrange.max = length;

    return (string_tp);
}

type_definitions()
{
    SYMTAB_NODE_PTR type_idp;
    while (token == IDENTIFIER)
    {
        search_and_enter_local_symtab(type_idp);
        type_idp->defn.key = TYPE_DEFN;

        get_token();
        if_token_get_else_error(EQUAL, MISSING_EQUAL);

        type_idp->typep = do_type();
        if (type_idp->typep->type_idp == NULL)
            type_idp->typep->type_idp = type_idp;

        analyze_type_defn(type_idp);

        synchronize(follow_declaration_list, declaration_start_list, statement_start_list);
        if_token_get(SEMICOLON);
        else if (token_in(declaration_start_list) || token_in(statement_start_list))
            error(MISSING_SEMICOLON);
    }
}

TYPE_STRUCT_PTR do_type()
{
    switch (token)
    {
    case IDENTIFIER:
    {
        SYMTAB_NODE_PTR idp;
        search_all_symtab(idp);
        if (idp == NULL)
        {
            error(UNDEFINED_IDENTIFIER);
            return (&dummy_type);
        }
        else if (idp->defn.key == TYPE_DEFN)
            return identifier_type(idp);
        else if (idp->defn.key == CONST_DEFN)
            return subrange_type(idp);
        else
        {
            error(NOT_A_TYPE_IDENTIFIER);
            return (&dummy_type);
        }
    }
    case LPAREN:
        return (enumeration_type());
    case ARRAY:
        return (array_type());
    case RECORD:
        return (record_type());
    case PLUS:
    case MINUS:
    case NUMBER:
    case STRING:
        return (subrange_type(NULL));
    default:
        error(INVALID_TYPE);
        return (&dummy_type);
    }
}

TYPE_STRUCT_PTR identifier_type(SYMTAB_NODE_PTR idp)
{
    TYPE_STRUCT_PTR tp = NULL;
    tp = idp->typep;
    get_token();

    return (tp);
}

TOKEN_CODE follow_min_limit_list[] = {DOTDOT, IDENTIFIER, PLUS, MINUS, NUMBER, STRING, SEMICOLON, END_OF_FILE, 0};

TYPE_STRUCT_PTR subrange_type(SYMTAB_NODE_PTR min_idp)
{
    TYPE_STRUCT_PTR max_typep;
    TYPE_STRUCT_PTR tp = alloc_struct(TYPE_STRUCT);

    tp->form = SUBRANGE_FORM;
    tp->type_idp = NULL;

    /*
    --  Minimum constant.
    */
    get_subrange_limit(min_idp, &(tp->info.subrange.min), &(tp->info.subrange.range_typep));

    /*
    --  Error synchronization:  Should be ..
    */
    synchronize(follow_min_limit_list, NULL, NULL);
    if_token_get(DOTDOT);
    else if (token_in(follow_min_limit_list) || token_in(declaration_start_list) || token_in(statement_start_list))
    {
        error(MISSING_DOTDOT);
    }

    get_subrange_limit(NULL, &(tp->info.subrange.max), &max_typep);

    /*
    --  Check limits.
    */
    if (max_typep == tp->info.subrange.range_typep)
    {
        if (tp->info.subrange.min > tp->info.subrange.max)
        {
            error(MIN_GT_MAX);
        }
    }
    else
    {
        error(INCOMPATIBLE_TYPES);
    }
    tp->size = max_typep == char_typep ? sizeof(char) : sizeof(int);
    return (tp);
}

get_subrange_limit(SYMTAB_NODE_PTR minmax_idp, int *minmaxp, TYPE_STRUCT_PTR *typepp)
{
    SYMTAB_NODE_PTR idp = minmax_idp;
    TOKEN_CODE sign = PLUS;   /* unary + or - sign */
    BOOLEAN saw_sign = FALSE; /* TRUE iff unary sign */

    if ((token == PLUS) || (token == MINUS))
    {
        sign = token;
        saw_sign = TRUE;
        get_token();
    }

    if (token == NUMBER)
    {
        if (literal.type == INTEGER_LIT)
        {
            *typepp = integer_typep;
            *minmaxp = (sign == PLUS) ? literal.value.integer : -literal.value.integer;
        }
        else
        {
            error(INVALID_SUBRANGE_TYPE);
        }
    }
    else if (token == IDENTIFIER)
    {
        if (idp == NULL)
            search_all_symtab(idp);
        if (idp == NULL)
        {
            error(UNDEFINED_IDENTIFIER);
        }
        else if (idp->typep == real_typep)
        {
            error(INVALID_SUBRANGE_TYPE);
        }
        else if (idp->defn.key == CONST_DEFN)
        {
            *typepp = idp->typep;
            if (idp->typep == char_typep)
            {
                if (saw_sign)
                    error(INVALID_CONSTANT);
                *minmaxp = idp->defn.info.constant.value.character;
            }
            else if (idp->typep == integer_typep)
            {
                *minmaxp = idp->defn.info.constant.value.integer;
                if (sign == MINUS)
                    *minmaxp = -(*minmaxp);
            }
            else /* enumeration constant */
            {
                if (saw_sign)
                    error(INVALID_CONSTANT);
                *minmaxp = idp->defn.info.constant.value.integer;
            }
        }
        else
            error(NOT_A_CONSTANT_IDENTIFIER);
    }
    else if (token == STRING)
    {
        if (saw_sign)
            error(INVALID_CONSTANT);
        *typepp = char_typep;
        *minmaxp = literal.value.string[0];
        if (strlen(literal.value.string) != 1)
            error(INVALID_SUBRANGE_TYPE);
    }
    else
        error(MISSING_CONSTANT);
    get_token();
}

var_declarations(SYMTAB_NODE_PTR rtn_idp)
{
    var_or_field_declarations(rtn_idp, NULL, 0);
}

// P101 链表图第一个enumation (完成)
TYPE_STRUCT_PTR enumeration_type()
{
    SYMTAB_NODE_PTR const_idp; //constant id
    SYMTAB_NODE_PTR last_idp = NULL;
    TYPE_STRUCT_PTR tp = alloc_struct(TYPE_STRUCT);
    int const_value = -1;

    tp->form = ENUM_FORM;
    tp->size = sizeof(int);
    tp->type_idp = NULL;

    get_token();
    while (token == IDENTIFIER)
    {
        search_and_enter_local_symtab(const_idp);
        const_idp->defn.key = CONST_DEFN;
        const_idp->defn.info.constant.value.integer = ++const_value;
        const_idp->typep = tp;

        if (last_idp == NULL)
        {
            tp->info.enumeration.const_idp = last_idp = const_idp;
        }
        else
        {
            last_idp->next = const_idp;
            last_idp = const_idp;
        }
        get_token();
        if_token_get(COMMA);
    }
    if_token_get_else_error(RPAREN, MISSING_RPAREN);
    tp->info.enumeration.max = const_value;
    return (tp);
}

TOKEN_CODE index_type_start_list[] = {IDENTIFIER, NUMBER, STRING, LPAREN, MINUS, PLUS, 0};
TOKEN_CODE follow_dimension_list[] = {COMMA, RBRACKET, OF, SEMICOLON, END_OF_FILE, 0};
TOKEN_CODE follow_indexes_list[] = {OF, IDENTIFIER, LPAREN, ARRAY, RECORD, PLUS, MINUS, NUMBER, STRING, SEMICOLON, END_OF_FILE, 0};

TYPE_STRUCT_PTR array_type()
{
    TYPE_STRUCT_PTR tp = alloc_struct(TYPE_STRUCT);
    TYPE_STRUCT_PTR index_tp;     /* index type */
    TYPE_STRUCT_PTR elmt_tp = tp; /* element type */
    int array_size();

    get_token();
    if (token != LBRACKET)
        error(MISSING_LBRACKET);
    do
    {
        get_token();
        if (token_in(index_type_start_list))
        {
            elmt_tp->form = ARRAY_FORM;
            elmt_tp->size = 0;
            elmt_tp->type_idp = NULL;
            elmt_tp->info.array.index_typep = index_tp = do_type();

            switch (index_tp->form)
            {
            case ENUM_FORM:
                elmt_tp->info.array.elmt_count = index_tp->info.enumeration.max + 1;
                elmt_tp->info.array.min_index = 0;
                elmt_tp->info.array.max_index = index_tp->info.enumeration.max;
                break;
            case SUBRANGE_FORM:
                elmt_tp->info.array.elmt_count = index_tp->info.subrange.max - index_tp->info.subrange.min + 1;
                elmt_tp->info.array.min_index = index_tp->info.subrange.min;
                elmt_tp->info.array.max_index = index_tp->info.subrange.max;
                break;
            default:
                elmt_tp->form = NO_FORM;
                elmt_tp->size = 0;
                elmt_tp->type_idp = NULL;
                elmt_tp->info.array.index_typep = &dummy_type;
                error(INVALID_INDEX_TYPE);
                break;
            }
        }
        else
        {
            elmt_tp->form = NO_FORM;
            elmt_tp->size = 0;
            elmt_tp->type_idp = NULL;
            elmt_tp->info.array.index_typep = &dummy_type;
            error(INVALID_INDEX_TYPE);
        }

        synchronize(follow_dimension_list, NULL, NULL);

        if (token == COMMA)
            elmt_tp = elmt_tp->info.array.elmt_typep = alloc_struct(TYPE_STRUCT);
    } while (token == COMMA);

    if_token_get_else_error(RBRACKET, MISSING_RBRACKET);

    /*
    --  Error synchronization:  Should be OF
    */
    synchronize(follow_indexes_list, declaration_start_list, statement_start_list);
    if_token_get_else_error(OF, MISSING_OF);

    /*
    --  Element type.
    */
    elmt_tp->info.array.elmt_typep = do_type();

    tp->size = array_size(tp);
    return (tp);
}

int array_size(TYPE_STRUCT_PTR tp)
{
    if (tp->info.array.elmt_typep->size == 0)
        tp->info.array.elmt_typep->size = array_size(tp->info.array.elmt_typep);
    tp->size = tp->info.array.elmt_count * tp->info.array.elmt_typep->size;
    return (tp->size);
}
TYPE_STRUCT_PTR record_type()
{
    TYPE_STRUCT_PTR record_tp = alloc_struct(TYPE_STRUCT);

    record_tp->form = RECORD_FORM;
    record_tp->type_idp = NULL;
    record_tp->info.record.field_symtab = NULL;

    get_token();
    var_or_field_declarations(NULL, record_tp, 0);

    if_token_get_else_error(END, MISSING_END);
    return (record_tp);
}

TOKEN_CODE follow_variables_list[] = {SEMICOLON, IDENTIFIER, END_OF_FILE, 0};
TOKEN_CODE follow_fields_list[] = {SEMICOLON, END, IDENTIFIER, END_OF_FILE, 0};

var_or_field_declarations(SYMTAB_NODE_PTR rtn_idp, TYPE_STRUCT_PTR record_tp, int offset)
{
    SYMTAB_NODE_PTR idp, first_idp, last_idp; //variable or field ids

    SYMTAB_NODE_PTR prev_last_idp = NULL;                        /* last id of list */
    TYPE_STRUCT_PTR tp;                                          /* type */
    BOOLEAN var_flag = (rtn_idp != NULL); /* TRUE:  variables */ /* FALSE: fields */

    int size;
    int total_size = 0;

    while (token == IDENTIFIER)
    {
        first_idp = NULL;
        /*
	    --  Loop process each variable or field id in a sublist.
	    */
        while (token == IDENTIFIER)
        {
            if (var_flag)
            {
                search_and_enter_local_symtab(idp);
                idp->defn.key = VAR_DEFN;
            }
            else
            {
                search_and_enter_this_symtab(idp, record_tp->info.record.field_symtab);
                idp->defn.key = FIELD_DEFN;
            }
            idp->label_index = 0;

            if (first_idp == NULL)
            {
                first_idp = last_idp = idp;
                if (var_flag && (rtn_idp->defn.info.routine.locals == NULL))
                {
                    rtn_idp->defn.info.routine.locals = idp;
                }
            }
            else
            {
                last_idp->next = idp;
                last_idp = idp;
            }
            get_token();
            if_token_get(COMMA);
        }
        if_token_get_else_error(COLON, MISSING_COLON);

        tp = do_type();
        size = tp->size;

        for (idp = first_idp; idp != NULL; idp = idp->next)
        {
            idp->typep = tp;
            if (var_flag)
            {
                total_size += size;
                idp->defn.info.data.offset = offset++;
                analyze_var_decl(idp);
            }
            else /* record fields */
            {
                idp->defn.info.data.offset = offset;
                offset += size;
            }
        }
        if (prev_last_idp != NULL)
            prev_last_idp->next = first_idp;
        prev_last_idp = last_idp;

        synchronize(var_flag ? follow_variables_list : follow_fields_list, declaration_start_list, statement_start_list);
        if_token_get(SEMICOLON);
        else if (var_flag && ((token_in(declaration_start_list) || token_in(statement_start_list))))
            error(MISSING_SEMICOLON);
    }
    if (var_flag)
        rtn_idp->defn.info.routine.total_local_size = total_size;
    else
        record_tp->size = offset;
}
