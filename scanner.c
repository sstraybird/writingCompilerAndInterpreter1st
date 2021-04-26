
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <math.h>
#include "scanner.h"
#include "error.h"
/*------------------------------------------------*/
/*宏定义
/*------------------------------------------------*/
#define EOF_CHAR '\x7f'
#define MAX_FILE_NAME_LENGTH 32
#define DATE_STRING_LENGTH 26

#define MAX_DIGIT_COUNT 20

#define MIN_RESERVED_WORD_LENGTH 2
#define MAX_RESERVED_WORD_LENGTH 9

#define MAX_EXPONENT 37
#define MAX_INTEGER 32767

#define TAB_SIZE 8
/*-------------------------------------------------*/
/*字符编码类
/*-------------------------------------------------*/
typedef enum
{
    LETTER,
    DIGIT,
    QUOTE,
    SPECIAL,
    EOF_CODE,
} CHAR_CODE;

/*-------------------------------------------------*/
/* 保留字表格
/*-------------------------------------------------*/
typedef struct
{
    char *string;
    TOKEN_CODE token_code;
} RW_STRUCT;

RW_STRUCT rw_2[] = {
    {"do", DO},
    {"if", IF},
    {"in", IN},
    {"of", OF},
    {"or", OR},
    {"to", TO},
    {NULL, 0},
};

RW_STRUCT rw_3[] = {
    {"and", AND},
    {"div", DIV},
    {"end", END},
    {"for", FOR},
    {"mod", MOD},
    {"nil", NIL},
    {"not", NOT},
    {"set", SET},
    {"var", VAR},
    {NULL, 0},
};

RW_STRUCT rw_4[] = {
    {"case", CASE},
    {"else", ELSE},
    {"file", FFILE},
    {"goto", GOTO},
    {"then", THEN},
    {"type", TYPE},
    {"with", WITH},
    {NULL, 0},
};

RW_STRUCT rw_5[] = {
    {"array", ARRAY},
    {"begin", BEGIN},
    {"const", CONST},
    {"label", LABEL},
    {"until", UNTIL},
    {"while", WHILE},
    {NULL, 0},
};

RW_STRUCT rw_6[] = {
    {"downto", DOWNTO},
    {"packed", PACKED},
    {"record", RECORD},
    {"repeat", REPEAT},
    {NULL, 0},
};

RW_STRUCT rw_7[] = {
    {"program", PROGRAM},
    {NULL, 0},
};

RW_STRUCT rw_8[] = {
    {"function", FUNCTION},
    {NULL, 0},
};

RW_STRUCT rw_9[] = {
    {"procedure", PROCEDURE},
    {NULL, 0},
};

RW_STRUCT *rw_table[] = {
    NULL,
    NULL,
    rw_2,
    rw_3,
    rw_4,
    rw_5,
    rw_6,
    rw_7,
    rw_8,
    rw_9,
};




/*-------------------------------------------------*/
/*全局变量                                          */
/*-------------------------------------------------*/
char ch;
char source_buffer[MAX_SOURCE_LINE_LENGTH];
char source_name[MAX_FILE_NAME_LENGTH];
char date[DATE_STRING_LENGTH];
FILE *source_file;
int line_number = 0;
int page_number = 0;
int buffer_offset;
int level = 0;
int line_count = MAX_LINES_PER_PAGE;

char token_string[MAX_TOKEN_STRING_LENGTH];
char *tokenp = token_string;

CHAR_CODE char_table[256];
char *bufferp = source_buffer;

TOKEN_CODE token;

LITERAL literal;                           //字面值
char word_string[MAX_TOKEN_STRING_LENGTH]; /* 转为小写 */

int digit_count;             //number中digit的总共数量
BOOLEAN count_error = FALSE; //在number中太多digit

BOOLEAN print_flag = TRUE; // TRUE to print source lines

BOOLEAN     block_flag = FALSE; /* TRUE only when parsing a block */
/*-------------------------------*/
/* 返回字符编码
/*-------------------------------*/
#define char_code(ch) char_table[ch]

