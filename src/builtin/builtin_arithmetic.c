#include "builtin_arithmetic.h"
#include "util/assertion.h"
#include "core/lisp_vm.h"
#include "core/lisp_lambda.h"
/* 
 * @todo typecheck for values
 * @todo overflow check and high prec. arithmetric
 */
static int lisp_builtin_plus(lisp_eval_env_t     * env,
                             const lisp_lambda_t * lambda,
                             lisp_size_t           nargs)
{
  lisp_cell_t * stack = env->stack + env->stack_top - nargs;
  lisp_integer_t i;
  env->values->type_id      = LISP_TID_INTEGER;
  env->values->data.integer = 0;
  env->n_values             = 1;
  for(i = 0; i < nargs; i++) 
  {
    if(!LISP_IS_INTEGER(&stack[i])) 
    {
      *env->values = lisp_nil;
      return LISP_TYPE_ERROR;
    }
    env->values->data.integer+= stack[i].data.integer;
  }
  return LISP_OK;
}

int lisp_make_func_plus(struct lisp_vm_t * vm,
                        struct lisp_cell_t * cell)
{
  /* @todo make arguments */
  return lisp_make_builtin_lambda(vm,
                                  cell,
                                  0,
                                  NULL,
                                  lisp_builtin_plus);
}

