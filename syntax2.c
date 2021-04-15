#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "parser.h"

extern TOKEN_CODE token;
extern int line_number, error_count;

extern TYPE_STRUCT dummy_type;
/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/
char buffer[MAX_PRINT_LINE_LENGTH];

int main(int argc, char const *argv[])
{
    SYMTAB_NODE_PTR program_idp;

    init_scanner(argv[1]);
    init_symtab();

    /*
    --  Create an artifical program id node.
    */
    program_idp = alloc_struct(SYMTAB_NODE);
    program_idp->defn.key = PROG_DEFN;
    program_idp->defn.info.routine.key = DECLARED;
    program_idp->defn.info.routine.parm_count = 0;
    program_idp->defn.info.routine.total_parm_size = 0;
    program_idp->defn.info.routine.total_local_size = 0;
    program_idp->typep = &dummy_type;
    program_idp->label_index = 0;

    /*
    --  Parse a block.
    */
    get_token();
    block(program_idp);

    while (token != END_OF_FILE)
    {
        error(UNEXPECTED_TOKEN);
        get_token();
    }
    quit_scanner();

    /*
    --  Print the parser's summary.
    */
    print_line("\n");
    print_line("\n");
    sprintf(buffer, "%20d Source lines.\n", line_number);
    print_line(buffer);
    sprintf(buffer, "%20d Source errors.\n", error_count);
    print_line(buffer);

    if (error_count == 0)
        exit(0);
    else
        exit(-SYNTAX_ERROR);
    return 0;
}

/*--------------------------------------------------------------*/
/*  block               Process a block, which consists of      */
/*                      declarations followed by a compound     */
/*                      statement.                              */
/*--------------------------------------------------------------*/

TOKEN_CODE follow_decls_list[] = {SEMICOLON, BEGIN, END_OF_FILE, 0};

block(SYMTAB_NODE_PTR rtn_idp)
{
    extern BOOLEAN block_flag;
    declarations(rtn_idp);

    synchronize(follow_decls_list, NULL, NULL);
    if (token != BEGIN)
        error(MISSING_BEGIN);
    
    compound_statement();
}