#ifndef __LISP_VM_H__
#define __LISP_VM_H__

/**
   @file  lisp_vm.h
   @brief core virtual machine functions 

   @todo  Replace create function with "make" functions.
   
           Convention
           int make_XXX(lisp_vm_t * vm, lisp_cell_t * target, ...)
           int make_XXX_version2(lisp_vm_t * vm, lisp_cell_t * target, ...)
           int make_XXX_variant3(lisp_vm_t * vm, lisp_cell_t * target, ...)
           target points to an non-initialized object or 
           an atom.
           returns: 0 on success, error code otherwise
 */
#include <stdlib.h>
#include <stdarg.h>
#include "lisp_type.h"
#include "util/hash_table.h"

typedef struct lisp_call_t 
{
  lisp_instr_t    pc_next;
  lisp_instr_t    nargs;
  lisp_lambda_t * lambda;
} lisp_call_t;

typedef struct lisp_thread_t
{
  struct lisp_vm_t * vm;
  lisp_cell_t   * data_stack;
  lisp_size_t     data_stack_top;
  lisp_size_t     data_stack_size;
  lisp_call_t   * call_stack;
  lisp_size_t     call_stack_top;
  lisp_size_t     call_stack_size;
} lisp_thread_t;

typedef struct lisp_vm_t 
{
  /* @todo move to lisp_thread_t */
  lisp_cell_t   * data_stack;
  lisp_size_t     data_stack_top;
  lisp_size_t     data_stack_size;
  lisp_call_t   * call_stack;
  lisp_size_t     call_stack_top;
  lisp_size_t     call_stack_size;
  /* @todo */
  lisp_size_t     program_counter;
  /* @todo remove (use stack instead */
  lisp_cell_t     value;

  lisp_type_t   * types;
  lisp_size_t     types_size;

  hash_table_t    symbols;

  /* cons data and garbage collector */
  lisp_cons_t               ** cons_table;
  lisp_size_t                  cons_table_size;

  lisp_size_t                  black_cons_top;
  lisp_size_t                  grey_cons_begin;
  lisp_size_t                  grey_cons_top;
  lisp_size_t                  white_cons_top;


  lisp_root_cons_t           * root_cons_table;
  lisp_size_t                  root_cons_table_size;
  lisp_size_t                  root_cons_top;


  lisp_cons_t               ** cons_pages;
  lisp_size_t                  n_cons_pages;
  lisp_size_t                  cons_page_size;

} lisp_vm_t;

typedef struct lisp_vm_param_t
{
  size_t data_stack_size;
  size_t call_stack_size;
} lisp_vm_param_t;

extern lisp_vm_param_t lisp_vm_default_param;

lisp_vm_t * lisp_create_vm( lisp_vm_param_t * param);
void lisp_free_vm(   lisp_vm_t * vm);


/*****************************************************************************
 * 
 * Types
 * 
 *****************************************************************************/

/** Register a user defined type 
 *  @param vm the machine
 *  @param name name of the type
 *  @param destructor (optional) 
 *         custom destructor that is called when the reference count is 0.
 *         if NULL, the memory is released.
 *  @param new_type the type id of the registered type
 *  @return LISP_OK or LISP_TYPE_ERROR
 */
int lisp_register_object_type(lisp_vm_t * vm,
			      lisp_char_t      * name,
			      lisp_destructor_t  destructor,
			      lisp_type_id_t   * new_type);



/* @todo replace with lisp_eval_lambda */
int lisp_vm_run(    lisp_vm_t   * vm, 
		    lisp_lambda_t * lambda);



/* @todo implement copy as root object and test if conses 
         have been rooted
   target must be atom or uninitalized
*/
int lisp_copy_object( lisp_vm_t   * vm,
		      lisp_cell_t * target,
		      const lisp_cell_t * source);

int lisp_copy_object_as_root( lisp_vm_t   * vm,
			      lisp_cell_t * target,
			      const lisp_cell_t * source);

int lisp_copy_n_objects( lisp_vm_t   * vm,
			 lisp_cell_t * target,
			 const lisp_cell_t * source,
			 lisp_size_t   n);

int lisp_copy_n_objects_as_root( lisp_vm_t   * vm,
				 lisp_cell_t * target,
				 const lisp_cell_t * source,
				 lisp_size_t   n);

int lisp_unset_object(lisp_vm_t * vm, 
		      lisp_cell_t * target);

int lisp_unset_object_root(lisp_vm_t * vm,
			   lisp_cell_t * target);
		      


/** __CELL__ is pointer to uninitialized cell
 * @todo replace with lisp_copy_object */

