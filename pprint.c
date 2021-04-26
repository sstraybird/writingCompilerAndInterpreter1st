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

char pprint_buffer[MAX_PRINT_LINE_LENGTH];   /* print buffer */
int  left_margin = 0;                       /* margin in buffer */

char *code_segmentp;                    /* code segment ptr */
char *code_segment_limit;               /* end of code segment */

TOKEN_CODE ctoken;                      /* token from code segment */

int main(int argc, char const *argv[])
{

    print_flag = FALSE;
    init_scanner(argv[1]);


    get_token();
    program();
    quit_scanner();
    return 0;
}

emit(char *string)
{
    int buffer_length = strlen(pprint_buffer);
    int string_length = strlen(string);

    if(buffer_length + string_length >= MAX_PRINT_LINE_LENGTH - 1){
        flush();
        indent();
    }
    strcat(pprint_buffer,string);
}

flush()
{
    if(pprint_buffer[0] != '\0'){
        printf("%s\n",pprint_buffer);
        pprint_buffer[0] = '\0';
    }
}

indent()
{
    if(left_margin > 0)
        sprintf(pprint_buffer,"%*s",left_margin," ");
    else 
        pprint_buffer[0] = '\0';
}