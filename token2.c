#include <stdio.h>
#include "scanner.h"
#include "error.h"
/*-------------------------------------------------*/
/*Token命名字符串
/*-------------------------------------------------*/
char *symbol_strings[] = {
    "<no token>", "<IDENTIFIER>", "<NUMBER>", "<STRING>",
    "^", "*", "(", ")", "-", "+", "=", "[", "]", ":", ";",
    "<", ">", ",", ".", "/", ":=", "<=", ">=", "<>", "..",
    "<END OF FILE>", "<ERROR>",
    "AND", "ARRAY", "BEGIN", "CASE", "CONST", "DIV", "DO", "DOWNTO",
    "ELSE", "END", "FILE", "FOR", "FUNCTION", "GOTO", "IF", "IN",
    "LABEL", "MOD", "NIL", "NOT", "OF", "OR", "PACKED", "PROCEDURE",
    "PROGRAM", "RECORD", "REPEAT", "SET", "THEN", "TO", "TYPE",
    "UNTIL", "VAR", "WHILE", "WITH",
};
/*-------------------------------------------/
/*外部变量
/*------------------------------------------*/
extern TOKEN_CODE token;
extern char token_string[];
extern LITERAL literal;


//程序入口
int main(int argc, char const *argv[])
{

    init_scanner(argv[1]);
    // init_lister(argv[1]);
    // while(get_source_line());

    
    do{
        get_token();
        if(token == END_OF_FILE){
            error(UNEXPECTED_END_OF_FILE);
            break;
        }
        print_token();
    }while (token != PERIOD) ;

    quit_scanner();
    return 0;
}

print_token()
{
    char line[MAX_PRINT_LINE_LENGTH];
    char *symbol_string = symbol_strings[token];
    switch (token)
    {
    case NUMBER:
        if(literal.type == INTEGER_LIT)
            sprintf(line,"      >> %-16s %d (integral)\n",symbol_string,literal.value.integer);
        else
            sprintf(line,"      >> %-16s %g (real)\n",symbol_string,literal.value.real);
        break;
    case STRING:
        sprintf(line,"      >> %-16s '%-s'\n",symbol_string,literal.value.string);
        break;
    default:
        sprintf(line,"      >> %-16s %-s\n",symbol_string,token_string);
        break;
    }
    print_line(line);
}

















//初始化全局变量(不再使用)
// init_lister(char *name)
// {
//     time_t timer;

//     //拷贝文件名并且打开文件
//     strcpy(source_name,name);
//     source_file=fopen(source_name,"r");

//     //设置日期和时间
//     time(&timer);
//     strcpy(date,asctime(localtime(&timer))) ;
// }




