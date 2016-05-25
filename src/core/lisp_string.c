#include "lisp_vm.h"
#include "util/xmalloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*@todo eq_string */

int lisp_make_string(lisp_vm_t         * vm,
                     lisp_cell_t       * cell,
                     const lisp_char_t * cstr)
{
  size_t size = strlen(cstr);
  lisp_string_t * str = MALLOC_OBJECT(sizeof(lisp_string_t), 1);
  str->data  = MALLOC_OBJECT(sizeof(lisp_char_t) * (size+1), 1);
  str->begin = 0;
  str->end   = size;
  strcpy( (char*) str->data, cstr);
  cell->type_id = LISP_TID_STRING;
  cell->data.ptr = str;
  return 0;
}

int lisp_make_substring(lisp_vm_t           * vm,
			lisp_cell_t         * target,
			const lisp_string_t * str,
			lisp_size_t           a,
			lisp_size_t           b)
{
  if(a > b) 
  {
    *target  = lisp_nil; /* @todo exception */
    return LISP_RANGE_ERROR;
  }
  else if(a > str->end) 
  {
    *target  = lisp_nil; /* @todo exception */
    return LISP_RANGE_ERROR;
  }
  else if(b > str->end) 
  {
    *target  = lisp_nil; /* @todo exception */
    return LISP_RANGE_ERROR;
  }
  ((lisp_ref_count_t *) str->data)[-1]++;
  lisp_string_t * substr = MALLOC_OBJECT(sizeof(lisp_string_t), 1);
  substr->data  = str->data;
  substr->begin = a;
  substr->end   = b;
  target->type_id  = LISP_TID_STRING;
  target->data.ptr = (void*)substr;
  return LISP_OK;
}


int lisp_sprintf(lisp_vm_t         * vm,
                 lisp_cell_t       * cell,
                 const lisp_char_t * fmt,
                 ...)
{
  int ret;
  int size;
  va_list ap1, ap2;
  va_start(ap1, fmt);
  va_copy(ap2, ap1);
  size = vsnprintf(NULL, 0, fmt, ap1);
  va_end(ap1);
  lisp_string_t * str = MALLOC_OBJECT(sizeof(lisp_string_t), 1);
  str->data  = MALLOC_OBJECT(sizeof(lisp_char_t) * (size+1), 1);;
  str->begin = 0;
  str->end   = size;
  ret = vsprintf(str->data, fmt, ap2);
  va_end(ap2);
  cell->type_id = LISP_TID_STRING;
  cell->data.ptr = str;
  return ret;
}

lisp_size_t lisp_string_length(const lisp_string_t * str)
{
  return str->end - str->begin;
}

const char * lisp_c_string(const lisp_cell_t * cell)
{
  /* @todo use lisp_string_t as argument */
  return (char*)LISP_AS(cell, lisp_string_t)->data;
}

int lisp_string_cmp_c_string(const lisp_string_t * a, const char * cstr)
{
  size_t rhs_l = strlen(cstr);
  int ret = strncmp(a->data + a->begin, cstr, rhs_l);
  if(ret == 0) 
  {
    if((a->end - a->begin) == rhs_l) 
    {
      return 0;
    }
    else if((a->end - a->begin) < rhs_l) 
    {
      return -1;
    }
    else 
    {
      return 1;
    }
  }
  return ret;
}
