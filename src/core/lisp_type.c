#include "lisp_type.h"
#include "lisp_vm.h"
#include "util/hash_table.h"
#include "util/xmalloc.h"
#include "util/murmur_hash3.h"
#include "util/assertion.h"
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"
#include "core/lisp_exception.h"
#include <string.h>

const lisp_cell_t lisp_nil = 
{
 type_id : LISP_TID_NIL
};

static int _lisp_register_object_type(lisp_vm_t         * vm,
                                      lisp_char_t       * name,
                                      lisp_destructor_t   destructor,
                                      lisp_printer_t      printer,
                                      lisp_type_id_t      new_type_id);

static int _lisp_register_cons_type(lisp_vm_t         * vm,
                                    lisp_char_t       * name,
                                    lisp_type_id_t      new_type_id);

static void _destruct_string(lisp_vm_t * vm, void * ptr);

/*****************************************************************************
 * 
 * types
 * 
 *****************************************************************************/
int _lisp_register_object_type(lisp_vm_t         * vm,
                               lisp_char_t       * name,
                               lisp_destructor_t   destructor,
                               lisp_printer_t      printer,
                               lisp_type_id_t      new_type_id)
{
  REQUIRE_GE_I(new_type_id, LISP_TID_OBJECT_MASK);
  REQUIRE_LE_I(new_type_id, 0xff);
  REQUIRE_EQ_I(vm->types[new_type_id].type_id, 0);
  vm->types[new_type_id].type_id = new_type_id;
  vm->types[new_type_id].name = lisp_nil; /* @todo copy string */
  vm->types[new_type_id].destructor = destructor;
  vm->types[new_type_id].printer = printer;
  return LISP_OK;
}

static int _lisp_register_cons_type(lisp_vm_t         * vm,
                                    lisp_char_t       * name,
                                    lisp_type_id_t      new_type_id)
{
  REQUIRE_GE_I(new_type_id, LISP_TID_CONS_MASK);
  REQUIRE_LE_I(new_type_id, 0x7f);
  REQUIRE_EQ_I(vm->types[new_type_id].type_id, 0);
  vm->types[new_type_id].type_id = new_type_id;
  vm->types[new_type_id].name = lisp_nil; /* @todo copy string */
  vm->types[new_type_id].destructor = NULL;
  return LISP_OK;
}

int lisp_register_object_type(lisp_vm_t         * vm,
                              lisp_char_t       * name,
                              lisp_destructor_t   destructor,
                              lisp_printer_t      printer,
                              lisp_type_id_t    * new_type)
{
  size_t i;
  *new_type = 0;
  for(i = LISP_TID_OBJECT_MASK; i < 0x100; i++) 
  {
    if(vm->types[i].type_id == 0) 
    {
      *new_type = i;
      return _lisp_register_object_type(vm, name, destructor, printer, i);
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
  err |= _lisp_register_object_type(vm, 
                                    "OBJECT",
                                    NULL,
                                    NULL,
                                    LISP_TID_OBJECT);

  err |= _lisp_register_object_type(vm,
                                    "EXCEPTION",
                                    lisp_exception_destruct,
                                    NULL,
                                    LISP_TID_EXCEPTION);
  err |= _lisp_register_object_type(vm, 
                                    "SYMBOL", 
                                    lisp_symbol_destruct,
                                    lisp_symbol_print,
                                    LISP_TID_SYMBOL);

  err |= _lisp_register_object_type(vm,
                                    "STRING",
                                    _destruct_string,
                                    NULL,
                                    LISP_TID_STRING);

  err |= _lisp_register_object_type(vm,
                                    "FORM",
                                    NULL,
                                    NULL,
                                    LISP_TID_FORM);

  err |= _lisp_register_cons_type(vm,
                                  "CONS",
                                  LISP_TID_CONS);

  err |= _lisp_register_cons_type(vm,
                                  "LAMBDA",
                                  LISP_TID_LAMBDA);

  if(err) 
  {
    return LISP_TYPE_ERROR;
  }
  /* init symbol table */
  if(hash_table_init(&vm->symbols,
                     lisp_symbol_hash_eq,
                     NULL, 
                     lisp_symbol_construct,
                     NULL,  255))
  {
    return LISP_ALLOC_ERROR;
  }
  vm->symbols.user_data = vm;
  return LISP_OK;
}



/*****************************************************************************
 * 
 * string
 * 
 *****************************************************************************/
static void _destruct_string(lisp_vm_t * vm, void * ptr)
{
  if(!-- ((lisp_ref_count_t*)((lisp_string_t*)ptr)->data)[-1]) 
  {
    FREE_OBJECT( ((lisp_string_t*)ptr)->data);
  }
  FREE_OBJECT(ptr);
}

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

const char * lisp_error_message(int code)
{
  switch(code)
  {
  case LISP_OK: return "OK";
  case LISP_ALLOC_ERROR: return "ALLOC_ERROR";
  case LISP_TYPE_ERROR: return "TYPE_ERROR";
  case LISP_EVAL_ERROR: return "EVAL_ERROR";
  case LISP_UNSUPPORTED: return "UNSUPPORTED";
  case LISP_RANGE_ERROR: return "RANGE_ERROR";
  case LISP_COMPILATION_ERROR: return "COMPILATION_ERROR";
  case LISP_STACK_OVERFLOW: return "STACK_OVERFLOW";
  default: return "???";
  }
}
