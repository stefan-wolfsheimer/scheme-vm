#include "lisp_exception.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int lisp_make_va_exception(lisp_vm_t         * vm,
                           lisp_cell_t       * exception,
                           lisp_integer_t      code,
                           lisp_lambda_t     * lambda,
                           lisp_size_t         pc,
                           const lisp_char_t * msg,
                           va_list             va)
{
  int         ret;
  lisp_cell_t elems[4];

  /* code */
  lisp_make_integer(&elems[0], code);

  /* message */
  lisp_va_sprintf(vm, &elems[1], msg, va);

  /* lambda */
  if(lambda == NULL)
  {
    elems[2] = lisp_nil;
  }
  else
  {
    elems[2].type_id = LISP_TID_LAMBDA;
    elems[2].data.ptr = lambda;
  }

  /* pc */
  lisp_make_integer(&elems[3], pc);

  ret = lisp_make_list_root_typed(vm,
                                  exception,
                                  LISP_TID_EXCEPTION,
                                  elems,
                                  4u);
  lisp_unset_object(vm, &elems[1]);
  return ret;
}

int lisp_make_exception(lisp_vm_t         * vm,
                        lisp_cell_t       * exception,
                        lisp_integer_t      code,
                        lisp_lambda_t     * lambda,
                        lisp_size_t         pc,
                        const lisp_char_t * msg,
                        ...)
{
  int     ret;
  va_list va;
  va_start(va, msg);
  ret = lisp_make_va_exception(vm,
                               exception,
                               code,
                               lambda,
                               pc,
                               msg,
                               va);
  va_end(va);
  return ret;
}

void lisp_raise_va_exception(lisp_eval_env_t   * env,
                             lisp_integer_t      code,
                             lisp_lambda_t     * lambda,
                             lisp_size_t         pc,
                             const lisp_char_t * msg,
                             va_list va)
{
  int ret;
  ret = lisp_unset_object(env->vm, &env->exception);
  if(ret != LISP_OK)
  {
    /* @todo fatal */
  }
  ret = lisp_make_va_exception(env->vm,
                               &env->exception,
                               code,
                               lambda,
                               pc,
                               msg,
                               va);
  if(ret != LISP_OK)
  {
    /* @todo fatal */
  }
}

void lisp_raise_exception(lisp_eval_env_t   * env,
                          lisp_integer_t      code,
                          lisp_lambda_t     * lambda,
                          lisp_size_t         pc,
                          const lisp_char_t * msg,
                          ...)
{
  va_list va;
  va_start(va, msg);
  lisp_raise_va_exception(env,
                          code,
                          lambda,
                          pc,
                          msg,
                          va);
  va_end(va);
}

int lisp_exception_code(const lisp_cell_t * cell)
{
  if(LISP_IS_EXCEPTION(cell))
  {
    return LISP_CAR(cell)->data.integer;
  }
  else
  {
    return LISP_OK;
  }
}

const lisp_cell_t * lisp_exception_message(const lisp_cell_t * cell)
{
  if(LISP_IS_EXCEPTION(cell))
  {
    return LISP_CADR(cell);
  }
  else
  {
    /* @todo return empty string constant */
    return NULL;
  }
}

const lisp_cell_t * lisp_exception_lambda(const lisp_cell_t * cell)
{
  if(LISP_IS_EXCEPTION(cell))
  {
    return LISP_CADDR(cell);
  }
  else
  {
    return &lisp_nil;
  }
}

lisp_integer_t lisp_exception_pc(const lisp_cell_t * cell)
{
  if(LISP_IS_EXCEPTION(cell))
  {
    return LISP_CADDDR(cell)->data.integer;
  }
  else
  {
    return 0;
  }
}
