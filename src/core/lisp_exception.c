#include "lisp_exception.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void _lisp_make_exception_msg(lisp_cell_t       * target,
				     const lisp_char_t * msg,
				     va_list             va)
{
  /*@todo malloc error handling */
  int             size;
  lisp_string_t * str;
  va_list         va2;
  va_copy(va2, va);
  size = vsnprintf(NULL, 0, msg, va);
  va_end(va);
  str = MALLOC_OBJECT(sizeof(lisp_string_t), 1);
  str->data  = MALLOC_OBJECT(sizeof(lisp_char_t) * (size+1), 1);
  str->begin = 0;
  str->end   = size;
  vsprintf(str->data, msg, va2);
  va_end(va2);
  target->type_id = LISP_TID_STRING;
  target->data.ptr = str;
}

int lisp_make_exception(lisp_vm_t         * vm,
			lisp_cell_t       * cell,
			lisp_integer_t      code,
			lisp_byte_code_t     * lambda,
			lisp_size_t         pc,
			const lisp_char_t * msg,
			...)
{
  lisp_exception_t * exception;  
  exception = MALLOC_OBJECT(sizeof(lisp_exception_t), 1);
  if(!exception) 
  {
    /* @todo fatal exception */
  }
  va_list va;
  va_start(va, msg);
  _lisp_make_exception_msg(&exception->error_message, msg, va);
  exception->error_code = code;
  exception->pc = pc;
  exception->lambda = lambda;
  cell->type_id = LISP_TID_EXCEPTION;
  cell->data.ptr = exception;
  return LISP_OK;
}

void lisp_exception_destruct(lisp_vm_t * vm, void * ptr)
{
  lisp_unset_object_root(vm, &((lisp_exception_t*)ptr)->error_message);
  FREE_OBJECT(ptr);
}
