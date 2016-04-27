#include "lisp_type.h"
#include "lisp_vm.h"
#include "util/hash_table.h"
#include "util/xmalloc.h"
#include "util/murmur_hash3.h"
#include <string.h>

const lisp_cell_t lisp_nil = 
{
 type_id : LISP_TID_NIL
};

static int _lisp_register_object_type(lisp_vm_t         * vm,
				      lisp_char_t       * name,
				      lisp_destructor_t   destructor,
				      lisp_type_id_t      new_type_id);

static int _lisp_register_cons_type(lisp_vm_t         * vm,
				    lisp_char_t       * name,
				    lisp_type_id_t      new_type_id);

#if 0
static void _destruct_lambda(lisp_vm_t * vm,
                             void      * ptr);
#endif
static void _destruct_symbol(lisp_vm_t * vm, void * ptr);

static int _construct_symbol(void       * target,
                             const void * src,
                             size_t       size,
                             void       * user_data);

static int _symbol_eq(const void * a, const void * b);

/*****************************************************************************
 * 
 * types
 * 
 *****************************************************************************/
int _lisp_register_object_type(lisp_vm_t         * vm,
			      lisp_char_t       * name,
			      lisp_destructor_t   destructor,
			      lisp_type_id_t      new_type_id)
{
  if(new_type_id <  LISP_TID_OBJECT_MASK || 
     new_type_id > 0xff) 
  {
    return LISP_TYPE_ERROR;
  }
  if(vm->types[new_type_id].type_id != 0) 
  {
    return LISP_TYPE_ERROR;
  }
  else 
  {
    vm->types[new_type_id].type_id = new_type_id;
    vm->types[new_type_id].name = lisp_nil; /* @todo copy string */
    vm->types[new_type_id].destructor = destructor;
    return LISP_OK;
  }
}

static int _lisp_register_cons_type(lisp_vm_t         * vm,
				    lisp_char_t       * name,
				    lisp_type_id_t      new_type_id)
{
  if(new_type_id <  LISP_TID_CONS_MASK || 
     new_type_id > 0x7f) 
  {
    return LISP_TYPE_ERROR;
  }
  if(vm->types[new_type_id].type_id != 0) 
  {
    return LISP_TYPE_ERROR;
  }
  else 
  {
    vm->types[new_type_id].type_id = new_type_id;
    vm->types[new_type_id].name = lisp_nil; /* @todo copy string */
    vm->types[new_type_id].destructor = NULL;
    return LISP_OK;
  }
}

int lisp_register_object_type(lisp_vm_t         * vm,
			      lisp_char_t       * name,
			      lisp_destructor_t   destructor,
			      lisp_type_id_t    * new_type)
{
  size_t i;
  *new_type = 0;
  for(i = LISP_TID_OBJECT_MASK; i < 0x100; i++) 
  {
    if(vm->types[i].type_id == 0) 
    {
      *new_type = i;
      return _lisp_register_object_type(vm, name, destructor, i);
    }
  }
  return LISP_TYPE_ERROR;
}

int lisp_register_cons_type(lisp_vm_t          * vm,
			    lisp_char_t        * name,
			    lisp_type_id_t     * new_type)
{
  size_t i;
  *new_type = 0;
  for(i = LISP_TID_CONS_MASK; i < 0x80; i++) 
  {
    if(vm->types[i].type_id == 0) 
    {
      *new_type = i;
      return _lisp_register_cons_type(vm, name, i);
    }
  }
  return LISP_TYPE_ERROR;
}


int _lisp_init_types(lisp_vm_t * vm)
{
  int err = 0;
  /* @todo cleanup on failure (name is copied) */
  /* @todo move to lisp_type.c */
  err |= _lisp_register_object_type(vm, 
				    "SYMBOL", 
				    _destruct_symbol,
				    LISP_TID_SYMBOL);
  err |= _lisp_register_object_type(vm, 
				    "LAMBDA", 
				    _destruct_symbol,
				    LISP_TID_LAMBDA);
  err |= _lisp_register_cons_type(vm,
				  "CONS",
				  LISP_TID_CONS);
  if(err) 
  {
    return LISP_TYPE_ERROR;
  }
  /* init symbol table */
  if(hash_table_init(&vm->symbols,
		     _symbol_eq,
		     NULL, 
		     _construct_symbol,
		     NULL,  255))
  {
    return LISP_ALLOC_ERROR;
  }
  vm->symbols.user_data = vm;
  return LISP_OK;
}

/*****************************************************************************
 * 
 * symbols
 * 
 *****************************************************************************/
static void _destruct_symbol(lisp_vm_t * vm, void * ptr)
{
  if(LISP_IS_NIL(& ((lisp_symbol_t*)ptr)->binding))
  {
    hash_table_remove_func(&vm->symbols,
			   (char*)ptr + sizeof(lisp_symbol_t),
			   ((lisp_symbol_t*)ptr)->code,
			   vm->symbols.eq_function);
  }
}

static int _construct_symbol(void        * target,
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



static int _symbol_eq(const void * a, const void * b)
{
  return ! strcmp(
                  ( (const char*)a + 
                    sizeof(lisp_ref_count_t) + sizeof(lisp_symbol_t)),
                  (const char*)b);
}




/*****************************************************************************
 * 
 * lambda
 * 
 *****************************************************************************/
/* @todo move it to other c file */
#if 0
static void _destruct_lambda(lisp_vm_t * vm,
                             void      * ptr)
{
  lisp_cell_t * data = 
    (lisp_cell_t*)
    (((char*)ptr) + 
     sizeof(lisp_lambda_t) + 
     ((lisp_lambda_t*)ptr)->instr_size * sizeof(lisp_instr_t));
  lisp_size_t n = ((lisp_lambda_t*)ptr)->data_size;
  lisp_size_t i;
  for(i = 0; i < n; i++) 
  {
    lisp_unset_object(vm, &data[i]);
  }
  FREE_OBJECT(ptr);
}
#endif

/*****************************************************************************
 * 
 * integer
 * 
 *****************************************************************************/
void lisp_make_integer(lisp_cell_t * cell, lisp_integer_t value)
{
  cell->type_id = LISP_TID_INTEGER;
  cell->data.integer = value;
}
