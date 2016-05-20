#include "lisp_vm.h"
#include "util/murmur_hash3.h"
#include <string.h>

int lisp_make_symbol(lisp_vm_t         * vm,
                       lisp_cell_t       * cell,
                       const lisp_char_t * cstr)
{
  /* @todo error handling */
  int inserted = 1;
  size_t len = strlen(cstr);
  uint32_t seed = 1;
  uint32_t code;
  lisp_ref_count_t * ref;
  murmur_hash3_x86_32 (  cstr,
                         len,
                         seed,
                         &code);
  ref = hash_table_find_or_insert_func(&vm->symbols,
                                       cstr,
                                       sizeof(lisp_symbol_t) +
                                       sizeof(lisp_ref_count_t) + 
                                       strlen(cstr)+1,
                                       code,
                                       vm->symbols.eq_function,
                                       &inserted);
  ref[0]++;
  cell->type_id  =  LISP_TID_SYMBOL;
  cell->data.ptr = &ref[1];
  ((lisp_symbol_t*)cell->data.ptr)->code = code;
  if(inserted) 
  {
    ((lisp_symbol_t*)cell->data.ptr)->binding = lisp_nil;
  }
  return LISP_OK;
}

int lisp_symbol_set(lisp_vm_t         * vm,
		     lisp_symbol_t     * symbol,
		     const lisp_cell_t * obj)
{
  if(LISP_IS_NIL(&symbol->binding))
  {
    lisp_make_cons_root_car_cdr(vm, &symbol->binding, obj, &lisp_nil);
  }
  else 
  {
    lisp_cons_set_car_cdr(vm, 
			  LISP_AS(&symbol->binding, lisp_cons_t),
			  obj,
			  NULL);
  }
  return LISP_OK;
}

lisp_cell_t * lisp_symbol_get(lisp_vm_t           * vm,
			      const lisp_symbol_t * symbol)
{
  if(LISP_IS_NIL(&symbol->binding))
  {
    return NULL;
  }
  else 
  {
    return &LISP_AS(&symbol->binding, lisp_cons_t)->car;
  }
}

int lisp_symbol_unset(lisp_vm_t * vm,
		      lisp_symbol_t * symbol)
{
  if(LISP_IS_NIL(&symbol->binding))
  {
    return LISP_UNDEFINED;

  }
  else 
  {
    lisp_cons_t * cons = LISP_AS(&symbol->binding, lisp_cons_t);
    lisp_unset_object(vm, &cons->car);
    if(LISP_IS_NIL(&cons->cdr))
    {
      lisp_unset_object_root(vm, &symbol->binding);
    }
    else 
    {
      lisp_cell_t tmp;
      lisp_copy_object_as_root(vm, &tmp, &cons->cdr);
      lisp_unset_object_root(vm, &symbol->binding);
      symbol->binding = tmp;
    }
    return LISP_OK;
  }
}

		   
