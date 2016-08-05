#include "lisp_lambda.h"
#include "lisp_vm.h"
#include "util/xmalloc.h"
#include "core/lisp_eval.h"
#include "core/lisp_symbol.h"
#include "core/lisp_exception.h"
#include "core/lisp_asm.h"
#include <string.h>

static int lisp_compile_alloc(lisp_vm_t     * vm,
                              lisp_cell_t   * cell,
                              lisp_instr_t ** instr,
                              lisp_size_t     instr_size);


static int _lisp_compile_phase1(lisp_vm_t         * vm,
                                lisp_size_t       * instr_size,
                                const lisp_cell_t * expr);

static int _lisp_compile_phase1_list_of_expressions(lisp_vm_t           * vm,
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

static int _lisp_compile_prepend_data(lisp_vm_t         * vm,
                                      lisp_cell_t * cell,
                                      const lisp_cell_t * obj);

/****************************************************************************/
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

/****************************************************************************/
static int _lisp_compile_phase1(lisp_vm_t         * vm,
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
      return _lisp_compile_phase1_list_of_expressions(vm, instr_size,
                                                      LISP_CDR(expr));
    }
    else if(LISP_IS_SYMBOL(LISP_CAR(expr)))
    {
      lisp_cell_t * value = lisp_symbol_get(vm, LISP_AS(LISP_CAR(expr), lisp_symbol_t));
      if(value && LISP_IS_FORM(value)) 
      {
        /* @todo */
        
      }
      else 
      {
        /* @todo */
      }
      return LISP_UNSUPPORTED;
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

/****************************************************************************/
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
    return _lisp_compile_prepend_data(vm, cell, expr);
  }
  else if(LISP_IS_OBJECT(expr))
  {
    /* LDVD expr
       RET
    */
    LISP_SET_INSTR(LISP_ASM_LDVD, instr, lisp_cell_t, *expr);
    instr+= LISP_SIZ_LDVD;    
    *instr = LISP_ASM_RET;
    return _lisp_compile_prepend_data(vm, cell, expr);
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
                         LISP_AS(LISP_CAR(expr), lisp_lambda_t));
        return _lisp_compile_prepend_data(vm, cell, expr);
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

static int _lisp_compile_prepend_data(lisp_vm_t         * vm,
                                      lisp_cell_t * cell,
                                      const lisp_cell_t * obj) 
{
  /* ( instr . ( arg_list . data_list)) -> 
     ( instr . ( arg_list . (obj . data_list)))
   */
  lisp_cell_t rest;
  lisp_make_cons_car_cdr(vm, &rest, obj, LISP_CDDR(cell));
  lisp_cons_set_car_cdr(vm,
                        LISP_AS(LISP_CDR(cell), lisp_cons_t),
                        NULL,
                        &rest);
  return LISP_OK;
}

/****************************************************************************/
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
                           LISP_SIZ_BUILTIN + LISP_SIZ_RET);
  if(ret == LISP_OK) 
  {
    LISP_SET_INSTR(LISP_ASM_BUILTIN, instr, lisp_builtin_function_t, func);
    instr+= LISP_SIZ_BUILTIN;    
    *instr = LISP_ASM_RET;
  }
  return LISP_OK;
}

int lisp_make_builtin_form(struct lisp_vm_t       * vm,
                           lisp_cell_t            * cell,
                           lisp_compile_phase1_t    phase1,
                           lisp_compile_phase2_t    phase2)
{
  lisp_form_t * form = MALLOC_OBJECT(sizeof(lisp_form_t), 1);
  cell->type_id  = LISP_TID_FORM;
  cell->data.ptr = form;
  form->phase1 = phase1;
  form->phase2 = phase2;
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
  lisp_size_t    i;
  int            ret;
  lisp_instr_t * instr;
  lisp_cell_t  * cell;
  lisp_size_t    pc = 0;

  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  env->n_values = 0;
  ret = lisp_push_halt(env);
  if(ret != LISP_OK) 
  {
    return ret;
  }
  instr = (lisp_instr_t*) &(LISP_AS(&lambda->car,
                                    lisp_byte_code_t)[1]);
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
      ret = (*LISP_INSTR_ARG(instr, lisp_builtin_function_t))(env, lambda, nargs);
      while(nargs)
      {
        lisp_unset_object_root(env->vm, &env->stack[--env->stack_top]);
        --nargs;
      }
      instr+= LISP_SIZ_BUILTIN;
      return ret;
      break;
    case LISP_ASM_RET:
      REQUIRE_GT_U(env->call_stack_top, 0u);
      env->call_stack_top--;
      instr = env->call_stack[env->call_stack_top].next_instr;
      lambda = env->call_stack[env->call_stack_top].lambda;
      break;
    case LISP_ASM_JP:
      nargs = *LISP_INSTR_ARG(instr, lisp_size_t);
      lambda = *LISP_INSTR_ARG_2(instr, lisp_size_t, lisp_lambda_t*);
      instr = (lisp_instr_t*) &(LISP_AS(&lambda->car,
                                        lisp_byte_code_t)[1]);
      break;
    case LISP_ASM_PUSHD:
      /* @todo check result and make push more efficient */
      lisp_push(env, LISP_INSTR_ARG(instr, lisp_cell_t));
      //lisp_copy_object_as_root(env->vm, &env->stack[env->stack_top], LISP_INSTR_ARG(instr, lisp_cell_t));
      //env->stack_top++;
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
                             &instr_size,
                             expr);
  if(ret != LISP_OK) 
  {
    return ret;
  }
  /* @todo check ret */
  /* @todo check allocation */
  /* @todo remove halt */
  byte_code = MALLOC_OBJECT(sizeof(lisp_byte_code_t) + 
                            instr_size,   
                            1);
  byte_code->instr_size = instr_size;
  lisp_make_cons_typed(env->vm, cell, LISP_TID_LAMBDA);
  LISP_CAR(cell)->type_id  = LISP_TID_OBJECT;
  LISP_CAR(cell)->data.ptr = byte_code;
  lisp_make_cons_car_cdr(env->vm, LISP_CDR(cell), &lisp_nil, &lisp_nil);
  /*@todo check ret */
  ret = _lisp_compile_phase2(env->vm,
                             cell,
                             (lisp_instr_t*) &byte_code[1],
                             expr);
  if(ret != LISP_OK) 
  {
    return ret;
  }
  return LISP_OK;
}

