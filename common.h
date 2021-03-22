#ifndef common_h
#define common_h

#define FORM_FEED_CHAR		'\f'
#define MAX_SOURCE_LINE_LENGTH  256
#define MAX_PRINT_LINE_LENGTH   80
#define MAX_LINES_PER_PAGE      50
#define MAX_TOKEN_STRING_LENGTH MAX_SOURCE_LINE_LENGTH

typedef enum{
    FALSE,TRUE,
}BOOLEAN;

		/****************************************/
		/*                                      */
		/*      Macros for memory allocation    */
		/*                                      */
		/****************************************/
#define     alloc_struct(type)      (type*)malloc(sizeof(type))
#define     alloc_array(type,count) (type*)malloc(count*sizeof(type))
#define     alloc_bytes(length)     (char*)malloc(length)


#endif