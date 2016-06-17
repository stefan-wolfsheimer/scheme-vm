#include "lisp_lambda.h"
#include "lisp_vm.h"
#include "util/xmalloc.h"
#include "core/lisp_eval.h"
#include "core/lisp_symbol.h"
#include "core/lisp_exception.h"
#include <string.h>

typedef unsigned char lisp_instr_t;
/* load cell data to value register */
#define LISP_INSTR_SIZE 

#define LISP_ASM_LDVD      0x01
#define LISP_SIZ_LDVD      sizeof(lisp_instr_t) + sizeof(lisp_cell_t)

#define LISP_ASM_LDVR      0x02
#define LISP_SIZ_LDVR      sizeof(lisp_instr_t) + sizeof(lisp_cell_t)

#define LISP_ASM_PUSHD     0x03
#define LISP_SIZ_PUSHD     sizeof(lisp_instr_t) + sizeof(lisp_cell_t)

#define LISP_ASM_RET       0x10
#define LISP_SIZ_RET       sizeof(lisp_instr_t)

#define LISP_ASM_JP        0x11
#define LISP_SIZ_JP        sizeof(lisp_instr_t) + sizeof(lisp_size_t) + \
                           sizeof(lisp_lambda_t*)

#define LISP_ASM_HALT      0x12
#define LISP_SIZ_HALT      sizeof(lisp_instr_t)

#define LISP_ASM_BUILTIN   0x20
#define LISP_SIZ_BUILTIN   sizeof(lisp_instr_t) + sizeof(lisp_builtin_function_t)

#define LISP_INSTR_ARG(__INSTR__, __TYPE__)     \
  ((__TYPE__*)((__INSTR__) + 1))

#define LISP_INSTR_ARG_2(__INSTR__, __TYPE__, __TYPE2__)        \
  ((__TYPE2__*)(LISP_INSTR_ARG(__INSTR__,__TYPE__) + 1))

#define LISP_SET_INSTR(__TYPE_ID__, __INSTR__, __TYPE__, __VALUE__)  \
  {                                                                  \
    *(__INSTR__) = (__TYPE_ID__);                                    \
    *LISP_INSTR_ARG(__INSTR__, __TYPE__) = (__VALUE__);              \
  }

#define LISP_SET_INSTR_2(__TYPE_ID__, __INSTR__, __TYPE__, __VALUE__,   \
                         __TYPE2__, __VALUE2__)                         \
  {                                                                     \
    *(__INSTR__) = (__TYPE_ID__);                                       \
    *LISP_INSTR_ARG(__INSTR__, __TYPE__) = (__VALUE__);                 \
    *LISP_INSTR_ARG_2(__INSTR__,__TYPE__,__TYPE2__) = (__VALUE2__);     \
  }

static int _lisp_compile_phase1(lisp_vm_t         * vm,
                                lisp_cell_t       * cell,
                                lisp_size_t       * instr_size,
                                const lisp_cell_t * expr);

static int _lisp_compile_phase1_list_of_expressions(lisp_vm_t           * vm,
                                                    lisp_cell_t         * cell,
                                                    lisp_size_t         * instr_size,
                                                    const lisp_cell_t   * rest);

static int _lisp_compile_phase2(lisp_vm_t         * vm,
                                lisp_cell_t       * cell,
                                lisp_instr_t      * instr,
                                const lisp_cell_t * expr);

static int _lisp_compile_phase2_list_of_expressions(lisp_vm_t           * vm,
                                                    lisp_cell_t         * cell,
                                                    lisp_instr_t       ** instr,
                                                    lisp_size_t         * n_expr,
                                                    const lisp_cell_t   * rest);


static int lisp_compile_alloc(lisp_vm_t     * vm,
                              lisp_cell_t   * cell,
                              lisp_instr_t ** instr,
                              lisp_size_t     instr_size);


