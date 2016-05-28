#include "lisp_arithmetic.h"
#include "util/assertion.h"
#include "core/lisp_vm.h"

int lisp_builtin_plus(lisp_eval_env_t * env,
		      lisp_cell_t     * stack)
{
  REQUIRE(LISP_IS_INTEGER(stack));
  lisp_integer_t nargs = stack->data.integer;
  lisp_integer_t i;
  env->values->type_id      = LISP_TID_INTEGER;
  env->values->data.integer = 0;
  env->n_values             = 1;
  for(i = 0; i < nargs; i++) 
  {
    if(!LISP_IS_INTEGER(&stack[i+1])) 
    {
      *env->values = lisp_nil;
      return LISP_TYPE_ERROR;
    }
    env->values->data.integer+= stack[i+1].data.integer;
  }
  return LISP_OK;
}
