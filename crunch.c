#include <stdio.h>
#include "common.h"
#include "scanner.h"
#include "symtab.h"

extern BOOLEAN print_flag;
extern SYMTAB_NODE_PTR symtab_root;
extern TOKEN_CODE token;
extern char word_string[];
extern char token_string[];

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/
short index = 0; /* symtab entry index */
FILE *crunch_file;

int main(int argc, char const *argv[])
{
    print_flag = FALSE;
    init_scanner(argv[1]);

    do_pass_1();
    close_source_file();

    /*
    --  Open the crunch file and output the crunched
    --  symbol table.
    */
    crunch_file = fopen(argv[2], "wb");
    if (crunch_file == NULL)
    {
        fprintf(stderr, "*** ERROR: Failed to open source crunch file.\n");
        exit(-2);
    }
    fwrite(&index, sizeof(short), 1, crunch_file);
    output_crunched_symtab(symtab_root);

    /*
    --  Pass 2.
    */
    open_source_file(argv[1]);
    do_pass_2();
    fclose(crunch_file);
    quit_scanner();
    return 0;
}

do_pass_1()
{
    SYMTAB_NODE_PTR np;
    do
    {
        get_token();
        if (token == END_OF_FILE)
            break;
        switch (token)
        {
        case IDENTIFIER:
            if ((np = search_symtab(word_string, symtab_root)) == NULL)
            {
                np = enter_symtab(word_string, &symtab_root);
                np->info = (char *)index++;
            }
            break;
        case NUMBER:
        case STRING:
            if ((np == search_symtab(token_string, symtab_root)) == NULL)
            {
                np = enter_symtab(token_string, &symtab_root);
                np->info = (char *)index++;
            }
            break;
        default:
            break;
        }
    } while (token != PERIOD);
}

do_pass_2()
{
    SYMTAB_NODE_PTR np;
    do
    {
        get_token();
        if (token == END_OF_FILE)
            break;
        output_crunched_token();
    } while (token != PERIOD);
}

output_crunched_token()
{
    SYMTAB_NODE_PTR np;
    char token_code = token;

    fwrite(&token_code, 1, 1, crunch_file);
    
    switch (token)
    {
    case IDENTIFIER:
        np = search_symtab(word_string, symtab_root);
        index = (short)np->info;
        fwrite(&index, sizeof(short), 1, crunch_file);
        break;
    case NUMBER:
    case STRING:
        np = search_symtab(token_string, symtab_root);
        index = (short)np->info;
        fwrite(&index, sizeof(short), 1, crunch_file);
        break;
    default:
        break;
    }
}
output_crunched_symtab(SYMTAB_NODE_PTR np)
{
    char length;
    if (np == NULL)
        return;

    /*
    --  First, crunch the left subtree.
    */
    output_crunched_symtab(np->left);
    /*
    --  Then, crunch the root of the subtree.
    */
    length = strlen(np->name) + 1;
    index = (short)np->info;
    printf("index %d %s\n ",index,np->name);
    fwrite(&index, sizeof(short), 1, crunch_file);
    fwrite(&length, 1, 1, crunch_file);
    fwrite(np->name, length, 1, crunch_file);
    /*
    --  Finally, crunch the right subtree.
    */
    output_crunched_symtab(np->right);
}
