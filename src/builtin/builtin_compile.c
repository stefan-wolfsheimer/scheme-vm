#include "builtin_compile.h"
#include "core/lisp_vm.h"
#include "core/lisp_lambda.h"
//#include "util/assertion.h"
//#include "util/xmalloc.h"


/****************************************************************************/
static int _lisp_builtin_compile(lisp_eval_env_t      * env,
                                 const lisp_lambda_t  * lambda,
                                 lisp_size_t            nargs)
{
  int ret = LISP_OK;
  /* @TODO check nargs */
  const lisp_cell_t * expr = env->stack + env->stack_top - nargs;
  env->n_values            = 1;
  ret = lisp_lambda_compile(env,
                            env->values,
                            expr);
  return ret;
}

int lisp_make_func_compile(lisp_vm_t * vm, 
                           lisp_cell_t * cell)
{
  /* @todo make arguments 
     @todo static data size
   */
  return lisp_make_builtin_lambda(vm, cell, 0, NULL, _lisp_builtin_compile);
}

