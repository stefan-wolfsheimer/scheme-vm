#ifndef __LISP_VALUES_H__
#define __LISP_VALUES_H__

struct lisp_eval_env_t;
struct lisp_cell_t;

int lisp_builtin_values(struct lisp_eval_env_t * env,
			struct lisp_cell_t     * stack);

#endif

