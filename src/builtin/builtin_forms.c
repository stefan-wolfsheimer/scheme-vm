#include "builtin_forms.h"
#include "util/assertion.h"
#include "core/lisp_vm.h"
#include "core/lisp_eval.h"
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"

static int lisp_builtin_define(lisp_eval_env_t * env,
                               const lisp_lambda_t * lambda,
                               lisp_size_t       nargs)
{
  lisp_cell_t  * stack =  env->stack + env->stack_top - nargs;
  REQUIRE_EQ_I(stack->data.integer, 2);
  if(LISP_IS_SYMBOL(&stack[1])) 
  {
    /* @todo errror handling */
    //lisp_eval(env, &stack[2]);
    lisp_symbol_set(env->vm,
                    LISP_AS(&stack[1], lisp_symbol_t),
                    env->values);
    return LISP_OK;
  }    
  else 
  {
    return LISP_TYPE_ERROR;
  }
}

int lisp_make_form_define(lisp_vm_t * vm, 
                          lisp_cell_t * cell)
{
  /* @todo arguments for form define */
  return lisp_make_builtin_form(vm,
                                cell,
                                0,
                                NULL,
                                0,
                                NULL,
                                lisp_builtin_define);
}

