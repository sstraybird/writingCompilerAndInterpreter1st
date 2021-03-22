#include <stdio.h>
#include "symtab.h"
#include "scanner.h"
#include "common.h"

#define MAX_LINENUMS_PER_LINE 10

typedef struct linenum_item
{
    struct linenum_item *next;
    int line_number;
} LINENUM_ITEM, *LINENUM_ITEM_PTR;

typedef struct
{
    LINENUM_ITEM_PTR first_linenum, last_linenum;
} LINENUM_HEADER, *LINENUM_HEADER_PTR;

/*-----------------------------------------------*/
/*  Externals                                   */
/*----------------------------------------------*/
extern int line_number;
extern TOKEN_CODE token;
extern char word_string[];
extern SYMTAB_NODE_PTR symtab_root;

int main(int argc, char const *argv[])
{
    SYMTAB_NODE_PTR np;
    LINENUM_HEADER_PTR hp;
    int i = 1;

    init_scanner(argv[1]);
    do
    {
        get_token();
        if (token == END_OF_FILE)
        {
            break;
        }

        if (token == IDENTIFIER)
        {
            np = search_symtab(word_string, symtab_root);
            if (np == NULL)
            {
                np = enter_symtab(word_string, &symtab_root);
                hp = alloc_struct(LINENUM_HEADER);
                hp->first_linenum = hp->last_linenum = NULL;
                np->info = (char *)hp;
            }
            record_line_number(np, line_number);
        }
    } while (token != PERIOD);

    printf("\n\nCross-Reference");
    printf("\n------------------\n");
    print_xref(symtab_root);
    quit_scanner();
    return 0;
}

record_line_number(SYMTAB_NODE_PTR np, int number)
{
    LINENUM_ITEM_PTR ip;
    LINENUM_HEADER_PTR hp;

    ip = alloc_struct(LINENUM_ITEM);
    ip->line_number = number;
    ip->next = NULL;

    hp = (LINENUM_HEADER_PTR)np->info;
    if (hp->first_linenum == NULL)
    {
        hp->first_linenum = hp->last_linenum = ip;
    }
    else
    {
        (hp->last_linenum)->next = ip;
        hp->last_linenum = ip;
    }
}

print_xref(SYMTAB_NODE_PTR np)
{
    int n;
    LINENUM_ITEM_PTR ip;
    LINENUM_HEADER_PTR hp;

    if (np == NULL)
        return;

    /*
    --  First, print the left subtree.
    */
    print_xref(np->left);

    printf("\n%-16s   ", np->name);

    n = strlen(np->name) > 16 ? 0 : MAX_LINENUMS_PER_LINE;
    hp = (LINENUM_HEADER_PTR)np->info;
    for(ip = hp->first_linenum;ip != NULL;ip=ip->next){
        if(n == 0){
            printf("\n%-16s     "," ");
            n = MAX_LINENUMS_PER_LINE;
        }
        printf(" %4d",ip->line_number);
        --n;
    };
    printf("\n");

        /*
    --  Finally, print the right subtree.
    */
    print_xref(np->right);
}