static int _disass_instr(lisp_vm_t   * vm,
                         lisp_cons_t * cons,
                         const char  * str)
{
  lisp_cell_t symb;
  lisp_make_symbol(vm, &symb, str);
  lisp_cons_set_car_cdr(vm, cons, &symb, NULL);
  return LISP_OK;
}

static int _disass_instr1(lisp_vm_t         * vm,
                          lisp_cons_t       * cons,
                          const lisp_cell_t * data,
                          const char        * str)
{
  lisp_cell_t symb;
  lisp_make_symbol(vm, &symb, str);
  lisp_cons_set_car_cdr(vm, cons, &symb, NULL);
  return LISP_OK;
}

int lisp_lambda_disassemble(lisp_vm_t     * vm,
                            lisp_cell_t   * cell,
                            lisp_lambda_t * lambda)
{
  lisp_cons_t      * last      = NULL;
  lisp_byte_code_t * byte_code = lambda->car.data.ptr;
  lisp_instr_t     * instr     = (lisp_instr_t*) &byte_code[1];
  lisp_instr_t     * end_instr = instr + byte_code->instr_size;
  lisp_cell_t        tmp;
  while(instr < end_instr) 
  {
    if(last == NULL) 
    {
      lisp_make_cons_car_cdr(vm, cell, &lisp_nil, &lisp_nil);
      last = LISP_AS(cell, lisp_cons_t);
    }
    else 
    {
      lisp_cell_t tmp;
      lisp_make_cons_car_cdr(vm, &tmp, &lisp_nil, &lisp_nil);
      lisp_cons_set_car_cdr(vm, last, NULL, &tmp);
      last = LISP_AS(&tmp, lisp_cons_t);
    }
    switch(*instr) 
    {
    case LISP_ASM_LDVD:
      _disass_instr1(vm,
                     last,
                     LISP_INSTR_ARG(instr, lisp_cell_t),
                     "LDVD");
      instr+= LISP_SIZ_LDVD;
      break;
    case LISP_ASM_LDVR:
      _disass_instr1(vm,
                     last,
                     LISP_INSTR_ARG(instr, lisp_cell_t),
                     "LDVR");
      instr+= LISP_SIZ_LDVR;
      break;
    case LISP_ASM_BUILTIN:
      _disass_instr(vm,
                    last,
                    "BUILTIN");
      
      instr+= LISP_SIZ_BUILTIN;
      break;
    case LISP_ASM_RET:
      _disass_instr(vm,
                    last,
                    "RET");
      instr+= LISP_SIZ_RET;
      break;
    case LISP_ASM_JP:
      lisp_make_integer(&tmp, *LISP_INSTR_ARG(instr, lisp_size_t));
      _disass_instr1(vm,
                    last,
                     &tmp,
                    "JP");
      instr+= LISP_SIZ_JP;
      break;
    case LISP_ASM_PUSHD:
      _disass_instr1(vm,
                     last,
                     LISP_INSTR_ARG(instr, lisp_cell_t),
                     "PUSHD");
      instr+= LISP_SIZ_PUSHD;
      break;
    case LISP_ASM_HALT:
      _disass_instr(vm,
                    last,
                    "PUSHD");
      instr+= LISP_SIZ_HALT;
      break;
    default:
      return LISP_UNSUPPORTED;
    }
  }
  return LISP_OK;
}