static int _lisp_compile_phase1(lisp_vm_t         * vm,
                                lisp_cell_t       * cell,
                                lisp_size_t       * instr_size,
                                const lisp_cell_t * expr)
{
  if(LISP_IS_NIL(expr)) 
  {
    /* before is_atom */
    /* @todo exception */
    return LISP_COMPILATION_ERROR;
  }
  else if(LISP_IS_ATOM(expr)) 
  {
    *instr_size+= LISP_SIZ_LDVD + LISP_SIZ_RET;
    return LISP_OK;
  }
  else if(LISP_IS_SYMBOL(expr))
  {
    *instr_size+= LISP_SIZ_LDVD + LISP_SIZ_RET;
    return LISP_OK;
  }
  else if(LISP_IS_OBJECT(expr))
  {
    *instr_size+= LISP_SIZ_LDVD + LISP_SIZ_RET;
    return LISP_OK;
  }
  else if(LISP_IS_CONS(expr)) 
  {
    if(LISP_IS_LAMBDA(LISP_CAR(expr)))
    {
      *instr_size+= LISP_SIZ_JP;
      return _lisp_compile_phase1_list_of_expressions(vm, cell, instr_size,
                                                      LISP_CDR(expr));
    }
    else 
    {
      /* @todo */
      return LISP_UNSUPPORTED;
    }
  }
  else 
  {
    return LISP_UNSUPPORTED;
  }
}

static int _lisp_compile_phase1_list_of_expressions(lisp_vm_t           * vm,
                                                    lisp_cell_t         * cell,
                                                    lisp_size_t         * instr_size,
                                                    const lisp_cell_t   * rest)
{
  while(!LISP_IS_NIL(rest)) 
  {
    if(LISP_IS_CONS(rest)) 
    {
      if(LISP_IS_ATOM(LISP_CAR(rest))) 
      {
        /* @todo implement */
        /* PUSHD D */
        *instr_size += LISP_SIZ_PUSHD;
      }
      else 
      {
        /* @todo implement */
      }
      rest = LISP_CDR(rest);
    }
    else
    {
      /* @todo error */
      return LISP_UNSUPPORTED;
    }
  }
  return LISP_OK;
}

static int _lisp_compile_phase2(lisp_vm_t         * vm,
                                lisp_cell_t       * cell,
                                lisp_instr_t      * instr,
                                const lisp_cell_t * expr)
{
  if(LISP_IS_ATOM(expr)) 
  {
    /* LDVD expr
       RET
    */
    LISP_SET_INSTR(LISP_ASM_LDVD, instr, lisp_cell_t, *expr);
    instr+= LISP_SIZ_LDVD;    
    *instr = LISP_ASM_RET;
    return LISP_OK;
  }
  else if(LISP_IS_SYMBOL(expr))
  {
    /* LDVR expr
       RET
    */
    LISP_SET_INSTR(LISP_ASM_LDVR, instr, lisp_cell_t, *expr);
    instr+= LISP_SIZ_LDVR;    

    *instr = LISP_ASM_RET;

    lisp_cell_t rest;
    lisp_make_cons(vm, &rest);
    lisp_copy_object(vm, LISP_CAR(&rest), expr);
    lisp_cons_set_car_cdr(vm,
                          LISP_AS(cell, lisp_cons_t),
                          NULL,
                          &rest);
    return LISP_OK;
  }
  else if(LISP_IS_OBJECT(expr))
  {
    /* LDVD expr
       RET
    */
    LISP_SET_INSTR(LISP_ASM_LDVD, instr, lisp_cell_t, *expr);
    instr+= LISP_SIZ_LDVD;    

    *instr = LISP_ASM_RET;
    instr++;

    lisp_cell_t rest;
    lisp_make_cons(vm, &rest);
    lisp_copy_object(vm, LISP_CAR(&rest), expr);
    lisp_cons_set_car_cdr(vm,
                          LISP_AS(cell, lisp_cons_t),
                          NULL,
                          &rest);
    return LISP_OK;
  }
  else if(LISP_IS_CONS(expr)) 
  {
    if(LISP_IS_LAMBDA(LISP_CAR(expr)))
    {
      lisp_size_t n;
      int ret = _lisp_compile_phase2_list_of_expressions(vm, cell, &instr, &n, LISP_CDR(expr));
      if(ret == LISP_OK) 
      {
        LISP_SET_INSTR_2(LISP_ASM_JP, instr,
                         lisp_size_t, n,
                         lisp_lambda_t*,
                         LISP_AS(LISP_CAR(expr), lisp_lambda_t))
        lisp_cell_t rest;
        lisp_make_cons(vm, &rest);
        lisp_copy_object(vm, LISP_CAR(&rest), LISP_CAR(expr));
        lisp_cons_set_car_cdr(vm,
                              LISP_AS(cell, lisp_cons_t),
                              NULL,
                              &rest);
 
        return LISP_OK;
      }
      else 
      {
        return ret;
      }
    }
    else 
    {
      /* @todo */
      return LISP_UNSUPPORTED;
    }
  }
  else 
  {
    return LISP_UNSUPPORTED;
  }
}

