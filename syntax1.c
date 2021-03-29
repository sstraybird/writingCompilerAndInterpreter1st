#include <stdio.h>
#include "common.h"
#include "error.h"
#include "scanner.h"

/*--------------------------------------------------------------*/
/*  Externals                                                   */
/*--------------------------------------------------------------*/
extern  TOKEN_CODE  token;
extern  int         line_number;
extern  int         error_count;

/*--------------------------------------------------------------*/
/*  Globals                                                     */
/*--------------------------------------------------------------*/
char    buffer[MAX_PRINT_LINE_LENGTH] ;

int main(int argc, char const *argv[])
{
    init_scanner(argv[1]);

    get_token();
    statement();

    while(token != END_OF_FILE){
        error(UNEXPECTED_TOKEN);
        get_token();
    }

    quit_scanner();


    /*
    --  Print the parser's summary.
    */
    print_line("\n");
    print_line("\n");
    sprintf(buffer,"%20d Source lines.\n",line_number);
    print_line(buffer);
    sprintf(buffer,"%20d Source errors.\n",error_count);
    print_line(buffer);

    if(error_count == 0)
        exit(0);
    else
        exit(-SYNTAX_ERROR);
}
