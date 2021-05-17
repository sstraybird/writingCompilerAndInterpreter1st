#include <stdio.h>
#include "common.h"
#include "error.h"
#include "symtab.h"
#include "scanner.h"
#include "parser.h"
#include "exec.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/

extern int level;
extern int exec_line_number;
extern long exec_stmt_count;

extern char *code_segmentp;
extern char *statement_startp;
extern TOKEN_CODE ctoken;

extern STACK_ITEM *stack;
extern STACK_ITEM_PTR tos;
extern STACK_ITEM_PTR stack_frame_basep;

extern TYPE_STRUCT_PTR integer_typep, real_typep,
    boolean_typep, char_typep;

/*--------------------------------------------------------------*/
/*  exec_assignment_statement       Execute an assignment       */
/*                                  statement.                  */
/*--------------------------------------------------------------*/
exec_assignment_statement(SYMTAB_NODE_PTR idp)
{
    STACK_ITEM_PTR targetp; /* ptr to assignment target */
    TYPE_STRUCT_PTR target_tp, base_target_tp, expr_tp;

    if (idp->defn.key == FUNC_DEFN)
    {
        STACK_FRAME_HEADER_PTR hp;
        int delta; /* difference in levels */
        hp = (STACK_FRAME_HEADER_PTR)stack_frame_basep;
        delta = level - idp->level - 1;
        while (delta-- > 0)
            hp = (STACK_FRAME_HEADER_PTR)hp->static_link.address;
        targetp = (STACK_ITEM_PTR)hp;
        target_tp = idp->typep;
        get_ctoken();
    }
    else
    {
        target_tp = exec_variable(idp, TARGET_USE);
        targetp = (STACK_ITEM_PTR)tos->address;
        pop();
    }

    base_target_tp = base_type(target_tp);

    /*
    --  Routine exec_expression leaves the expression value
    --  on top of stack.
    */
    get_ctoken();
    expr_tp = exec_expression();

    /*
    --  Do the assignment.
    */
    if ((target_tp == real_typep) &&
        (base_type(expr_tp) == integer_typep))
    {
        /*
        --  real := integer
        */
        targetp->real = tos->integer;
    }
    else if ((target_tp->form == ARRAY_FORM) ||
             (target_tp->form == RECORD_FORM))
    {
        /*
    --  array  := array
    --  record := record
    */
        char *ptr1 = (char *)targetp;
        char *ptr2 = tos->address;
        int size = target_tp->size;

        while (size--)
            *ptr1++ = *ptr2++;
    }
    else if ((base_target_tp == integer_typep) ||
             (target_tp->form == ENUM_FORM))
    {
        /*
        --  Range check assignment to integer
        --  or enumeration subrange.
        */
        if ((target_tp->form == SUBRANGE_FORM) && ((tos->integer < target_tp->info.subrange.min) ||
                                                   (tos->integer > target_tp->info.subrange.max)))
            runtime_error(VALUE_OUT_OF_RANGE);
        /*
        --  integer     := integer
        --  enumeration := enumeration
        */
        targetp->integer = tos->integer;
    }
    else if (base_target_tp == char_typep)
    {
        /*
        --  Range check assigment to character subrange.
        */
        if ((target_tp->form == SUBRANGE_FORM) && ((tos->byte < target_tp->info.subrange.min) ||
                                                   (tos->byte > target_tp->info.subrange.max)))
            runtime_error(VALUE_OUT_OF_RANGE);
        /*
        --  character := character
        */
        targetp->byte = tos->byte;
    }
    else
    {
        /*
        --  real := real
        */
        targetp->real = tos->real;
    }

    pop(); /* pop off expression value */

    trace_data_store(idp, idp->typep, targetp, target_tp);
}

exec_compound_statement()
{
    get_ctoken();
    while (ctoken != END)
    {
        exec_statement();
    }
    get_ctoken();
}

