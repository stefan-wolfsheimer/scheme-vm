#include "lisp_vm.h"
#include "util/xmalloc.h"
#include "core/lisp_eval.h"
#include <string.h>

int lisp_make_builtin_lambda(lisp_vm_t   * vm,
			     lisp_cell_t * cell,
			     lisp_size_t   args_size,
			     lisp_cell_t * args,
			     lisp_builtin_function_t  func)
{
  lisp_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_lambda_t),
					 1);
  cell->type_id  = LISP_TID_LAMBDA;
  cell->data.ptr = lambda;
  lambda->func   = func;
  /* @todo alloc check */
  /* @todo arguments */
  return LISP_OK;
}

int lisp_make_builtin_form(lisp_vm_t                * vm,
			   lisp_cell_t              * cell,
			   lisp_size_t                args_size,
			   lisp_cell_t              * args,
			   lisp_builtin_function_t    func)
{
  lisp_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_lambda_t),
					 1);
  cell->type_id  = LISP_TID_FORM;
  cell->data.ptr = lambda;
  lambda->func   = func;
  return LISP_OK;
}

int lisp_make_builtin_c_str(lisp_vm_t                * vm,
			    lisp_cell_t              * cell,
			    lisp_size_t                args_size,
			    const char              ** args,
			    lisp_builtin_function_t    func)
{
  lisp_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_lambda_t),
					 1);
  cell->type_id  = LISP_TID_FORM;
  cell->data.ptr = lambda;
  lambda->func   = func;
  return LISP_OK;
}


int lisp_make_builtin_lambda_opt_args(lisp_vm_t   * vm,
				      lisp_cell_t * cell,
				      lisp_size_t   args_size,
				      lisp_cell_t * args,
				      lisp_size_t   opt_args_size,
				      lisp_cell_t * opt_args,
				      lisp_size_t   named_args_size,
				      lisp_cell_t * named_args,
				      lisp_cell_t * named_args_values,
				      int           has_rest_args,
				      lisp_builtin_function_t  func)
{
  /* @todo implement */
  return LISP_OK;
}

int lisp_eval_lambda(lisp_eval_env_t    * env,
		     lisp_lambda_t      * lambda,
		     lisp_cell_t        * rest)
{
  /* @todo: define argument signature class
     @todo: eval instead of copy */
  const lisp_cell_t * current = rest;
  lisp_integer_t n = 0;
  lisp_size_t i;
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  env->n_values = 0;
  while(!LISP_IS_NIL(current)) 
  {
    if(LISP_IS_CONS_OBJECT(current)) 
    {
      n++;
      current = &LISP_AS(current, lisp_cons_t)->cdr;
    }
    else 
    {
      /* @todo error */
    }
  }
  lisp_cell_t * stack_frame;
  stack_frame = MALLOC(sizeof(lisp_cell_t) * (n+1));
  lisp_make_integer(stack_frame, n);
  n = 1;
  current = rest;
  while(!LISP_IS_NIL(current)) 
  {
    if(LISP_IS_CONS_OBJECT(current)) 
    {
      /* @todo error handling */
      lisp_eval(env, &LISP_AS(current, lisp_cons_t)->car);
      lisp_copy_object_as_root(env->vm, &stack_frame[n], env->values);
      current = &LISP_AS(current, lisp_cons_t)->cdr;
      n++;
    }
    else 
    {
      /* @todo error */
    }
  }
  lambda->func(env, 
	       stack_frame);
  for(i = 0; i < n; i++) 
  {
    lisp_unset_object_root(env->vm, &stack_frame[i]);
  }
  FREE(stack_frame);
  return LISP_OK;
}

int lisp_eval_form(lisp_eval_env_t    * env,
		   lisp_lambda_t      * lambda,
		   lisp_cell_t        * rest)
{
  /* @todo: define argument signature class
     @todo: eval instead of copy */
  const lisp_cell_t * current = rest;
  lisp_integer_t n = 0;
  lisp_size_t i;
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  env->n_values = 0;
  while(!LISP_IS_NIL(current)) 
  {
    if(LISP_IS_CONS_OBJECT(current)) 
    {
      n++;
      current = &LISP_AS(current, lisp_cons_t)->cdr;
    }
    else 
    {
      /* @todo error */
    }
  }
  lisp_cell_t * stack_frame;
  stack_frame = MALLOC(sizeof(lisp_cell_t) * (n+1));
  lisp_make_integer(stack_frame, n);
  n = 1;
  current = rest;
  while(!LISP_IS_NIL(current)) 
  {
    if(LISP_IS_CONS_OBJECT(current)) 
    {
      /* @todo error handling */
      lisp_copy_object_as_root(env->vm, 
			       &stack_frame[n], 
			       &LISP_AS(current, lisp_cons_t)->car);
      current = &LISP_AS(current, lisp_cons_t)->cdr;
      n++;
    }
    else 
    {
      /* @todo error */
    }
  }
  lambda->func(env, 
	       stack_frame);
  for(i = 0; i < n; i++) 
  {
    lisp_unset_object_root(env->vm, &stack_frame[i]);
  }
  FREE(stack_frame);
  return LISP_OK;
}

/****************
 * Atom literals: A,B,C
 * Symbols:       S1, S2
 *
 * A
 *
 * start:  LD V,&c0 
 *         RET
 * c0:     A
 *
 ****************
 * S1
 *
 * start:  LDR V, &c0 ;deref and ld v
 *         RET
 * c0:     S1
 *
 ****************
 * (S1 A B)
 * 
 * start:  
 *         PUSHD  &c0
 *         PUSHD  &c1
 *         PUSHN  2
 *         JP     #S1    ;(or CALL #S1; RET)
 * c0:     A
 * c1:     B
 *
 * (S1 A (S2 B C) D )
 *
 * start: 
 *        PUSHD &c0
 *        PUSHD &c1
 *        PUSHD &c2
 *        PUSHN 2
 *        PUSHR &c4  ; deref symbol &c4 and push
 *        CALL
 *        PUSHV
 *        PUSHD &c3
 *        PUSHN 3
 *        PUSHR &c5 ; deref symbol &c4 and push
 *        JP
 * c0:    A
 * c1:    B
 * c2:    C
 * c3:    D
 * c4:    S1
 * c5:    S2
 * 
 ***********************/
int lisp_lambda_compile(lisp_eval_env_t * env,
			lisp_cell_t     * cell,
			lisp_cell_t     * expr)
{
  return LISP_OK;
}