init_scanner(char *name)
{
    int ch;
    for (ch = 0; ch < 256; ++ch)
        char_table[ch] = SPECIAL;
    for (ch = '0'; ch <= '9'; ++ch)
        char_table[ch] = DIGIT;
    for (ch = 'A'; ch <= 'Z'; ++ch)
        char_table[ch] = LETTER;
    for (ch = 'a'; ch <= 'z'; ++ch)
        char_table[ch] = LETTER;
    char_table['\''] = QUOTE; //新增单引号字符
    char_table[EOF_CHAR] = EOF_CODE;

    init_page_header(name);
    open_source_file(name);
}

//从source buffer中获取下一个token
get_token()
{
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
    case QUOTE:
        get_string();
        break;
    case EOF_CODE:
        token = END_OF_FILE;
        break;
    default:
        get_special();
        break;
    }

    if(block_flag)
        crunch_token();
}

//提取一个word token
get_word()
{
    BOOLEAN is_reserved_word();
    while ((char_code(ch) == LETTER) || (char_code(ch) == DIGIT))
    {
        *tokenp++ = ch;
        get_char();
    }
    *tokenp = '\0';

    downshift_word();
    if (!is_reserved_word())
        token = IDENTIFIER;
}

//判断是否为保留字
BOOLEAN is_reserved_word()
{
    int word_length = strlen(word_string);
    RW_STRUCT *rwp;
    if ((word_length >= MIN_RESERVED_WORD_LENGTH) && (word_length <= MAX_RESERVED_WORD_LENGTH))
    {
        for (rwp = rw_table[word_length]; rwp->string != NULL; ++rwp)
        {
            if (strcmp(word_string, rwp->string) == 0)
            {
                token = rwp->token_code;
                return (TRUE);
            }
        }
    }
    return (FALSE);
}
get_string()
{
    char *sp = literal.value.string;
    *tokenp++ = '\'';
    get_char();

    while (ch != EOF_CHAR)
    {

        if (ch == '\'')
        {
            *tokenp++ = ch;
            get_char();
            if (ch != '\'')
                break;
        }
        *tokenp++ = ch;
        *sp++ = ch;
        get_char();
    }
    *tokenp = '\0';
    *sp = '\0';
    token = STRING;
    literal.type = STRING_LIT;
}
//打开源文件并获取第一个字符
open_source_file(char *name)
{
    if ((name == NULL) || ((source_file = fopen(name, "r")) == NULL))
    {
        printf("*** Error:  Failed to open source file.\n");
        exit(-1);
    }
    bufferp = "";
    get_char();
}

//关闭文件
close_source_file()
{
    fclose(source_file);
}

/*-----------------------------------------*/
/*1.第一阶段的特殊字符只为PERID,其他的为ERROR
/*2.第二阶段提取全部特殊token
/*-----------------------------------------*/
get_special()
{
    *tokenp++ = ch;

    // token = (ch == '.') ? PERIOD : ERROR;
    switch (ch)
    {
    case '^':
        token = UPARROW;
        get_char();
        break;
    case '*':
        token = STAR;
        get_char();
        break;
    case '(':
        token = LPAREN;
        get_char();
        break;
    case ')':
        token = RPAREN;
        get_char();
        break;
    case '-':
        token = MINUS;
        get_char();
        break;
    case '+':
        token = PLUS;
        get_char();
        break;
    case '=':
        token = EQUAL;
        get_char();
        break;
    case '[':
        token = LBRACKET;
        get_char();
        break;
    case ']':
        token = RBRACKET;
        get_char();
        break;
    case ';':
        token = SEMICOLON;
        get_char();
        break;
    case ',':
        token = COMMA;
        get_char();
        break;
    case '/':
        token = SLASH;
        get_char();
        break;
    case ':':
        get_char();
        if (ch == '=')
        {
            *tokenp++ = '=';
            token = COLONEQUAL;
            get_char();
        }
        else
            token = COLON;
        break;
    case '<':
        get_char();
        if (ch == '=')
        {
            *tokenp++ = '=';
            token = LE;
            get_char();
        }
        else if (ch == '>')
        {
            *tokenp++ = '>';
            token = NE;
            get_char();
        }
        else
        {
            token = LT;
        }
        break;
    case '>':
        get_char();
        if (ch == '=')
        {
            *tokenp++ = '=';
            token = GE;
            get_char();
        }
        else
        {
            token = GT;
        }
        break;
    case '.':
        get_char();
        if (ch == '.')
        {
            *tokenp++ = '.';
            token = DOTDOT;
            get_char();
        }
        else
        {
            token = PERIOD;
        }
        break;
    default:
        token = ERROR;
        get_char();
        break;
    }

    *tokenp = '\0';
}