exec_actual_parms(SYMTAB_NODE_PTR rtn_idp)
{
    SYMTAB_NODE_PTR formal_idp; /* formal parm id */
    TYPE_STRUCT_PTR formal_tp, actual_tp;

    for (formal_idp = rtn_idp->defn.info.routine.parms; formal_idp != NULL; formal_idp = formal_idp->next)
    {
        formal_tp = formal_idp->typep;
        get_ctoken();

        if (formal_idp->defn.key == VALPARM_DEFN)
        {
            actual_tp = exec_expression();

            /*
            --  Range check for a subrange formal parameter.
            */
            if (formal_tp->form == SUBRANGE_FORM)
            {
                TYPE_STRUCT_PTR base_formal_tp = base_type(formal_tp);
                int value;
                value = ((base_formal_tp == integer_typep) || (base_formal_tp->form == ENUM_FORM)) ? tos->integer : tos->byte;
                if ((value < formal_tp->info.subrange.min) || (value > formal_tp->info.subrange.max))
                {
                    runtime_error(VALUE_OUT_OF_RANGE);
                }
            }
            else if ((formal_tp == real_typep) && (base_type(actual_tp) == integer_typep))
            {
                tos->real = tos->integer;
            }
            /*
            --  Formal parm is array or record:  Make a copy.
            */
            if ((formal_tp->form == ARRAY_FORM) || (formal_tp->form == RECORD_FORM))
            {
                int size = formal_tp->size;
                char *ptr1 = alloc_bytes(size);
                char *ptr2 = tos->address;
                char *save_ptr = ptr1;
                while (size--)
                {
                    *ptr1++ = *ptr2++;
                }
                tos->address = save_ptr;
            }
        }
        else
        {
            SYMTAB_NODE_PTR idp = get_symtab_cptr();
            exec_variable(idp, VARPARM_USE);
        }
    }
}

TYPE_STRUCT_PTR exec_declared_routine_call(SYMTAB_NODE_PTR rtn_idp)
{
    int old_level = level;              /* level of caller */
    int new_level = rtn_idp->level + 1; /* level of callee */
    STACK_ITEM_PTR new_stack_frame_basep;
    STACK_FRAME_HEADER_PTR hp; /* ptr to frame header */

    new_stack_frame_basep = tos + 1;
    push_stack_frame_header(old_level, new_level);

    /*
    --  Push parameter values onto the stack.
    */
    get_ctoken();
    if (ctoken == LPAREN)
    {
        exec_actual_parms(rtn_idp);
        get_ctoken();
    }

    /*
    --  Set the return address in the new stack frame,
    --  and execute the callee.
    */
    level = new_level;
    stack_frame_basep = new_stack_frame_basep;
    hp = (STACK_FRAME_HEADER_PTR)stack_frame_basep;
    hp->return_address.address = code_segmentp - 1;
    execute(rtn_idp);

    /*
    --  Return from callee.
    */
    level = old_level;
    get_ctoken(); /* first token after return */

    return rtn_idp->defn.key == PROC_DEFN ? NULL : rtn_idp->typep;
}

TYPE_STRUCT_PTR exec_routine_call(SYMTAB_NODE_PTR rtn_idp)
{
    TYPE_STRUCT_PTR exec_declared_routine_call();
    TYPE_STRUCT_PTR exec_standard_routine_call();

    if (rtn_idp->defn.info.routine.key == DECLARED)
        return (exec_declared_routine_call(rtn_idp));
    else
        return (exec_standard_routine_call(rtn_idp));
}

exec_case_statement()
{
    char* branch_table_location;   /* branch table addr */
    TYPE_STRUCT_PTR case_expr_tp;             /* CASE expr type */
    int             case_expr_value;          /* CASE expr value */
    int             case_label_count;         /* CASE label count */
    int             case_label_value;         /* CASE label value */
    BOOLEAN         done = FALSE;
    char* case_branch_location;    /* CASE branch addr */


    get_ctoken();
    branch_table_location = get_address_cmarker();

    get_ctoken();
    case_expr_tp = exec_expression();
    case_expr_value = (case_expr_tp == integer_typep) || (case_expr_tp->form == ENUM_FORM) ? tos->integer : tos->byte;
    pop();
    /*
    --  Search the branch table for the expression value.
    */
    code_segmentp = branch_table_location;
    get_ctoken();
    case_label_count = get_cinteger();
    while (!done && case_label_count--) {
        case_label_value = get_cinteger();
        case_branch_location = get_caddress();
        done = case_label_value == case_expr_value;
    }
    if (case_label_count >= 0) {
        code_segmentp = case_branch_location;
        get_ctoken();
        exec_statement();

        code_segmentp = get_address_cmarker();
        get_ctoken();
    }
    else {
        runtime_error(INVALID_CASE_VALUE);
    }
}

