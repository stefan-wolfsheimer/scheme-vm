#ifndef __LISP_EVAL_H__
#define __LISP_EVAL_H__
#include "lisp_vm.h"

typedef struct lisp_eval_env_t
{
  lisp_vm_t * vm;
} lisp_eval_env_t;

lisp_eval_env_t * lisp_create_eval_env(lisp_vm_t * vm);
void lisp_free_eval_env(lisp_eval_env_t * env);
int lisp_eval(lisp_eval_env_t   * env,
	      lisp_cell_t       * cell,
	      const lisp_cell_t * expr);


/*@todo function signature */
int lisp_register_builtin_function(lisp_eval_env_t         * env,
				   const char              * name,
				   lisp_builtin_function_t   func);
				   
				   
/* @todo move to separate module 
 * @todo typecheck for values
 * @todo overflow check and high prec. arithmetric
 */
int lisp_builtin_plus(lisp_vm_t     * vm,
		      lisp_cell_t   * target,
		      lisp_cell_t   * stack);

#endif
