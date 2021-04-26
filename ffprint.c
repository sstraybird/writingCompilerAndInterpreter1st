#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "parser.h"
#include "pprint.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/
extern BOOLEAN print_flag;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/

char *code_buffer;                      /* code buffer */
char *code_bufferp;                     /* code buffer ptr */


int main(int argc, char const *argv[])
{

    print_flag = FALSE;
    init_scanner(argv[1]);


    get_token();
    program();
    quit_scanner();
    return 0;
}