static int _lisp_compile_phase2_list_of_expressions(lisp_vm_t           * vm,
                                                    lisp_cell_t         * cell,
                                                    lisp_instr_t       ** instr,
                                                    lisp_size_t         * n_expr,
                                                    const lisp_cell_t   * rest)
{
  *n_expr = 0;
  while(!LISP_IS_NIL(rest)) 
  {
    if(LISP_IS_CONS(rest)) 
    {
      if(LISP_IS_ATOM(LISP_CAR(rest))) 
      {
        /* @todo implement */
        /* PUSHD D */
        LISP_SET_INSTR(LISP_ASM_PUSHD, *instr, lisp_cell_t, *LISP_CAR(rest));
        (*instr)+= LISP_SIZ_PUSHD;    
        (*n_expr)++;
      }
      else 
      {
        /* @todo implement */
      }
      rest = LISP_CDR(rest);
    }
    else
    {
      /* @todo error */
      return LISP_UNSUPPORTED;
    }
  }
  return LISP_OK;
}



static int lisp_compile_alloc(lisp_vm_t     * vm,
                              lisp_cell_t   * cell,
                              lisp_instr_t ** instr,
                              lisp_size_t     instr_size)
{
  /* @todo check allocation */
  lisp_byte_code_t * byte_code = MALLOC_OBJECT(sizeof(lisp_byte_code_t) + 
                                               instr_size,
                                               1);
  byte_code->instr_size = instr_size;
  *instr               = (lisp_instr_t*) &byte_code[1];
  lisp_make_cons_typed(vm, cell, LISP_TID_LAMBDA);
  LISP_CAR(cell)->type_id  = LISP_TID_OBJECT;
  LISP_CAR(cell)->data.ptr = byte_code;
  return LISP_OK;
}

int lisp_make_builtin_lambda(lisp_vm_t               * vm,
                             lisp_cell_t             * cell,
                             lisp_size_t               args_size,
                             const lisp_cell_t       * args,
                             lisp_builtin_function_t   func)
{
  /* @todo check allocation */
  /* @todo arguments */
  int ret;
  lisp_instr_t * instr;
  ret = lisp_compile_alloc(vm,
                           cell, 
                           &instr,
                           LISP_SIZ_BUILTIN + LISP_SIZ_RET + LISP_SIZ_HALT);
  if(ret == LISP_OK) 
  {
    LISP_SET_INSTR(LISP_ASM_BUILTIN, instr, lisp_builtin_function_t, func);
    instr+= LISP_SIZ_BUILTIN;    

    *instr = LISP_ASM_RET;
    instr++;

    /*@todo remove halt */
    *instr = LISP_ASM_HALT;
  }
  return LISP_OK;
}

int lisp_make_builtin_form(lisp_vm_t                * vm,
                           lisp_cell_t              * cell,
                           lisp_size_t                args_size,
                           const lisp_cell_t        * args,
                           lisp_size_t                data_size,
                           const lisp_cell_t        * data,
                           lisp_builtin_function_t    func)
{
  lisp_byte_code_t * lambda = MALLOC_OBJECT(sizeof(lisp_byte_code_t),
                                         1);
  cell->type_id  = LISP_TID_FORM;
  cell->data.ptr = lambda;
  lambda->instr_size = 0;
  return LISP_OK;
}

