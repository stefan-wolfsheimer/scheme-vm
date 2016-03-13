#ifndef __LISP_TYPES_H__
#define __LISP_TYPES_H__
#include <stdlib.h>
#include <stdint.h>

typedef size_t         lisp_size_t;
typedef char           lisp_char_t;
typedef int            lisp_integer_t;
typedef size_t         lisp_instr_t;
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


/** builtin types */
typedef struct lisp_lambda_t 
{
  lisp_size_t args_size;
  lisp_size_t instr_size;
  lisp_size_t data_size;
} lisp_lambda_t;

typedef struct lisp_string_t 
{
  lisp_size_t begin;
  lisp_size_t end;
} lisp_string_t;

typedef struct lisp_symbol_t 
{
  lisp_size_t size;
  uint32_t    code;
} lisp_symbol_t;

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


typedef struct lisp_vector_t 
{
  lisp_cell_t * data;
  lisp_size_t   size;
  lisp_size_t   reserved;
} lisp_vector_t ;

/*typedef struct lisp_array_t
{
  lisp_size_t    size;
  lisp_size_t    size_alloc;
  lisp_type_id_t type_id;
} lisp_array_t; */

typedef struct lisp_hash_table_t
{
} lisp_hash_table_t;

struct lisp_vm_t;

typedef void(*lisp_destructor_t)(struct lisp_vm_t * vm, void * ptr);

/** meta type */
typedef struct lisp_type_t 
{
  lisp_type_id_t    type_id;
  lisp_cell_t       name;
  lisp_destructor_t destructor;
} lisp_type_t;

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
#define LISP_TID_ATOM_MASK    0x00
#define LISP_TID_OBJECT_MASK  0x80

#define LISP_TID_NIL          0x00
#define LISP_TID_INTEGER      0x01

#define LISP_TID_CONS_MASK    0x40 /* 0x40 ... 0x7f */
#define LISP_TID_CONS         0x40


#define LISP_TID_LAMBDA       0x80 /* 0x80 ... 0xbf (0x80 + 0x3f) */
#define LISP_TID_STRING       0x81
#define LISP_TID_SYMBOL       0x82
#define LISP_TID_PARSER       0x83

#define LISP_OBJECT_REFCOUNT(__OBJ__)             \
  (((lisp_ref_count_t*)(__OBJ__))[-1])
  
#define LISP_REFCOUNT(__CELL__)                   \
  LISP_OBJECT_REFCOUNT((__CELL__)->data.ptr)

#define LISP_IS_ATOM(__CELL__)                  \
  ((__CELL__)->type_id < 0x40)
  
#define LISP_IS_OBJECT(__CELL__)                \
  ((__CELL__)->type_id & 0x80)

#define LISP_IS_CONS_OBJECT(__CELL__)                  \
  ((__CELL__)->type_id & LISP_TID_CONS_MASK)

#define LISP_IS_NIL(__CELL__)                   \
  (!(__CELL__)->type_id)

#define LISP_IS_SYMBOL(__CELL__)                \
  ((__CELL__)->type_id == LISP_TID_SYMBOL)

#define LISP_IS_CONS(__CELL__)                  \
  ((__CELL__)->type_id == LISP_TID_CONS)

#define LISP_IS_LAMBDA(__CELL__)                  \
  ((__CELL__)->type_id == LISP_TID_LAMBDA)

#define LISP_AS(__CELL__, __TYPE__)             \
  ((__TYPE__ *)((__CELL__)->data.ptr))


/* @todo make it possible for CONS_OBJECT as well */
#define LISP_CAR(__CELL__)                      \
  ((__CELL__)->type_id == LISP_TID_CONS ? &(__CELL__)->data.cons->car : NULL)

#define LISP_CDR(__CELL__)                      \
  ((__CELL__)->type_id == LISP_TID_CONS ? &(__CELL__)->data.cons->cdr : NULL)

/* create an integer. 
 * precondition __CELL__: unintialized or atom */
#define LISP_MAKE_INTEGER(__CELL__, __VALUE__)  \
  { ((__CELL__))->type_id = LISP_TID_INTEGER;   \
    ((__CELL__))->data.integer = __VALUE__; }


/** Error codes */
#define LISP_OK          0x00
#define LISP_ALLOC_ERROR 0x01
#define LISP_TYPE_ERROR  0x02



#endif
