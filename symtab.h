#ifndef     symtab_h
#define     symtab_h

typedef union
{
    int integer;
    float real;
    char character;
    char *strings;
} VALUE;

typedef enum
{
    UNDEFINED,
    CONST_DEFN,
    TYPE_DEFN,
    VAR_DEFN,
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
            VALUE value;
        } constant;
        struct
        {
            ROUTINE_KEY key;
            int parm_count;
            int total_parm_size;
            int total_local_size;
        } routine;
        struct
        {
            int offset;
            struct symtab_node *record_idp;
        } data;
    } info;
} DEFN_STRUCT;

typedef struct symtab_node
{
    struct symtab_node *left, *right;
    struct symtab_node *next;
    char *name;
    char *info;
    DEFN_STRUCT defn;

    int level;
    int label_index;
} SYMTAB_NODE, *SYMTAB_NODE_PTR;


//  Functions
SYMTAB_NODE_PTR search_symtab();
SYMTAB_NODE_PTR enter_symtab();
#endif