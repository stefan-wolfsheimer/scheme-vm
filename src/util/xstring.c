#include "xstring.h"
#include "xmalloc.h"
#include <string.h>
#include <stdio.h>

char * alloc_strcpy(const char * str)
{
  if(str == NULL) 
  {
    return NULL;
  }
  else 
  {
    size_t len = strlen(str);
    char * tmp = MALLOC(len + 1);
    if(tmp != NULL) 
    {
      strcpy(tmp, str);
    }
    return tmp;
  }
}

char * alloc_sprintf(const char * fmt, ...)
{
  int       size;
  va_list   ap1, ap2;
  char    * ret;
  va_start(ap1, fmt);
  va_copy(ap2, ap1);
  size = vsnprintf(NULL, 0, fmt, ap1);
  va_end(ap1);
  ret = MALLOC( sizeof(char) * (size+1) );
  if(ret) 
  {
    vsprintf(ret, fmt, ap2);  
  }
  va_end(ap2);
  return ret;
}

char * alloc_join(const char * glue,
                  const char ** arr, size_t n)
{
  return alloc_join_cstr(glue, "%s", arr, n);
}

#define ALLOC_JOIN(__TYPE__, __NAME__)                          \
  char * alloc_join_##__NAME__(const char * glue,               \
                               const char * fmt,                \
                               const __TYPE__ * arr,            \
                               size_t n)                        \
  {                                                             \
    size_t glue_size = strlen(glue);                            \
    size_t size = (n > 0) ? (glue_size * (n-1)) : 0;            \
    size_t i;                                                   \
    for(i = 0; i < n; i++)                                      \
    {                                                           \
      size+= snprintf(NULL, 0, fmt, arr[i]);                    \
    }                                                           \
    char * ret = MALLOC( sizeof(char) * (size+1) );             \
    char * current = ret;                                       \
    if(ret)                                                     \
    {                                                           \
      for(i = 0; i < n; i++)                                    \
      {                                                         \
        if(i > 0)                                               \
        {                                                       \
          strcpy(current, glue);                                \
          current+= glue_size;                                  \
        }                                                       \
        current+= sprintf(current, fmt, arr[i]);                \
      }                                                         \
    }                                                           \
    *current = '\0';                                            \
    return ret;                                                 \
  }

ALLOC_JOIN(char *, cstr)
ALLOC_JOIN(int, i)
ALLOC_JOIN(unsigned, u)
ALLOC_JOIN(void *, ptr)

#undef ALLOC_JOIN