int lisp_make_builtin_c_str(lisp_vm_t                * vm,
			    lisp_cell_t              * cell,
			    lisp_size_t                args_size,
			    const char              ** args,
			    lisp_builtin_function_t    func)
{
  lisp_byte_code_t * lambda = MALLOC_OBJECT(sizeof(lisp_byte_code_t),
					 1);
  cell->type_id  = LISP_TID_FORM;
  cell->data.ptr = lambda;
  lambda->instr_size = 0;
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
                     lisp_size_t          nargs)
{
  /* @todo: define argument signature class
     @todo: eval instead of copy 
     @todo: use stack instead of rest 
     @todo: remove arguments from stack after calling function
     @todo: create function to match rest with function signature
  */
  lisp_size_t i;
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  env->n_values = 0;
  lisp_instr_t * instr = (lisp_instr_t*) &(LISP_AS(&lambda->car,
                                                   lisp_byte_code_t)[1]);
  lisp_cell_t  * cell;
  lisp_size_t    pc = 0;
  while(1) 
  {
    switch(*instr) 
    {
    case LISP_ASM_LDVD:
      env->n_values = 1;
      lisp_copy_object_as_root(env->vm,
                               env->values,
                               LISP_INSTR_ARG(instr, lisp_cell_t));
      instr+= LISP_SIZ_LDVD;
      break;
    case LISP_ASM_LDVR:
      env->n_values = 1;
      REQUIRE(LISP_IS_SYMBOL(LISP_INSTR_ARG(instr, lisp_cell_t)));
      cell = lisp_symbol_get(env->vm, 
                             LISP_AS(LISP_INSTR_ARG(instr, lisp_cell_t),
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
    case LISP_ASM_BUILTIN:
      /*@todo stack */
      (*LISP_INSTR_ARG(instr, lisp_builtin_function_t))(env, lambda, nargs);
      while(nargs)
      {
        lisp_unset_object_root(env->vm, &env->stack[--env->stack_top]);
        --nargs;
      }
      instr+= LISP_SIZ_BUILTIN;
      break;
    case LISP_ASM_RET:
      /* @todo implement */
      instr+= LISP_SIZ_RET;
      break;
    case LISP_ASM_JP:
      nargs = *LISP_INSTR_ARG(instr, lisp_size_t);
      lambda = *LISP_INSTR_ARG_2(instr, lisp_size_t, lisp_lambda_t*);
      instr = (lisp_instr_t*) &(LISP_AS(&lambda->car,
                                        lisp_byte_code_t)[1]);
      break;
    case LISP_ASM_PUSHD:
      /* @todo check range */
      lisp_copy_object_as_root(env->vm, &env->stack[env->stack_top], LISP_INSTR_ARG(instr, lisp_cell_t));
      env->stack_top++;
      instr+= LISP_SIZ_PUSHD;
      break;
    case LISP_ASM_HALT:
      return LISP_OK;
    default:
      return LISP_UNSUPPORTED;
    }
    pc++;
  }
  return LISP_UNSUPPORTED;
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

      /* @todo
       *  (let ((S E)
       *        (S E))
       *    E)
       *
       * pattern: 
       * (define expression-form? (expr)
       *                          (cond ((null? expr) #f)
       *                                ((list? expr) #t)
       *                                (#t #t)))
       *                              
       * (define let-form? (lambda (expr) #t))
       * (define let-compile (lambda (expr) #t))
       * 
       * (letrec* 
       *     ((let-pair? (lambda (expr) 
       *                   (if (and (pair? expr) 
       *                            (symbol? (car expr))
       *                            (pair? (cdr expr))
       *                            (null? (cddr expr)))
       *                       #t
       *                       #f)))
       *
       *      (symbol-dev? (lambda (expr)
       *                         (or (null? expr)
       *                             (and (pair? expr)
       *                                  (let-pair? (car expr))
       *                                  (symbol-dev? (cdr expr)))))))
       *    
       *     (lambda (expr create-closure create-body) 
       *                         (if (and (pair? expr)
       *                                  (eq? (car expr) (quote let))
       *                                  (pair? (cdr expr))
       *                                  (symbol-dev? (car (cdr expr)) create-closure)
       *                                  (not (null? (cddr expr))))
       *                             (begin (create-body (cddr expr)) #t) 
       *                             #f))
       *
       *
       *    
       */

int lisp_lambda_compile(lisp_eval_env_t   * env,
			lisp_cell_t       * cell,
			const lisp_cell_t * expr)
{
  *cell = lisp_nil;
  int                ret;
  lisp_size_t        instr_size = 0;
  lisp_byte_code_t * byte_code;
  ret = _lisp_compile_phase1(env->vm,
                             cell,
                             &instr_size,
                             expr);
  if(ret != LISP_OK) 
  {
    return ret;
  }
  /* @todo remove halt */

  /* @todo check ret */
  /* @todo check allocation */
  /* @todo remove halt */
  instr_size+= LISP_SIZ_HALT;
  byte_code = MALLOC_OBJECT(sizeof(lisp_byte_code_t) + 
                            instr_size,   
                            1);
  byte_code->instr_size = instr_size;
  lisp_make_cons_typed(env->vm, cell, LISP_TID_LAMBDA);
  LISP_CAR(cell)->type_id  = LISP_TID_OBJECT;
  LISP_CAR(cell)->data.ptr = byte_code;
  /*@todo check ret */
  ret = _lisp_compile_phase2(env->vm,
                             cell,
                             (lisp_instr_t*) &byte_code[1],
                             expr);
  if(ret != LISP_OK) 
  {
    return ret;
  }
  /* @todo remove halt */
  ((lisp_instr_t*) &byte_code[1])[instr_size-1] = LISP_ASM_HALT;
  return LISP_OK;
}



