#ifndef __LISP_EXCEPTION_H__
#define __LISP_EXCEPTION_H__
#include "lisp_type.h"
#include "lisp_vm.h"
#include <stdarg.h>

typedef struct lisp_exception_t
{
  lisp_integer_t  error_code;
  lisp_cell_t     error_message;
  lisp_byte_code_t * lambda; /*@todo convert to lisp_lambda_t */
  lisp_size_t     pc;
} lisp_exception_t;


int lisp_make_exception(lisp_vm_t         * vm,
			lisp_cell_t       * cell,
			lisp_integer_t      code,
			lisp_byte_code_t     * lambda,
			lisp_size_t         pc,
			const lisp_char_t * msg,
			...);
void lisp_exception_destruct(lisp_vm_t * vm, void * ptr);

#endif
