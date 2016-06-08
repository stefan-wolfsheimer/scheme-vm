#include "lisp_vm.h"
#include "util/xmalloc.h"
#include "core/lisp_eval.h"
#include "core/lisp_symbol.h"
#include "core/lisp_exception.h"
#include <string.h>

static int lisp_lambda_eval_asm(lisp_eval_env_t * env,
				lisp_lambda_t   * lambda);

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
  lambda->instr_size = 0;
  lambda->data_size  = 0;

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
  lambda->instr_size = 0;
  lambda->data_size  = 0;
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
		     const lisp_cell_t  * rest)
{
  /* @todo: define argument signature class
     @todo: eval instead of copy 
     @todo: use stack instead of rest 
     @todo: remove arguments from stack after calling function
     @todo: create function to match rest with function signature
  */
  const lisp_cell_t * current = rest;
  lisp_integer_t n = 0;
  lisp_size_t i;
  int ret = LISP_OK;
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
  if(lambda->func) 
  {
    /* @todo implement lambda->func as asm instruction */
    lambda->func(env, 
		 stack_frame);
  }
  else 
  {
    ret = lisp_lambda_eval_asm(env, lambda);
  }
  for(i = 0; i < n; i++) 
  {
    lisp_unset_object_root(env->vm, &stack_frame[i]);
  }
  FREE(stack_frame);
  return ret;
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
 * start:  LDV A
 *         RET           
 *
 ****************
 * S1
 *
 * start:  LDVR  S1 ; deref and ld v
 *         RET
 *
 ****************
 * (S1 A B)
 * 
 * start:  
 *         PUSHR  S1   ; deref S1
 *         PUSHD  A
 *         PUSHD  B
 *         JP     3     ; (or CALL 3; RET)
 *
 * (S1 A S2)
 * 
 * start:
 *        PUSHR S1
 *        PUSHD A
 *        PUSHR S2
 *        JP    3      ; (or CALL 3; RET)
 *
 *****************
 * (S1 A (S2 B C) D )
 *
 * start: 
 *        PUSHR S1  ; deref S1 
 *        PUSHD A 
 *        PUSHR S2  ; deref S2
 *        PUSHD B 
 *        PUSHD C 
 *        CALL  3    ; (S2 B C) -> V
 *        PUSHV      
 *        PUSHD D
 *        JP 4       ; (or CALL 4; RET)
 * 
 ****************
 * (LET ((S1 A))
 *   S1)
 * 
 * start:
 *         PUSHD A
 *         CLOSURE 1,S1
 *         LDRV S1    ; ld v <- deref S1     
 *         POP
 *         RET
 *******************
 * (LET* ((S1 A)
 *        (S2 (+ S1 1)))
 *     (CONS S1 S2))
 *
 * start:
 *        PUSHD A
 *        CLOSURE 1,S1
 *        PUSHR +
 *        PUSHR S1
 *        PUSHD 1
 *        CALL  3       ; (+ S1 1)
 *        PUSHV         
 *        CLOSURE 1, S2
 *        PUSHR   CONS
 *        PUSHR   S1
 *        PUSHR   S2
 *        CALL    3     ; (CONS S1 S2)
 *        POP
 *        POP 
 *        RET
 *
 ***********************
 * (LET ((S1 A))
 *    (LET ((S1 B)
 *          (S2 S1))
 *      (CONS S1 S2)))
 *
 * start:
 *        PUSHD A
 *        CLOSURE 1, S1
 *        PUSHD B
 *        PUSHD S1
 *        CLOSURE  2,S1
 *        CLOSURE  1,S2
 *        PUSHR CONS
 *        PUSHR S1
 *        PUSHR S2
 *        CALL  2
 *
 * (LET ((S1 A))
 *    (LET* ((S1 B)
 *           (S2 S1))
 *      (CONS S1 S2)))
 *
 * start:
 *        PUSHD A
 *        CLOSURE 1, S1
 *        PUSHD B
 *        CLOSURE  1,S1
 *        PUSHD S1
 *        CLOSURE  1,S2
 *        PUSHR CONS
 *        PUSHR S1
 *        PUSHR S2
 *        CALL  2
 *
 ****************
 * (LET ((S1 A)
 *       (S2 (S3 B C)))
 *  (S4 S1 S2))
 *
 * start:
 *        LDV A
 *        CLOSURE 1,S1
 *        PUSHR S3
 *        PUSHD B
 *        PUSHD C
 *        CALL  3      ; (S3 B C)
 *        PUSHV
 *        CLOSURE 1,S2
 *        PUSHR S4
 *        PUSHR S1
 *        PUSHR S2
 *        CALL  3      ; stack_frame: 2
 *        POP
 *        POP
 *        RET
 *
 ****************
 * (LAMBDA (S1) 
 *     (S2 S1))
 *
 * start:
 *        PUSHR LAMBDA
 *        PUSHD (S1)
 *        PUSHD (S2 S1)
 *        JP    3
 * 
 * LAMBDA:
 *        CLOSURE arg1
 *        CLOSURE arg2
 *        ....
 *        POP
 *        POP

 ***********************
 * ((LAMBDA (S1) (S2 S1)) A)
 * start:
 *        PUSHR LAMBDA
 *        PUSHD (S1)
 *        PUSHD (S2 S1)
 *        CALL  3
 *        PUSHV
 *        PUSHD A
 *        JP    2
 *
 ***********************
 * (DEFINE S1 (LAMBDA () A))
 *
 * start:
 *        PUSHR DEFINE
 *        PUSHR S1
 *        PUSHR LAMBDA
 *        PUSHD ()
 *        PUSHD A
 *        CALL  3     ; (LAMBDA () A)
 *        PUSHV
 *        JP    3     ; (DEFINE S1 ?)
 *
 ************************
 * (DEFINE counter (LET ((C 0))
 *              (LAMBDA () (begin (SET! C (+ C 1)) C))))
 * start:
 *         PUSHR   DEFINE
 *         PUSHR   counter
 *         PUSHD   0
 *         CLOSURE 1,C
 *         PUSHR   LAMBDA
 *         PUSHD   ()
 *         PUSHD   (begin (SET! C (+ C 1)) C)
 *         CALL    3
 *         POP              ; closure C
 *         JP 2             ; (define counter ?)
 */


typedef unsigned char lisp_instr_t;
/* load cell data to value register */
#define LISP_ASM_LDVD  0x01
#define LISP_SIZ_LDVD  1 + sizeof(lisp_cell_t*)

#define LISP_ASM_LDVR  0x02
#define LISP_SIZ_LDVR  1 + sizeof(lisp_cell_t*)

#define LISP_ASM_RET     0x03
#define LISP_SIZ_RET     1

#define LISP_ASM_HALT    0x04
#define LISP_SIZ_HALT    1

static int lisp_lambda_eval_asm(lisp_eval_env_t * env,
				 lisp_lambda_t   * lambda)
{
  lisp_instr_t * instr = (lisp_instr_t*) lambda;
  instr+= sizeof(lisp_lambda_t);
  lisp_cell_t  * cell;
  lisp_size_t    pc = 0;
  while(1) 
  {
    switch(*instr) 
    {
    case LISP_ASM_LDVD:
      env->n_values = 1;
      lisp_copy_object_as_root(env->vm, env->values,
			       *((lisp_cell_t**)(instr + 1)));
      instr+= LISP_SIZ_LDVD;
      break;
    case LISP_ASM_LDVR:
      env->n_values = 1;
      REQUIRE(LISP_IS_SYMBOL(*((lisp_cell_t**)(instr + 1))));
      cell = lisp_symbol_get(env->vm, 
			     LISP_AS( *((lisp_cell_t**)(instr + 1)),
				      lisp_symbol_t));
      if(cell != NULL) 
      {
	lisp_copy_object_as_root(env->vm, env->values, cell);
      }
      else 
      {
	/* @todo check return code */
	lisp_make_exception(env->vm, 
			    env->values,
			    LISP_UNDEFINED,
			    NULL,
			    pc,
			    "Undefined symbol %s",
			    "xxx");
	return LISP_UNDEFINED;
      }
      instr+= LISP_SIZ_LDVR;
      break;
    case LISP_ASM_RET:
      instr+= LISP_SIZ_RET;
      break;
    case LISP_ASM_HALT:
      return LISP_OK;
    }
    pc++;
  }
  return LISP_UNSUPPORTED;
}

static int lisp_compile_alloc(lisp_cell_t   * cell,
			      lisp_instr_t ** instr,
			      lisp_cell_t  ** data,
			      lisp_size_t     instr_size,
			      lisp_size_t     data_size)
{
  /* @todo check allocation */
  lisp_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_lambda_t) + 
					 instr_size + 
					 data_size * sizeof(lisp_cell_t), 
					 1);
  lambda->instr_size = instr_size;
  lambda->data_size  = data_size;
  lambda->func       = NULL;
  cell->type_id      = LISP_TID_LAMBDA;
  cell->data.ptr     = lambda;
  *instr             = (lisp_instr_t*) &lambda[1];
  *data              = (lisp_cell_t*)  (*instr + instr_size);
  return LISP_OK;
}

