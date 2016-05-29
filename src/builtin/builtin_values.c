#include "builtin_values.h"
#include "core/lisp_vm.h"
#include "util/assertion.h"
#include "util/xmalloc.h"

static int lisp_builtin_values(lisp_eval_env_t * env,
			       lisp_cell_t     * stack)
{
  REQUIRE(LISP_IS_INTEGER(stack));
  lisp_integer_t nargs = stack->data.integer;
  size_t         i;
  if(nargs > env->max_values) 
  {
    lisp_cell_t * values;
    values = REALLOC(env->values, sizeof(lisp_cell_t)*nargs);
    if(values == NULL) 
    {
      return LISP_ALLOC_ERROR;
    }
    for(i = env->max_values; i < nargs; i++) 
    {
      values[i] =  lisp_nil;
    }
    env->values = values;
    env->max_values = nargs;
  }
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  for(i = 0; i < nargs; i++) 
  {
    lisp_copy_object_as_root(env->vm, &env->values[i], &stack[i+1]);
  }
  env->n_values = nargs;
  return LISP_OK;
}

int lisp_make_func_values(lisp_vm_t * vm, 
			  lisp_cell_t * cell)
{
  /* @todo make arguments */
  return lisp_make_builtin_lambda(vm, cell, 0, NULL, lisp_builtin_values);
}