get_number()
{
    float nvalue = 0.0;     // number值
    float evalue = 0.0;     //指数值
    int whole_count = 0;    /* no. digits in whole part */
    int decimal_offset = 0; /* no. digits to move decimal */
    BOOLEAN saw_dotdot = FALSE;
    char exponent_sign = '+';
    int exponent = 0;

    digit_count = 0;
    count_error = FALSE;
    token = NO_TOKEN;
    // do
    // {
    //     *tokenp++ = ch;
    //     if (++digit_count <= MAX_DIGIT_COUNT)
    //         nvalue = 10 * nvalue + (ch - '0');
    //     else
    //         count_error = TRUE;
    //     get_char();
    // } while (char_code(ch) == DIGIT);

    // if (count_error)
    // {
    //     token = ERROR;
    //     return;
    // }

    //先假设为Integer
    literal.type = INTEGER_LIT;
    /*
    --
    */
    accumulate_value(&nvalue, INVALID_NUMBER);
    if (token == ERROR)
        return;
    whole_count = digit_count;

    if (ch == '.')
    {
        get_char();
        if (ch == '.')
        {
            saw_dotdot = TRUE;
            --bufferp;
        }
        else
        {
            literal.type = REAL_LIT;
            *tokenp++ = '.';
            accumulate_value(&nvalue, INVALID_FRACTION);

            if (token == ERROR)
                return;
            decimal_offset = whole_count - digit_count;
        }
    }

    if (!saw_dotdot && ((ch == 'E') || (ch == 'e')))
    {
        literal.type = REAL_LIT;
        *tokenp++ = ch;
        get_char();
        if ((ch == '+') || (ch == '-'))
        {
            *tokenp++ = exponent_sign = ch;
            get_char();
        }
        accumulate_value(&evalue, INVALID_EXPONENT);
        if (token == ERROR)
            return;
        if (exponent_sign == '-')
            evalue = -evalue;
    }
    if (count_error)
    {
        error(TOO_MANY_DIGITS);
        token = ERROR;
        return;
    }

    exponent = evalue + decimal_offset;
    if ((exponent + whole_count < -MAX_EXPONENT) || (exponent + whole_count > MAX_EXPONENT))
    {
        error(REAL_OUT_OF_RANGE);
        token = ERROR;
        return;
    }
    if (exponent != 0)
    {
        nvalue *= pow(10, exponent);
    }

    if (literal.type == INTEGER_LIT)
    {
        if ((nvalue < -MAX_INTEGER) || (nvalue > MAX_INTEGER))
        {
            error(INTEGER_OUT_OF_RANGE);
            token = ERROR;
            return;
        }
        literal.value.integer = nvalue;
    }
    else
    {
        literal.value.real = nvalue;
    }
    // literal.value.integer = nvalue;
    *tokenp = '\0';
    token = NUMBER;
}

accumulate_value(float *valuep, ERROR_CODE error_code)
{
    float value = *valuep;
    //如果第一个字符不为digit则报错
    if (char_code(ch) != DIGIT)
    {
        error(error_code);
        token = ERROR;
        return;
    }
    //累加值，只要digit总数没有超过最大位数
    do
    {
        *tokenp++ = ch;
        if (++digit_count <= MAX_DIGIT_COUNT)
        {
            value = 10 * value + (ch - '0');
        }
        else
        {
            count_error = TRUE;
        }
        get_char();
    } while (char_code(ch) == DIGIT);
    *valuep = value;
}
skip_blanks()
{
    while (ch == ' ')
        get_char(); //取下一个字符
}

