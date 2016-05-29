#ifndef __LISP_EVAL_H__
#define __LISP_EVAL_H__
#include "lisp_vm.h"


/** Create a enviornment for evaluation 
 */
lisp_eval_env_t * lisp_create_eval_env(lisp_vm_t * vm);

/** Free the enviornment for evalutation 
 */
void lisp_free_eval_env(lisp_eval_env_t * env);

/** Evaluates a lisp expression and stores the 
 *  result to env->values
 */
int lisp_eval(lisp_eval_env_t   * env,
	      const lisp_cell_t * expr);

/*@todo function signature */
int lisp_register_builtin_function(lisp_eval_env_t         * env,
				   const char              * name,
				   lisp_builtin_function_t   func);
#endif
