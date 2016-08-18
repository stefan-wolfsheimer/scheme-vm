#ifndef __XSTRING_H__
#define __XSTRING_H__
#include <stdarg.h>
#include <stdlib.h>

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

char * alloc_join(const char * glue,
                  const char ** arr, size_t n);
char * alloc_join_cstr(const char * glue, const char * fmt,
                       const char ** arr, size_t n);
char * alloc_join_i(const char * glue, const char * fmt,
                    const int * arr, size_t n);
char * alloc_join_u(const char * glue, const char * fmt,
                    const unsigned * arr, size_t n);
char * alloc_join_ptr(const char * glue, const char * fmt,
                      const void ** arr, size_t n);

#endif
