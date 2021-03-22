#include <stdio.h>
#include "common.h"
#include "error.h"
#include "symtab.h"

//全局变量
SYMTAB_NODE_PTR symtab_root = NULL;

SYMTAB_NODE_PTR enter_symtab(char *name, SYMTAB_NODE_PTR *npp)
{
    int             cmp;                   /* result of strcmp */
    SYMTAB_NODE_PTR new_nodep; /* ptr to new entry */
    SYMTAB_NODE_PTR np;        /* ptr to node to test */

    /*
    --  Create the new node for the name.
    */
    new_nodep = alloc_struct(SYMTAB_NODE);
    new_nodep->name = alloc_bytes(strlen(name) + 1);
    strcpy(new_nodep->name, name);
    new_nodep->left = new_nodep->right = new_nodep->next = NULL;
    new_nodep->info = NULL;
    new_nodep->defn.key = UNDEFINED;
    new_nodep->level = new_nodep->label_index = 0;

    /*
    --  Loop to search for the insertion point.
    */
    while ((np = *npp) != NULL)
    {
        cmp = strcmp(name, np->name);
        npp = cmp < 0 ? &(np->left) : &(np->right);
    }

    *npp = new_nodep;       /* replace */
    return (new_nodep);
}

SYMTAB_NODE_PTR search_symtab(char *name, SYMTAB_NODE_PTR np)
{
    int cmp;
    while (np != NULL)
    {
        cmp = strcmp(name, np->name);
        if (cmp == 0)
            return np;
        else
            np = cmp < 0 ? np->left : np->right;
    }
    return NULL;
}