get_char()
{
    BOOLEAN get_source_line();
    if (*bufferp == '\0')
    {
        if (!get_source_line())
        {
            ch = EOF_CHAR;
            return;
        }
        bufferp = source_buffer;
        buffer_offset = 0;
    }
    ch = *bufferp++;
    // if ((ch == '\n') || (ch == '\t'))
    //     ch = ' ';
    switch (ch)
    {
    case '\t':
        buffer_offset += TAB_SIZE - buffer_offset % TAB_SIZE;
        ch = ' ';
        break;
    case '\n':
        ++buffer_offset;
        ch = ' ';
        break;
    case '{':
        ++buffer_offset;
        skip_comment();
        ch = ' ';
        break;
    default:
        ++buffer_offset;
        // printf("char %c\n",ch);
        // printf("buffer offset to %d \n",buffer_offset) ;

        break;
    }
}
skip_comment()
{
    do
    {
        get_char();
    } while ((ch != '}') && (ch != EOF_CHAR));
}
BOOLEAN get_source_line()
{
    char print_buffer[MAX_SOURCE_LINE_LENGTH + 9];

    if ((fgets(source_buffer, MAX_SOURCE_LINE_LENGTH, source_file)) != NULL)
    {
        line_number++;
        if (print_flag)
        {
            sprintf(print_buffer, "%4d %d: %s", line_number, level, source_buffer);
            print_line(print_buffer);
        }

        return TRUE;
    }
    else
        return FALSE;
}

print_line(char line[])
{
    char save_ch;
    char *save_chp = NULL;
    if (++line_count > MAX_LINES_PER_PAGE)
    {
        print_page_header();
        line_count = 1;
    }

    if (strlen(line) > MAX_PRINT_LINE_LENGTH)
        save_chp = &line[MAX_PRINT_LINE_LENGTH];
    if (save_chp)
    {
        save_ch = *save_chp;
        *save_chp = '\0';
    }
    printf(line);

    if (save_chp)
        *save_chp = save_ch;
}

print_page_header()
{
    putchar(FORM_FEED_CHAR);
    printf("Page %d     %s      %s\n\n", ++page_number, source_name, date);
}

init_page_header(name)
{
    time_t timer;

    strncpy(source_name, name, MAX_FILE_NAME_LENGTH - 1);
    //设置日期和时间
    time(&timer);
    strcpy(date, asctime(localtime(&timer)));
}

downshift_word()
{
    int offset = 'a' - 'A'; //偏移量，用来把字符转为小写
    char *wp = word_string;
    char *tp = token_string;

    //把字符转为小写
    do
    {
        *wp++ = (*tp >= 'A') && (*tp <= 'Z') ? *tp + offset : *tp;
        ++tp;
    } while (*tp != '\0');

    *wp = '\0';
}

quit_scanner()
{
    close_source_file();
}


BOOLEAN token_in(TOKEN_CODE token_list[])
{
    TOKEN_CODE  *tokenp;
    if(token_list == NULL)
        return (FALSE);
    for(tokenp = &token_list[0];*tokenp;++tokenp){
        if(token == *tokenp)
            return (TRUE);
    }
    return (FALSE);
}

/*--------------------------------------------------------------*/
/*  Token lists                                                 */
/*--------------------------------------------------------------*/

TOKEN_CODE statement_start_list[]   = {BEGIN, CASE, FOR, IF, REPEAT,
				       WHILE, IDENTIFIER, 0};

TOKEN_CODE statement_end_list[]     = {SEMICOLON, END, ELSE, UNTIL,
				       END_OF_FILE, 0};

TOKEN_CODE declaration_start_list[] = {CONST, TYPE, VAR, PROCEDURE,
				       FUNCTION, 0};

/*--------------------------------------------------------------*/
/*  synchronize         If the current token is not in one of   */
/*                      the token lists, flag it as an error.   */
/*                      Then skip tokens until one that is in   */
/*                      one of the token lists.                 */
/*--------------------------------------------------------------*/
synchronize(TOKEN_CODE token_list1[],TOKEN_CODE token_list2[],TOKEN_CODE token_list3[])
{
    BOOLEAN error_flag = (!token_in(token_list1)) && (!token_in(token_list2)) && (!token_in(token_list3));
    if(error_flag)
    {
        error(token == END_OF_FILE ? UNEXPECTED_END_OF_FILE : UNEXPECTED_TOKEN);
    }
    /*
	--  Skip tokens to resynchronize.
	*/
    while((!token_in(token_list1)) && (!token_in(token_list2)) && (!token_in(token_list3)) && (token != END_OF_FILE)){
        get_token();
    }
}