#define LISP_INIT(__VM__, __CELL__, __SOURCE__)                         \
  {                                                                     \
    if(LISP_IS_OBJECT((__SOURCE__)))                                    \
      {                                                                 \
        ++LISP_REFCOUNT((__SOURCE__));                                  \
      }                                                                 \
    *(__CELL__) = *(__SOURCE__);                                        \
  }

/** __CELL__ is pointer to initialized cell
 */
#define LISP_SET(__VM__, __CELL__, __SOURCE__)                          \
  {                                                                     \
    if(LISP_IS_OBJECT(__CELL__))                                        \
      {                                                                 \
        if(! --LISP_REFCOUNT((__CELL__)))                               \
          {                                                             \
            if((__VM__)->types[(__CELL__)->type_id].destructor != NULL) \
              {                                                         \
                (__VM__)->types[(__CELL__)->type_id]                    \
                .destructor((__VM__), (__CELL__)->data.ptr);            \
              }                                                         \
            else                                                        \
              {                                                         \
                FREE_OBJECT((__CELL__)->data.ptr);                      \
              }                                                         \
          }                                                             \
    }                                                                   \
    LISP_INIT((__VM__), (__CELL__), (__SOURCE__))                       \
  }

#if 1
/* @todo replace with lisp_unset_object and lisp_unset_root_object */
#define LISP_UNSET(__VM__, __CELL__)                                    \
  {                                                                     \
    lisp_cell_t * __cell__ = (__CELL__);                                \
    if(LISP_IS_OBJECT(__cell__))                                        \
      {                                                                 \
        if(! --LISP_REFCOUNT(__cell__))                                 \
          {                                                             \
            if((__VM__)->types[__cell__->type_id].destructor != NULL)	\
	    {                                     			\
                (__VM__)->types[(__cell__)->type_id]                    \
                  .destructor((__VM__), __cell__->data.ptr);            \
              }                                                         \
            else                                                        \
              {                                                         \
                FREE_OBJECT(__cell__->data.ptr);                        \
              }                                                         \
          }                                                             \
      }                                                                 \
    *__cell__ = lisp_nil;                                               \
  }
#endif
/*****************************************************************************
 * 
 * stack operations
 * 
 *****************************************************************************/

/*****************************************************************************
 * 
 * lambda
 * 
 *****************************************************************************/
int lisp_make_lambda_instr(lisp_vm_t          * vm, 
                           lisp_cell_t        * cell, 
                           lisp_size_t          args_size,
                           lisp_cell_t        * data,
                           lisp_size_t          data_size,
                           const lisp_instr_t * instr);

int lisp_make_lambda_n_instr(lisp_vm_t          * vm, 
                             lisp_cell_t        * cell, 
                             lisp_size_t          args_size,
                             lisp_cell_t        * data,
                             lisp_size_t          data_size,
                             const lisp_instr_t * instr,
                             lisp_size_t          n_instr);

/* @todo replace with lisp_make_lambda */
int lisp_create_lambda_n_instr(lisp_vm_t          * vm, 
                               lisp_cell_t        * cell, 
                               lisp_size_t          args_size,
                               lisp_size_t          data_size,
                               lisp_size_t          instr_size,
                               const lisp_instr_t * instr);


/* @todo implement 
 * evaluates the lambda function and returns the number of values 
 * added to the stack.
 * @todo encode error in lisp_size_t 
 * effect remove nargs element from stack and 
 * pushes return value on the stack.
 */
int lisp_lambda_eval(lisp_vm_t           * thr,
                     const lisp_lambda_t * lambda,
                     lisp_size_t           nargs);