static int lisp_compile_nil(lisp_eval_env_t   * env,
			    lisp_cell_t       * cell)
{
  /* @todo exception */
  return LISP_COMPILATION_ERROR;
}

static int lisp_compile_atom(lisp_eval_env_t   * env,
			     lisp_cell_t       * cell,
			     const lisp_cell_t * expr)
{
  /* @todo remove halt and embed it into execution env */
  int ret; 
  lisp_instr_t * instr = NULL;
  lisp_cell_t  * data  = NULL;
  ret = lisp_compile_alloc(cell, &instr, &data, 
			   LISP_SIZ_LDVD + LISP_SIZ_RET + LISP_SIZ_HALT,
			   1);
  if(ret == LISP_OK) 
  {
    /* LDVD expr
       RET
       HALT 
    */
    *data = *expr;

    *instr = LISP_ASM_LDVD;    
    instr++;
    *((lisp_cell_t**)instr) = data;
    instr+= sizeof(lisp_cell_t*);
    
    *instr = LISP_ASM_RET;
    instr++;
    
    *instr = LISP_ASM_HALT;
  }
  return ret;
}

static int lisp_compile_symbol(lisp_eval_env_t   * env,
			       lisp_cell_t       * cell,
			       const lisp_cell_t * expr)
{
    /* LDVR expr
       RET
       HALT 
    */
  int ret; 
  lisp_instr_t * instr = NULL;
  lisp_cell_t  * data  = NULL;
  ret = lisp_compile_alloc(cell, &instr, &data, 
			   LISP_SIZ_LDVD + LISP_SIZ_RET + LISP_SIZ_HALT,
			   1);
  if(ret == LISP_OK) 
  {
    *data = *expr;

    *instr = LISP_ASM_LDVR;    
    instr++;
    *((lisp_cell_t**)instr) = data;
    instr+= sizeof(lisp_cell_t*);
    
    *instr = LISP_ASM_RET;
    instr++;
    
    *instr = LISP_ASM_HALT;
  }
  return ret;
}

