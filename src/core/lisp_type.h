#ifndef __LISP_TYPES_H__
#define __LISP_TYPES_H__
#include <stdlib.h>
#include <stdint.h>

typedef size_t         lisp_size_t;
typedef char           lisp_char_t;
typedef int            lisp_integer_t;
typedef unsigned short lisp_type_id_t;
typedef size_t         lisp_ref_count_t;

struct lisp_cons_t;

typedef struct lisp_cell_t 
{
  lisp_type_id_t type_id;
  union 
  {
    void               * ptr;
    struct lisp_cons_t * cons;
    lisp_integer_t       integer;
  } data;
} lisp_cell_t;

typedef struct lisp_string_t 
{
  lisp_size_t   begin;
  lisp_size_t   end;
  lisp_char_t * data;
} lisp_string_t;


typedef struct lisp_cons_t 
{
  lisp_size_t is_root : 1;
  lisp_size_t gc_cons_index : sizeof(lisp_size_t)*8 - 1;
  lisp_cell_t car;
  lisp_cell_t cdr;
} lisp_cons_t;

typedef struct lisp_root_cons_t
{
  lisp_cons_t      * cons;
  lisp_ref_count_t   ref_count;
} lisp_root_cons_t;

struct lisp_vm_t;

typedef struct lisp_eval_env_t
{
  struct lisp_vm_t   * vm;
  lisp_cell_t * values;
  lisp_size_t   n_values;
  lisp_size_t   max_values;
  lisp_cell_t * stack;
  lisp_size_t   stack_top;
  lisp_size_t   stack_size;
} lisp_eval_env_t;

typedef void(*lisp_destructor_t)(struct lisp_vm_t * vm, void * ptr);

/** Call back for builtin functions. 
 *  The result is written to value register of env
 */
typedef int (*lisp_builtin_function_t)(struct lisp_eval_env_t * env,
				       lisp_cell_t            * stack);

/** meta type */
typedef struct lisp_type_t 
{
  lisp_type_id_t    type_id;
  lisp_cell_t       name;
  lisp_destructor_t destructor;
} lisp_type_t;

typedef struct lisp_lambda_t
{
  lisp_size_t instr_size;
  lisp_size_t data_size;
  lisp_builtin_function_t  func;
} lisp_lambda_t;

/* cons -> car, cdr */
/* array -> a1, a2, ..., an */
/* hash -> a1, a2, ..., an */
/* list -> a1, a2, ..., an */
/* lambda -> a1, a2, ..., an */
/* create object: 1. alloc
                  2. init
                  3. add object and children to black list

*/


extern const lisp_cell_t lisp_nil;
/*      7   6  5  4 3 2 1 0
 *    128  64 32 16 8 4 2 1
 *     80  40 20 10 8 4 2 1
*/
#define LISP_TID_ATOM_MASK      0x00
#define LISP_TID_OBJECT_MASK    0x80

#define LISP_TID_NIL            0x00
#define LISP_TID_INTEGER        0x01
#define LISP_TID_FDEFINE        0x30

#define LISP_TID_CONS_MASK      0x40 /* 0x40 ... 0x7f */
#define LISP_TID_CONS           0x40
#define LISP_TID_EVAL_ERROR     0x41

#define LISP_TID_LAMBDA         0x80 /* 0x80 ... 0xbf (0x80 + 0x3f) */
#define LISP_TID_FORM           0x81
#define LISP_TID_STRING         0x82
#define LISP_TID_SYMBOL         0x83
#define LISP_TID_PARSER         0x84

#define LISP_OBJECT_REFCOUNT(__OBJ__)             \
  (((lisp_ref_count_t*)(__OBJ__))[-1])
  
#define LISP_REFCOUNT(__CELL__)                   \
  LISP_OBJECT_REFCOUNT((__CELL__)->data.ptr)

#define LISP_IS_ATOM(__CELL__)                  \
  ((__CELL__)->type_id < 0x40)
  
#define LISP_IS_OBJECT(__CELL__)		\
  ((__CELL__)->type_id & 0x80)

#define LISP_IS_CONS_OBJECT(__CELL__)		\
  ((__CELL__)->type_id & LISP_TID_CONS_MASK)

#define LISP_IS_NIL(__CELL__)                   \
  (!(__CELL__)->type_id)

#define LISP_IS_SYMBOL(__CELL__)                \
  ((__CELL__)->type_id == LISP_TID_SYMBOL)

#define LISP_IS_CONS(__CELL__)                  \
  ((__CELL__)->type_id == LISP_TID_CONS)

#define LISP_IS_EVAL_ERROR(__CELL__)		\
  ((__CELL__)->type_id == LISP_TID_EVAL_ERROR)

#define LISP_IS_LAMBDA(__CELL__)		\
  ((__CELL__)->type_id == LISP_TID_LAMBDA)

#define LISP_IS_FORM(__CELL__)                  \
  ((__CELL__)->type_id == LISP_TID_FORM)

#define LISP_IS_FDEFINE(__CELL__)		\
  ((__CELL__)->type_id == LISP_TID_FDEFINE)

#define LISP_IS_INTEGER(__CELL__)		\
  ((__CELL__)->type_id == LISP_TID_INTEGER)

#define LISP_IS_STRING(__CELL__)		\
  ((__CELL__)->type_id == LISP_TID_STRING)

#define LISP_IS_LIST(__CELL__)					\
  (LISP_IS_NIL((__CELL__)) || LISP_IS_CONS_OBJECT((__CELL__)))

#define LISP_AS(__CELL__, __TYPE__)             \
  ((__TYPE__ *)((__CELL__)->data.ptr))

/* @todo check if this macro can be used consitently */
#define LISP_CAR(__CELL__)			\
  (&((__CELL__)->data.cons->car))

#define LISP_CDR(__CELL__)			\
  (&((__CELL__)->data.cons->cdr))

/** Error codes */
#define LISP_OK          0x00
#define LISP_ALLOC_ERROR 0x01
#define LISP_TYPE_ERROR  0x02
#define LISP_EVAL_ERROR  0x03
#define LISP_UNSUPPORTED 0x04
#define LISP_UNDEFINED   0x05
#define LISP_RANGE_ERROR 0x06
#define LISP_COMPILATION_ERROR 0x07

#endif