/*  @todo implement excecution machine
 *
 ***********************************************************
 * (lambda () 1)
 *
 * precondition: [&ret, 0 ]
 *
 * 1  PUSHD 1     ; [&ret, 0, 1]
 * 2  RET         ; [1] SHIFT(2+0), JUMP(&ret)
 *
 * optimized:
 * 1  SHIFT 3
 * 2  PUSHD 1
 *
 ***********************************************************
 * (lambda () (+ 1 2))
 *
 * precondition:  [&ret, 0 ]
 *
 * 1 PUSHD 1    ; [&ret, 0, 1 ]
 * 2 PUSHD 2    ; [&ret, 0, 1, 2 ]
 * 3 #+    2    ; [&ret, 0, 3 ]
 * 4 RET        ; [3], SHIFT(2+0), JUMP(&ret)
 *
 ***********************************************************
 * (lambda (x) (+ x 1))
 *
 * precondition: [&ret, y, 1]
 *                        |
 *                       arg
 * 1 PUSHA 0         ; [&ret, y, 1, y]
 * 2 PUSHD 1         ; [&ret, y, 1, y, 1]
 * 3 #+    2         ; [&ret, y, 1, y + 1 ]
 * 4 RET             ; [y+1], SHIFT(2+1), JUMP(&ret)
 *
 ***********************************************************
 * (lambda () (+ (+ 1 2) (+ 3 4)))
 *
 * precondition: [&ret, 0]
 *
 * 1 PUSHD 1          ; [&ret, 0, 1]
 * 2 PUSHD 2          ; [&ret, 0, 1,2]
 * 3 #+    2          ; [&ret, 0, 3]
 * 4 PUSHD 3          ; [&ret, 0, 3,3]
 * 5 PUSHD 4          ; [&ret, 0, 3,3,4]
 * 6 #+    2          ; [&ret, 0, 3,7]
 * 7 #+    2          ; [&ret, 0, 10]
 * 8 RET              ; [10], SHIFT(2+0), JUMP(&ret)
 *                                    |
 *                                 stack_top-1
 ***********************************************************
 * ( lambda () 
 *   ( (lambda (x y) (+ x y)) 2 3) )
 *
 * precondition: [&c, &a, 0]
 *
 * main: 1  PUSHR &4        ; [&ret, 0, &6]
 *       2  PUSHD 2         ; [&ret, 0, &6, 2]
 *       3  PUSHD 3         ; [&ret, 0, &6, 2, 3]
 *       4  PUSHN 2         ; [&ret, 0, &6, 2, 3, 2]  
 *       5  CALL  &l1       ; 
 *       6  RET             ; [5], SHIFT(2+0), JUMP(&ret)       
 * l1:   7  PUSHS 3         ; [&ret, 0, &6, 2, 3, 2, 2]
 *                          ; stack-top-3+1
 *       8  PUSHS 3         ; [&ret, 0, &6, 2, 3, 2, 2, 3]
         9  PUSHN 2         ; [&ret, 0, &6, 2, 3, 2, 2, 3, 2]
 *       10 #+              ; [&ret, 0, &6, 2, 3, 2, 5]
 *       11 RET             ; [&ret, 0, 5 ], SHIFT(2+2), JUMP(&6)
 *                                                   |
 *                                               stack_top-1
 *                         
 ********************************************************
 * (lambda (x) (let ((a 1)
 *                   (b 2)) 
 *             (+ b x a 10)))
 *
 * precondition: [&ret, y]
 *
 * 1 PUSHR &???           ; [&ret, y, &?? ]
 * 1 PUSHD 1              ; [&ret, y, &??, 1 ]
 * 2 PUSHD 2              ; [&ret, y, &??, 2 ]
 * 3 PUSHN 2              ; [&ret, y, &??, 2, 2]
 * 4 #let                 ; [&ret,
 *                             (#let,  2, &next))
 * 2  PUSHD(1)             ->  [y, 1 ]
 * 3  PUSHD(2)             ->  [y, 1, 2]
 * 4  PUSHC(#+, 4)         -> ((#null, 1, &stop),
 *                             (#let,  2, &next),
 *                             (#+,    4, &next))
 * 5  PUSHA(1)             ->  [y, 1, 2, 2]
 * 6  PUSHA(4)             ->  [y, 1, 2, 2, y]
 * 7  PUSHA(4)             ->  [y, 1, 2, 2, y, 1]
 * 8  PUSHD(1)             ->  [y, 1, 2, 2, y, 1, 10]
 * 9  EVAL                 ->  [y, 1, 2, 13+y]
 *                            ((#null, 1, &stop),
 *                             (#let,  2, &next),
 * 10 EVAL                 ->  [y, 13+y]
 *                            ((#null, 1, &stop))
 * 11 RET                  ->  [13+y]
 *                             pc:= &stop
 *                             
 ********************************************************
 * (get-value (v n) #instr)
 *
 * 
 * (lambda () (get-value (make-tuple 2 nil) 0)
 */

/* @todo make private */
#define LISP_LAMBDA_DATA(__LAMBDA__)                                    \
  ((lisp_cell_t*)( ((char*)(__LAMBDA__)) +                              \
                   sizeof(lisp_lambda_t) +                              \
                   (__LAMBDA__)->instr_size * sizeof(lisp_instr_t)))    \

/*****************************************************************************
 * 
 * string
 * 
 *****************************************************************************/
/* @todo rename to lisp_make_string */
int lisp_make_string(lisp_vm_t         * vm,
                     lisp_cell_t       * cell,
                     const lisp_char_t * cstr);

int lisp_sprintf(lisp_vm_t         * vm,
                 lisp_cell_t       * cell,
                 const lisp_char_t * cstr,
                 ...);

const char * lisp_c_string(const lisp_cell_t * cell);

