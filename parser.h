#ifndef parser_h
#define parser_h

#include    "common.h"
#include    "symtab.h"
/*--------------------------------------------------------------*/
/*  Uses of a variable                                          */
/*--------------------------------------------------------------*/
typedef enum{
    EXPR_USE,TARGET_USE,VARPARM_USE,
}USE;

/*--------------------------------------------------------------*/
/*  Functions                                                   */
/*--------------------------------------------------------------*/

TYPE_STRUCT_PTR expression();
TYPE_STRUCT_PTR variable();
TYPE_STRUCT_PTR routine_call();
TYPE_STRUCT_PTR base_type();
BOOLEAN         is_assign_type_compatible();


#define if_token_get_else_error(token_code,error_code)  \
    if(token == token_code)     \
        get_token();            \
    else                        \
        error(error_code)       \


#define if_token_get(token_code)    \
    if(token == token_code) get_token()


TYPE_STRUCT_PTR variable();
TYPE_STRUCT_PTR base_type();

#ifndef analyze
#define analyze_const_defn(idp)
#define analyze_type_defn(idp)
#define analyze_var_decl(idp)
#define analyze_routine_header(idp)
#define analyze_block(idp)
#endif

#endif