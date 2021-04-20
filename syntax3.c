#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"
#include "parser.h"

/*--------------------------------------------------------------*/
/*  main		Initialize the scanner and call	the	*/
/*			statement routine.			*/
/*--------------------------------------------------------------*/

main(argc, argv)

    int  argc;
    char *argv[];

{
    /*
    --  Initialize the scanner.
    */
    init_scanner(argv[1]);

    /*
    --  Process a program.
    */
    get_token();
    program();
}
