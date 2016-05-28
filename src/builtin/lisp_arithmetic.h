#ifndef __LISP_ARITHMETIC_H__
#define __LISP_ARITHMETIC_H__

struct lisp_eval_env_t;
struct lisp_cell_t;

/* 
 * @todo typecheck for values
 * @todo overflow check and high prec. arithmetric
 */
int lisp_builtin_plus(struct lisp_eval_env_t * env,
		      struct lisp_cell_t     * stack);

#endif