static int lisp_compile_object(lisp_eval_env_t   * env,
			       lisp_cell_t       * cell,
			       const lisp_cell_t * expr)
{
  lisp_instr_t * instr = NULL;
  lisp_cell_t  * data  = NULL;
  int            ret; 
  ret = lisp_compile_alloc(cell, &instr, &data, 
			   LISP_SIZ_LDVD + LISP_SIZ_RET + LISP_SIZ_HALT,
			   1);
  if(ret == LISP_OK) 
  {
    *instr = LISP_ASM_LDVD;    
    instr++;
    *((lisp_cell_t**)instr) = data;
    instr+= sizeof(lisp_cell_t*);

    *instr = LISP_ASM_RET;
    instr++;

    *instr = LISP_ASM_HALT;
    lisp_copy_object_as_root(env->vm, data, expr);
  }
  return ret;
}

static int lisp_compile_cons(lisp_eval_env_t * env,
			     lisp_cell_t     * cell,
			     const lisp_cell_t    * expr) 
{
  int ret = LISP_UNSUPPORTED;
  return ret;
}

int lisp_lambda_compile(lisp_eval_env_t   * env,
			lisp_cell_t       * cell,
			const lisp_cell_t * expr)
{
  *cell = lisp_nil;
  if(LISP_IS_NIL(expr)) 
  {
    /* before is_atom */
    return lisp_compile_nil(env, cell);
  }
  else if(LISP_IS_ATOM(expr)) 
  {
    return lisp_compile_atom(env, cell, expr);
  }
  else if(LISP_IS_SYMBOL(expr))
  {
    return lisp_compile_symbol(env, cell, expr);
  }
  else if(LISP_IS_OBJECT(expr))
  {
    return lisp_compile_object(env, cell, expr);
  }
  else if(LISP_IS_CONS(expr)) 
  {
    return lisp_compile_cons(env, cell, expr);
  }
  else 
  {
    return LISP_UNSUPPORTED;
  }
}


void lisp_lambda_destruct(lisp_vm_t * vm, void * ptr)
{
  lisp_size_t n = ((lisp_lambda_t*)(ptr))->data_size;
  lisp_cell_t * data = 
    (lisp_cell_t*)( (lisp_instr_t*)ptr + 
		    sizeof(lisp_lambda_t) + 
		    sizeof(lisp_instr_t) * 
		    ((lisp_lambda_t*)(ptr))->instr_size );
  lisp_size_t i;
  for(i = 0; i < n; i++) 
  {
    lisp_unset_object_root(vm, &data[i]);
  }
  FREE_OBJECT(ptr);
}

