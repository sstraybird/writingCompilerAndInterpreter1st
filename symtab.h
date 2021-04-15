#ifndef symtab_h
#define symtab_h

#include "common.h"
typedef union
{
    int integer;    //整数
    float real;     //浮点数
    char character; //字符
    char *stringp;  //字符串
} VALUE;

typedef enum
{
    UNDEFINED,
    CONST_DEFN,         //常量定义  
    TYPE_DEFN,          //类型定义
    VAR_DEFN,           //变量定义
    FIELD_DEFN,
    VALPARM_DEFN,
    VARPARM_DEFN,
    PROG_DEFN,
    PROC_DEFN,
    FUNC_DEFN,
} DEFN_KEY;

typedef enum
{
    DECLARED,
    FORWARD,
    READ,
    READLN,
    WRITE,
    WRITELN,
    ABS,
    ARCTAN,
    CHR,
    COS,
    EOFF,
    EOLN,
    EXP,
    LN,
    ODD,
    ORD,
    PRED,
    ROUND,
    SIN,
    SQR,
    SQRT,
    SUCC,
    TRUNC,
} ROUTINE_KEY;

typedef struct
{
    DEFN_KEY key;
    union
    {
        struct
        {
            VALUE value; //存储常量值
        } constant;
        struct
        {
            ROUTINE_KEY key;
            int parm_count;
            int total_parm_size;
            int total_local_size;

            struct symtab_node *parms;
            struct symtab_node *locals;
            struct symtab_node *local_symtab;
            char               *code_segment;
        } routine;
        struct
        {
            int offset;
            struct symtab_node *record_idp;
        } data;
    } info;
} DEFN_STRUCT;

typedef enum
{
    NO_FORM,
    SCALAR_FORM,
    ENUM_FORM,
    SUBRANGE_FORM,
    ARRAY_FORM,
    RECORD_FORM,
} TYPE_FORM;                    //类型格式
typedef struct type_struct
{
    TYPE_FORM form;
    int size;
    struct symtab_node *type_idp;
    union
    {
        struct
        {
            struct symtab_node *const_idp;
            int max;
        } enumeration;
        struct 
        {
            struct type_struct *range_typep;
            int                 min,max;
        }subrange;
        struct{
            struct type_struct *index_typep,*elmt_typep;
            int     min_index,max_index;
            int     elmt_count;
        }array;
        struct {
            struct symtab_node *field_symtab;
        }record;
    } info;
} TYPE_STRUCT, *TYPE_STRUCT_PTR; //类型
typedef struct symtab_node
{
    struct symtab_node *left, *right;
    struct symtab_node *next;               /* for chaining nodes */
    char *name;
    char *info;
    DEFN_STRUCT defn;

    int level;
    int label_index;

    TYPE_STRUCT_PTR typep; /* ptr to type struct */
} SYMTAB_NODE, *SYMTAB_NODE_PTR;

//  Functions
SYMTAB_NODE_PTR search_symtab();
SYMTAB_NODE_PTR enter_symtab();

/*--------------------------------------------------------------*/
/*  enter_name_local_symtab         Enter the given name into   */
/*                                  the local symbol table, and */
/*                                  set a pointer to the entry. */
/*--------------------------------------------------------------*/
#define enter_name_local_symtab(idp, name) idp = enter_symtab(name, &symtab_root)

#define search_and_enter_local_symtab(idp)                       \
    if ((idp = search_symtab(word_string, symtab_root)) == NULL) \
    {                                                            \
        idp = enter_symtab(word_string, &symtab_root);           \
    }                                                            \
    else                                                         \
        error(REDEFINED_IDENTIFIER)

#define search_all_symtab(idp) \
    idp = search_symtab(word_string, symtab_root);

#define search_and_enter_this_symtab(idp,this_symtab)           \
    if((idp = search_symtab(word_string,this_symtab)) == NULL){ \
        idp = enter_symtab(word_string,&this_symtab);           \
    }else{                                                      \
        error(REDEFINED_IDENTIFIER);                            \
    }


TYPE_STRUCT_PTR make_string_typep();

#define search_and_find_all_symtab(idp)                         \
    if((idp = search_symtab(word_string,symtab_root)) == NULL){ \
        error(REDEFINED_IDENTIFIER);                            \
        idp = enter_symtab(word_string,&symtab_root);           \
        idp->defn.key = UNDEFINED;                              \
        idp->typep = &dummy_type;                               \
    }  


#define search_this_symtab(idp,this_symtab)                     \
    idp = search_symtab(word_string,this_symtab)                                                                    
#endif