lisp_size_t lisp_string_length(const lisp_string_t * str);


/*****************************************************************************
 * 
 * symbol
 * 
 *****************************************************************************/
int lisp_create_symbol(lisp_vm_t         * vm,
                       lisp_cell_t       * cell,
                       const lisp_char_t * cstr);

/* Todo: */
int lisp_create_symbol_lstring(lisp_vm_t         * vm,
                               lisp_cell_t       * cell,
                               lisp_cell_t       * lstr);

/* Todo: */
int lisp_create_symbol_sprintf(lisp_vm_t         * vm,
                               lisp_cell_t       * cell,
                               const lisp_char_t * fmt,
                               ...);
/* Todo: */
lisp_symbol_t * lisp_get_symbol(lisp_vm_t * vm,
                                const lisp_char_t * cstr);

/* Todo: */
lisp_symbol_t * lisp_get_symbol_lstring(lisp_vm_t * vm,
                                        const lisp_cell_t * lstr);

/* Todo: */
lisp_symbol_t * lisp_get_symbol_sprintf(lisp_vm_t * vm,
                                        const lisp_char_t * fmt,
                                        ...);

/*****************************************************************
 *
 * cons objects
 *
 *****************************************************************/
/* @todo ensure invariant: black/root conses don't have white children 
 * @param cell unitialized cell or atom 
 */
int lisp_make_cons(lisp_vm_t   * vm,
                   lisp_cell_t * cell);

/* @todo implement */
int lisp_make_cons_car_cdr(lisp_vm_t * vm,
                           lisp_cell_t * cell,
                           const lisp_cell_t * car, 
                           const lisp_cell_t * cdr);

int lisp_make_cons_typed(lisp_vm_t     * vm,
                         lisp_cell_t   * cell,
                         lisp_type_id_t  type_id);

/* @todo implement */
int lisp_make_cons_typed_car_cdr(lisp_vm_t     * vm,
                                 lisp_cell_t   * cell,
                                 lisp_type_id_t  type_id,
                                 const lisp_cell_t * car, 
                                 const lisp_cell_t * cdr);

                           


int lisp_make_cons_root(lisp_vm_t   * vm,
                        lisp_cell_t * cell);

/* @todo implement */
int lisp_make_cons_root_car_cdr(lisp_vm_t * vm,
                                lisp_cell_t * cell,
                                const lisp_cell_t * car, 
                                const lisp_cell_t * cdr);



int lisp_make_cons_root_typed(lisp_vm_t     * vm,
                              lisp_cell_t   * cell,
                              lisp_type_id_t  type_id);

/* @todo implement */
int lisp_make_cons_root_typed_car_cdr(lisp_vm_t     * vm,
                                      lisp_cell_t   * cell,
                                      lisp_type_id_t  type_id,
                                      const lisp_cell_t * car, 
                                      const lisp_cell_t * cdr);
                                      

/** 
 * Returns the number of reference of the cons 
 * object in the root set.
 * If cell is not a cons, return 0 by default.
 *
 * @param  vm virtual machine context 
 * @param  cell to be tested
 * @return reference count
 */
lisp_ref_count_t lisp_cons_root_refcount(lisp_vm_t         * vm,
                                         const lisp_cell_t * cell);

int lisp_cons_root(lisp_vm_t * vm, lisp_cons_t * cons);
int lisp_cons_unroot(lisp_vm_t * vm, lisp_cons_t * cons);

/* @todo: implement */
int lisp_cons_set_car();
/* @todo: implement */
int lisp_cons_set_cdr();

/*****************************************************************
 *
 * create basic objects
 *
 *****************************************************************/
#ifdef DEBUG
void * lisp_malloc_object( const char      * file,
                           int               line, 
                           size_t            size,
                           lisp_ref_count_t  rcount);

void lisp_free_object( const char    * file,
                       int             line,
                       void          * ptr);

#define MALLOC_OBJECT(SIZE, RCOUNT) lisp_malloc_object(__FILE__,        \
                                                       __LINE__,        \
                                                       (SIZE),          \
                                                       (RCOUNT))

#define FREE_OBJECT(PTR) lisp_free_object(__FILE__,__LINE__,(PTR))

#else

void * lisp_malloc_object( size_t           size,
                           lisp_ref_count_t rcount );

void lisp_free_object( void * ptr);

/** Create a managed object of size SIZE with reference count RCOUNT
 */
#define MALLOC_OBJECT(SIZE, RCOUNT) lisp_malloc_object((SIZE),(RCOUNT))

/** 
 * Free a managed object.
 * Should be handled by the garbage collector
 */
#define FREE_OBJECT(PTR) lisp_free_object((PTR))

#endif

#endif
