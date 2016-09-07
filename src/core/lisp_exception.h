#ifndef __LISP_EXCEPTION_H__
#define __LISP_EXCEPTION_H__
#include "lisp_type.h"
#include "lisp_vm.h"
#include <stdarg.h>

int lisp_make_exception(lisp_vm_t         * vm,
                        lisp_cell_t       * exception,
                        lisp_integer_t      code,
                        lisp_lambda_t     * lambda,
                        lisp_size_t         pc,
                        const lisp_char_t * msg,
                        ...);

int lisp_make_va_exception(lisp_vm_t         * vm,
                           lisp_cell_t       * exception,
                           lisp_integer_t      code,
                           lisp_lambda_t     * lambda,
                           lisp_size_t         pc,
                           const lisp_char_t * msg,
                           va_list             va);

void lisp_raise_exception(lisp_eval_env_t   * env,
                          lisp_integer_t      code,
                          lisp_lambda_t     * lambda,
                          lisp_size_t         pc,
                          const lisp_char_t * msg,
                          ...);

void lisp_raise_va_exception(lisp_eval_env_t   * env,
                             lisp_integer_t      code,
                             lisp_lambda_t     * lambda,
                             lisp_size_t         pc,
                             const lisp_char_t * msg,
                             va_list va);

int lisp_exception_code(const lisp_cell_t * cell);
const lisp_cell_t * lisp_exception_message(const lisp_cell_t * cell);
const lisp_cell_t * lisp_exception_lambda(const lisp_cell_t * cell);
lisp_integer_t lisp_exception_pc(const lisp_cell_t * cell);

#define LISP_EXCEPTION(__ENV__)                          \
  ( LISP_IS_EXCEPTION(&(__ENV__)->exception) ?           \
    LISP_CAR(&(__ENV__)->exception)->data.integer :      \
    LISP_OK )

#endif
