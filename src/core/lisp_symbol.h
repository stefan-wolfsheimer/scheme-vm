#ifndef __LISP_SYMBOL__
#define __LISP_SYMBOL__
#include "core/lisp_type.h"

typedef struct lisp_symbol_t 
{
  lisp_size_t   size;
  uint32_t      code;
  lisp_cell_t   binding;
} lisp_symbol_t;

int lisp_make_symbol(lisp_vm_t         * vm,
		     lisp_cell_t       * cell,
		     const lisp_char_t * cstr);

int lisp_symbol_set(lisp_vm_t         * vm,
		    lisp_symbol_t     * symbol,
		    const lisp_cell_t * obj);

/** 
 * get value of symbol */
lisp_cell_t * lisp_symbol_get(lisp_vm_t           * vm,
			      const lisp_symbol_t * symbol);

int lisp_symbol_unset(lisp_vm_t * vm,
		      lisp_symbol_t * symbol);



void lisp_symbol_destruct(lisp_vm_t * vm, void * ptr);
int  lisp_symbol_construct(void        * target,
			   const void  * src,
			   size_t        size,
			   void        * user_data);
int lisp_symbol_hash_eq(const void * a, const void * b);





#endif