exec_repeat_statement()
{
        char *loop_start_location = code_segmentp;  /* addr of loop start */
        do{
            get_ctoken();       /* token after REPEAT */
            do{
                exec_statement();
            }while(ctoken !=UNTIL);

            /*
            --  Evaluate the boolean expression.
            */
           get_ctoken();
           exec_expression();
           if(tos->integer == 0)
                code_segmentp = loop_start_location;
            pop();
        }while(code_segmentp == loop_start_location);
}
exec_for_statement()
{
    SYMTAB_NODE_PTR control_idp;           /* control var id */
    TYPE_STRUCT_PTR control_tp;            /* control var type */
    STACK_ITEM_PTR  targetp;               /* ptr to control target */
    char* loop_start_location;  /* addr of start of loop */
    char* loop_end_location;    /* addr of end of loop */
    int             control_value;         /* value of control var */
    int             initial_value, final_value, delta_value;

    get_ctoken();
    loop_end_location = get_address_cmarker();

    /*
    --  Get the address of the control variable's stack item.
    */
    get_ctoken();
    control_idp = get_symtab_cptr();
    control_tp = base_type(exec_variable(control_idp, TARGET_USE));
    targetp = (STACK_ITEM_PTR)tos->address;
    pop();      /* control variable address */

    /*
    --  Evaluate the initial expression.
    */
    get_ctoken();
    exec_expression();
    initial_value = (control_tp == integer_typep)
        ? tos->integer
        : tos->byte;
    pop();      /* initial value */

    delta_value = (ctoken == TO) ? 1 : -1;

    /*
    --  Evaluate the final expression.
    */
    get_ctoken();
    exec_expression();
    final_value = (control_tp == integer_typep)
        ? tos->integer
        : tos->byte;
    pop();      /* final value */

    loop_start_location = code_segmentp;
    control_value = initial_value;

    /*
    --  Execute the FOR loop.
    */
    while (((delta_value == 1) && (control_value <= final_value))
        || ((delta_value == -1) && (control_value >= final_value))) {
        if (control_tp == integer_typep) {
            targetp->integer = control_value;
        }
        else
        {
            targetp->byte = control_value;
        }
        get_ctoken();       /* token after DO */
        exec_statement();

        control_value += delta_value;
        code_segmentp = loop_start_location;
    }

    code_segmentp = loop_end_location;
    get_ctoken();       /* token after FOR statement */
}
exec_if_statement()
{
    char* false_location;
    BOOLEAN test;

    get_ctoken();       /* token after IF */
    false_location = get_address_cmarker();

    /*
    --  Evaluate the boolean expression.
    */
    get_ctoken();
    exec_expression();
    test = tos->integer == 1;
    pop();

    if (test) {
        /*
        --  True:  Execute the true branch.
        */
        get_ctoken();       /* token after THEN */
        exec_statement();
        if (ctoken == ELSE) {
            get_ctoken();
            code_segmentp = get_address_cmarker();
            get_ctoken();
        }
    }
    else {
        /*
        --  False:  Execute the false branch if there is one.
        */
        code_segmentp = false_location;
        get_ctoken();
        if (ctoken == ELSE) {
            get_ctoken();
            get_address_cmarker();

            get_ctoken();
            exec_statement();
        }
    }
}
exec_while_statement()
{
    char* loop_end_location;         /* addr of end of loop */
    char* test_location;             /* addr of boolean expr */
    BOOLEAN loop_done = FALSE;

    get_ctoken();       /* token after WHILE */
    
    loop_end_location = get_address_cmarker();
    test_location = code_segmentp;

    do {
        get_ctoken();
        exec_expression();
        if (tos->integer == 0) {
            code_segmentp = loop_end_location;
            loop_done = TRUE;
        }
        pop();      /* boolean value */

        /*
        --  If true, execute the statement.
        */
        if (!loop_done) {
            get_ctoken();
            exec_statement();
            code_segmentp = test_location;
        }
    } while (!loop_done);
    get_ctoken();
}
exec_statement()
{
    if (ctoken == STATEMENT_MARKER)
    {
        exec_line_number = get_statement_cmarker();
        ++exec_stmt_count;

        statement_startp = code_segmentp;
        trace_statement_execution();
        get_ctoken();
    }
    switch (ctoken)
    {
    case IDENTIFIER:
    {
        SYMTAB_NODE_PTR idp = get_symtab_cptr();

        if (idp->defn.key == PROC_DEFN)
            exec_routine_call(idp);
        else
            exec_assignment_statement(idp);
        break;
    }
    case BEGIN:
        exec_compound_statement();
        break;
    case CASE:
        exec_case_statement();
        break;
    case FOR:
        exec_for_statement();
        break;
    case IF:
        exec_if_statement();
        break;
    case REPEAT:
        exec_repeat_statement();
        break;
    case WHILE:
        exec_while_statement();
        break;

    case SEMICOLON:
    case END:
    case ELSE:
    case UNTIL:
        break;
    default:
        runtime_error(UNIMPLEMENTED_RUNTIME_FEATURE);
    }

    while (ctoken == SEMICOLON)
        get_ctoken();
}