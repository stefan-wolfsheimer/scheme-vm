#ifndef __LISP_SYMBOL__
#define __LISP_SYMBOL__
#include "core/lisp_type.h"

struct lisp_symbol_t;

typedef struct lisp_closure_t
{
  unsigned int            is_attached : 1;
  unsigned int : 0;
  struct lisp_closure_t * prev;
  struct lisp_closure_t * next;
  struct lisp_symbol_t  * symbol;
  lisp_cell_t             cell;
} lisp_closure_t;

typedef struct lisp_symbol_t 
{
  lisp_size_t      size;
  uint32_t         code;
  lisp_cell_t      binding;
  lisp_closure_t * first_closure;
  lisp_closure_t * last_closure;
} lisp_symbol_t;

int lisp_make_symbol(lisp_vm_t         * vm,
		     lisp_cell_t       * cell,
		     const lisp_char_t * cstr);

void lisp_init_closure(lisp_vm_t             * vm,
		       lisp_closure_t * closure);

void lisp_init_closure_append(lisp_vm_t      * vm,
			      lisp_closure_t * closure,
			      lisp_symbol_t  * symbol);

int lisp_symbol_release_closure(lisp_vm_t      * vm,
				lisp_closure_t * closure);

/** Get current closure of symbol. 
 *  If symbol does not have any closure add one 
 * @todo implement 
 */
lisp_closure_t * lisp_symbol_current_closure(lisp_vm_t     * vm,
					     lisp_symbol_t * symbol);


int lisp_symbol_set(lisp_vm_t         * vm,
		    lisp_symbol_t     * symbol,
		    const lisp_cell_t * obj);

/** 
 * get value of symbol */
lisp_cell_t * lisp_symbol_get(lisp_vm_t           * vm,
			      const lisp_symbol_t * symbol);

int lisp_symbol_unset(lisp_vm_t * vm,
		      lisp_symbol_t * symbol);


/** integration into lisp_vm_t */
void lisp_symbol_destruct(lisp_vm_t * vm, void * ptr);
int  lisp_symbol_construct(void        * target,
			   const void  * src,
			   size_t        size,
			   void        * user_data);
int lisp_symbol_hash_eq(const void * a, const void * b);

#endif
