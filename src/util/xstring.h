#ifndef __XSTRING_H__
#define __XSTRING_H__
#include <stdarg.h>
/** 
 * Allocate and copy string.
 * If the memory could not be allocated return NULL
 *
 * @return copied string or NULL 
 */
char * alloc_strcpy(const char * str);

/** 
 * Allocate, copy formated string.
 * 
 * @return formatted string (or NULL of allocation fails)
 */
char * alloc_sprintf(const char * fmt, ...);

#endif
