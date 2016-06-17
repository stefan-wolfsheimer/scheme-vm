#include "builtin_compile.h"
#include "core/lisp_vm.h"
#include "util/assertion.h"
#include "util/xmalloc.h"

static int lisp_builtin_compile(lisp_eval_env_t * env,
                                lisp_cell_t     * stack)
{
  return LISP_OK;
}

int lisp_make_func_compile(lisp_vm_t * vm, 
                           lisp_cell_t * cell)
{
  /* @todo make arguments 
     @todo static data size
   */
  return lisp_make_builtin_lambda(vm, cell, 0, NULL, lisp_builtin_compile);
}

