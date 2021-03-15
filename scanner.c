
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include "scanner.h"
/*------------------------------------------------*/
/*宏定义
/*------------------------------------------------*/
#define EOF_CHAR                '\x7f'
#define MAX_FILE_NAME_LENGTH    32
#define DATE_STRING_LENGTH      26




#define MAX_DIGIT_COUNT         20

/*-------------------------------------------------*/
/*字符编码类
/*-------------------------------------------------*/
typedef enum{
    LETTER,DIGIT,SPECIAL,EOF_CODE,
} CHAR_CODE;

/*-------------------------------------------------*/
/*全局变量                                          */
/*-------------------------------------------------*/
char    ch;
char    source_buffer[MAX_SOURCE_LINE_LENGTH];
char    source_name[MAX_FILE_NAME_LENGTH] ;
char    date[DATE_STRING_LENGTH] ;     
FILE    *source_file;
int     line_number = 0;
int     page_number = 0;
int     buffer_offset;
int     level       = 0;
int     line_count = MAX_LINES_PER_PAGE ;

char token_string[MAX_TOKEN_STRING_LENGTH];
char *tokenp  = token_string; 


CHAR_CODE char_table[256] ;
char *bufferp = source_buffer;

TOKEN_CODE token;

LITERAL     literal;        //字面值



/*-------------------------------*/
/* 返回字符编码
/*-------------------------------*/
#define char_code(ch)   char_table[ch]


init_scanner(char *name){
    int ch;
    for(ch =0;ch<256;++ch) char_table[ch]=SPECIAL;
    for(ch='0';ch<='9';++ch) char_table[ch]=DIGIT;
    for (ch = 'A'; ch <= 'Z'; ++ch) char_table[ch] = LETTER;
    for (ch = 'a'; ch <= 'z'; ++ch) char_table[ch] = LETTER;
    char_table[EOF_CHAR] = EOF_CODE;

    init_page_header(name) ;
    open_source_file(name);
}

//从source buffer中获取下一个token
get_token(){
    skip_blanks();
    tokenp = token_string;
    switch (char_code(ch))
    {
        case LETTER:
            get_word();
            break;
        case DIGIT:
            get_number();
            break;
        case EOF_CODE:
            token = END_OF_FILE;
            break;
        default:
            get_special();
            break;
    }
}

//打开源文件并获取第一个字符
open_source_file(char *name)
{
     if ((name == NULL) ||((source_file = fopen(name, "r")) == NULL)) {
	    printf("*** Error:  Failed to open source file.\n");
	    exit(-1);
    }
    bufferp = "";
    get_char();
}


//关闭文件
close_source_file(){
    fclose(source_file);
}

/*-----------------------------------------*/
/*1.第一阶段的特殊字符只为PERID,其他的为ERROR
/*2.第二阶段提取全部特殊token
/*-----------------------------------------*/
get_special(){
    *tokenp++ = ch;
    
    token = (ch == '.') ? PERIOD :ERROR;
    get_char();
    *tokenp = '\0' ;
}

//提取一个word token
get_word()
{
    BOOLEAN is_reserved_word();
    while((char_code(ch) == LETTER) || (char_code(ch) == DIGIT)){
        *tokenp++ = ch;
        get_char();
    }
    *tokenp = '\0';
    token = WORD;
}

get_number(){
    int nvalue = 0;         // number值
    int digit_count = 0;    //number中digit的总共数量
    BOOLEAN count_error = FALSE;        //在number中太多digit
    do{
        *tokenp++ = ch;
        if(++digit_count <= MAX_DIGIT_COUNT)
            nvalue = 10*nvalue + (ch-'0');
        else
            count_error = TRUE;
        get_char();
    }while (char_code(ch) == DIGIT);

    if(count_error){
        token = ERROR;
        return ;
    }
    literal.type        = INTEGER_LIT;
    literal.value.integer   = nvalue;
    *tokenp = '\0' ;
    token = NUMBER;
}

skip_blanks()
{
    while (ch == ' ') 
        get_char(); //取下一个字符
}

get_char()
{

    BOOLEAN get_source_line();
    if(*bufferp == '\0'){
        if(!get_source_line()){
            ch = EOF_CHAR;
            return ;
        }
        bufferp = source_buffer;
        buffer_offset = 0;
    }
    ch = *bufferp++;
   
    if((ch=='\n') || (ch == '\t'))
        ch = ' ' ;
}

BOOLEAN get_source_line()
{
    char print_buffer[MAX_SOURCE_LINE_LENGTH + 9] ;

    if((fgets(source_buffer,MAX_SOURCE_LINE_LENGTH,source_file))!=NULL){
        line_number++;
        sprintf(print_buffer,"%4d %d: %s",line_number,level,source_buffer);
        print_line(print_buffer) ;
        return TRUE;
    }else return FALSE ;
}

print_line(char line[])
{
    char save_ch;
    char *save_chp = NULL;
    if(++line_count>MAX_LINES_PER_PAGE){
        print_page_header();
        line_count = 1;
    }

    if(strlen(line) > MAX_PRINT_LINE_LENGTH)
        save_chp = &line[MAX_PRINT_LINE_LENGTH] ;
    if(save_chp){
        save_ch = *save_chp;
        *save_chp = '\0';
    }
    printf(line) ;

    if(save_chp)
        *save_chp = save_ch;
}


print_page_header()
{
    putchar(FORM_FEED_CHAR) ;
    printf("Page %d     %s      %s\n\n",++page_number,source_name,date) ;
}


init_page_header(name)
{
    time_t timer;

    strncpy(source_name,name,MAX_FILE_NAME_LENGTH -1) ;
    //设置日期和时间
    time(&timer);
    strcpy(date,asctime(localtime(&timer))) ;
}