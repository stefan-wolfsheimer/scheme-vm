#include "lisp_vm.h"
#include "util/murmur_hash3.h"
#include "util/assertion.h"
#include "core/lisp_symbol.h"

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
  ((lisp_symbol_t*)&ref[1])->code = code;
  if(inserted) 
  {
    ((lisp_symbol_t*)&ref[1])->first_closure = NULL;
    ((lisp_symbol_t*)&ref[1])->last_closure = NULL;
    /* @todo remove binding */
    ((lisp_symbol_t*)cell->data.ptr)->binding = lisp_nil;
  }
  return LISP_OK;
}

void lisp_init_closure(lisp_vm_t             * vm,
		       lisp_closure_t * closure)
{
  closure->is_attached = 0;
  closure->next = NULL;
  closure->prev = NULL;
  closure->symbol = NULL;
  /* @todo init binding */
}

void lisp_init_closure_append(lisp_vm_t      * vm,
			      lisp_closure_t * closure,
			      lisp_symbol_t  * symbol)
{
  closure->is_attached = 1;
  closure->next = NULL;
  closure->prev = symbol->last_closure;
  closure->symbol = symbol;
    /* @todo init binding */
  LISP_OBJECT_REFCOUNT(symbol)++;
  if(closure->prev) 
  {
    closure->prev->next =  closure;
    symbol->last_closure = closure;
  }
  else 
  {
    symbol->first_closure = closure;
    symbol->last_closure  = closure;
  }
}

int lisp_symbol_release_closure(lisp_vm_t      * vm,
				lisp_closure_t * closure)
{
  REQUIRE_NEQ_PTR(closure, NULL);
  REQUIRE_NEQ_PTR(closure->symbol, NULL);
  REQUIRE_GT_U(LISP_OBJECT_REFCOUNT(closure->symbol), 0u);
  if(closure->prev != NULL) 
  {
    closure->prev->next = closure->next;
  }
  else 
  {
    closure->symbol->first_closure = closure->next;
  }
  if(closure->next != NULL) 
  {
    closure->next->prev = closure->prev;
  }
  else 
  {
    closure->symbol->last_closure = closure->prev;
  }
  /* @todo direct removal instead of using lisp_unset_object 
           is more efficient */
  lisp_cell_t tmp;
  tmp.type_id = LISP_TID_SYMBOL;
  tmp.data.ptr = closure->symbol;
  lisp_unset_object(vm, &tmp);

  closure->is_attached = 0;
  closure->symbol = NULL;
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

/*****************************************************************************
 * 
 * symbols
 * 
 *****************************************************************************/
void lisp_symbol_destruct(lisp_vm_t * vm, void * ptr)
{
  if(LISP_IS_NIL(& ((lisp_symbol_t*)ptr)->binding))
  {
    hash_table_remove_func(&vm->symbols,
			   (char*)ptr + sizeof(lisp_symbol_t),
			   ((lisp_symbol_t*)ptr)->code,
			   vm->symbols.eq_function);
  }
}

int lisp_symbol_construct(void        * target,
			  const void  * src,
			  size_t        size,
			  void        * user_data)
{
  ((lisp_ref_count_t*) target)[0] = 0;
  lisp_symbol_t * symbol = (lisp_symbol_t*) 
    (((char*) target)  + sizeof(lisp_ref_count_t));
  symbol->size = size - sizeof(lisp_ref_count_t) - sizeof(lisp_symbol_t) - 1;
  strncpy( (char*) &symbol[1], (const char*)src, symbol->size);
  ((char*) &symbol[1])[symbol->size] = '\0';
  return 0;
}

int lisp_symbol_hash_eq(const void * a, const void * b)
{
  return ! strcmp(
                  ( (const char*)a + 
                    sizeof(lisp_ref_count_t) + sizeof(lisp_symbol_t)),
                  (const char*)b);
}



		   
