#include "lisp_vm.h"
#include "lisp_asm.h"
#include "util/xmalloc.h"
#include "util/murmur_hash3.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define LISP_CONS_PAGE_SIZE 1024

lisp_vm_param_t lisp_vm_default_param = 
{
  1024, 1024 
};

static void lisp_init_cons_gc(lisp_vm_t * vm);
static void lisp_free_cons_gc(lisp_vm_t * vm);
static void _lisp_create_vm_cleanup(lisp_vm_t * vm);

/* defined in lisp_type.c */
int _lisp_init_types(lisp_vm_t * vm);

/* cleanup on failure */
static void _lisp_create_vm_cleanup(lisp_vm_t * vm)
{
  if(vm->data_stack != NULL) FREE(vm->data_stack);
  if(vm->call_stack != NULL) FREE(vm->call_stack);
  if(vm->types      != NULL) FREE(vm->types);
  FREE(vm);
}

lisp_vm_t * lisp_create_vm(lisp_vm_param_t * param)
{
  lisp_vm_t * ret;
  ret = MALLOC(sizeof(lisp_vm_t));
  if(ret == NULL) 
  {
    return NULL;
  }
  /* @todo create continuations with own stack each */
  ret->data_stack      = NULL;
  ret->data_stack_size = param->data_stack_size;
  ret->data_stack_top  = 0;
  ret->data_stack      = MALLOC(sizeof(lisp_cell_t) * param->data_stack_size);
  ret->call_stack      = NULL;
  ret->call_stack_size = param->call_stack_size;
  ret->call_stack      = MALLOC(sizeof(lisp_call_t) * param->call_stack_size);
  ret->call_stack_top  = 0;

  /* @todo remove this, last value is pushed onto stack */
  ret->value = lisp_nil;

  /* init type system */
  ret->types      = NULL;
  ret->types_size = 255;
  ret->types      = MALLOC(sizeof(lisp_type_t) * ret->types_size);
  if(ret->call_stack == NULL || 
     ret->data_stack == NULL || 
     ret->types      == NULL) 
  {
    _lisp_create_vm_cleanup(ret);
    return NULL;
  }
  /* init conses */
  lisp_init_cons_gc(ret);

  lisp_size_t i;
  for(i = 0; i < ret->types_size; i++) 
  {
    ret->types[i].type_id = 0;
    ret->types[i].destructor = NULL;
    ret->types[i].name = lisp_nil;
  }
  /* types and symbol table */
  if(_lisp_init_types(ret)) 
  {
    _lisp_create_vm_cleanup(ret);
    return NULL;
  }
  return ret;
}

void lisp_free_vm(lisp_vm_t * vm)
{
  FREE(vm->data_stack);
  FREE(vm->call_stack);
  hash_table_finalize(&vm->symbols);
  lisp_free_cons_gc(vm);
  FREE(vm->types);
  FREE(vm);
}

void lisp_init_cons_gc(lisp_vm_t * vm)
{
  /* init cons pages */
  vm->cons_page_size       = LISP_CONS_PAGE_SIZE;
  vm->cons_pages           = NULL;
  vm->n_cons_pages         = 0;

  vm->cons_table           = NULL;
  vm->cons_table_size      = 0;
  vm->black_cons_top       = 0;
  vm->grey_cons_begin      = 0;
  vm->grey_cons_top        = 0;
  vm->white_cons_top       = 0;
 

  vm->root_cons_table      = NULL;
  vm->root_cons_table_size = 0;
  vm->root_cons_top        = 0;
}

void lisp_free_cons_gc(lisp_vm_t * vm)
{
  size_t i;
  if(vm->cons_table)
  {
    for(i = 0; i < vm->black_cons_top; i++) 
    {
      lisp_unset_object(vm, &vm->cons_table[i]->car);
      lisp_unset_object(vm, &vm->cons_table[i]->cdr);
    }
    for(i = vm->grey_cons_begin; i < vm->white_cons_top; i++) 
    {
      lisp_unset_object(vm, &vm->cons_table[i]->car);
      lisp_unset_object(vm, &vm->cons_table[i]->cdr);
    }
    FREE(vm->cons_table);
  }
  if(vm->root_cons_table)
  {
    for(i = 0; i < vm->root_cons_top; i++) 
    {
      lisp_unset_object(vm, &vm->root_cons_table[i].cons->car);
      lisp_unset_object(vm, &vm->root_cons_table[i].cons->cdr);
    }
    FREE(vm->root_cons_table);
  }
  for(i = 0; i < vm->n_cons_pages; i++) 
  {
    FREE(vm->cons_pages[i]);
  }
  if(vm->cons_pages) 
  {
    FREE(vm->cons_pages);
  }
}


/*****************************************************************************
 * 
 * Object
 * 
 *****************************************************************************/
#ifdef DEBUG
void * lisp_malloc_object( const char      * file,
                             int               line, 
                             size_t            size,
                             lisp_ref_count_t  rcount )
{
  if(memcheck_next_mock(file, line)) 
  {
    lisp_ref_count_t * obj = malloc(sizeof(lisp_ref_count_t) + size);
    obj[0] = rcount;
    memcheck_register_alloc(file, line, obj);
    return &obj[1];
  }
  else 
  {
    return NULL;
  }
}

void lisp_free_object( const char    * file,
		       int             line,
		       void          * obj)
{
  memcheck_register_freed(file, line, (lisp_ref_count_t*) obj - 1);
  free((lisp_ref_count_t*) obj - 1);
}

#else 

void * lisp_malloc_object( size_t size, lisp_ref_count_t rcount)
{
  lisp_ref_count_t * obj = malloc(sizeof(lisp_ref_count_t) + size);
  obj[0] = rcount;
  return &obj[1];
}


void lisp_free_object( void * obj)
{
  free((lisp_ref_count_t*) obj - 1);
}

#endif

static inline void _lisp_copy_object( lisp_vm_t   * vm,
				      lisp_cell_t * target,
				      const lisp_cell_t * source)
{
  if(LISP_IS_OBJECT(source)) 
  {
    ++LISP_REFCOUNT(source);
  }
  target->type_id = source->type_id;
  target->data    = source->data;
}

static inline int _lisp_copy_object_as_root( lisp_vm_t   * vm,
					     lisp_cell_t * target,
					     const lisp_cell_t * source)
{
  if(LISP_IS_OBJECT(source)) 
  {
    ++LISP_REFCOUNT(source);
  }
  else if(LISP_IS_CONS_OBJECT(source))
  {
    if(lisp_cons_root(vm, source->data.cons)) 
    {
      *target = lisp_nil;
      return LISP_ALLOC_ERROR;
    }
  }
  target->type_id = source->type_id;
  target->data    = source->data;
  return LISP_OK;
}


int lisp_copy_object( lisp_vm_t   * vm,
                      lisp_cell_t * target,
                      const lisp_cell_t * source)
{
  _lisp_copy_object(vm, target, source);
  return LISP_OK;
}

int lisp_copy_object_as_root( lisp_vm_t   * vm,
			      lisp_cell_t * target,
			      const lisp_cell_t * source)
{
  return _lisp_copy_object_as_root(vm, target, source);
}

int lisp_copy_n_objects( lisp_vm_t   * vm,
                         lisp_cell_t * target,
                         const lisp_cell_t * source,
                         lisp_size_t   n)
{
  lisp_size_t i;
  for(i = 0; i < n; i++) 
  {
    _lisp_copy_object(vm, &target[i], &source[i]);
  }
  return LISP_OK;
}


int lisp_copy_n_objects_as_root( lisp_vm_t   * vm,
				 lisp_cell_t * target,
				 const lisp_cell_t * source,
				 lisp_size_t   n)
{
  int ret = 0;
  lisp_size_t i;
  for(i = 0; i < n; i++) 
  {
    ret = _lisp_copy_object_as_root(vm, &target[i], &source[i]);
    if(ret) 
    {
      /* revert transaction */
      lisp_size_t j;
      for(j = 0; j < i; j++) 
      {
        lisp_unset_object_root(vm, &target[j]);
      }
      for(j = i; j < n; j++) 
      {
	target[j] = lisp_nil;
      }
      return ret;
    }
  }
  return LISP_OK;
}

int lisp_unset_object(lisp_vm_t * vm,  lisp_cell_t * target)
{
  if(LISP_IS_OBJECT(target) || LISP_IS_SYMBOL(target)) 
  {
    if(! --LISP_REFCOUNT(target)) 
    {
      if(vm->types[target->type_id].destructor != NULL)
      {					
	vm->types[target->type_id].destructor(vm, target->data.ptr);
      }
      else
      {
	FREE_OBJECT(target->data.ptr);				\
      }								\
    }
  }
  *target = lisp_nil;
  return LISP_OK;
}

int lisp_unset_object_root(lisp_vm_t * vm, lisp_cell_t * target)
{
  if(LISP_IS_OBJECT(target)|| LISP_IS_SYMBOL(target)) 
  {
    if(! --LISP_REFCOUNT(target)) 
    {
      if(vm->types[target->type_id].destructor != NULL)
      {					
	vm->types[target->type_id].destructor(vm, target->data.ptr);
      }
      else
      {
	FREE_OBJECT(target->data.ptr);				\
      }								\
    }
  }
  else if(LISP_IS_CONS_OBJECT(target)) 
  {
    if(lisp_cons_unroot(vm, target->data.cons))
    {
      return LISP_ALLOC_ERROR;
    }
  }
  *target = lisp_nil;
  return LISP_OK;
}

/*******************************************************************
 * 
 * string
 * 
 *******************************************************************/
int lisp_eq_object(const lisp_cell_t * a,
		   const lisp_cell_t * b)
{
  if(a->type_id == b->type_id) 
  {
    /* @todo switch statement and unit test */
    if(LISP_IS_INTEGER(a)) 
    {
      return a->data.integer == b->data.integer;
    }
    else if(a->type_id == LISP_TID_SYMBOL) 
    {
      return a->data.ptr == b->data.ptr;
    }
  }
  else 
  {
    return 0;
  }
}


/*******************************************************************
 * 
 * string
 * 
 *******************************************************************/
int lisp_make_string(lisp_vm_t         * vm,
                     lisp_cell_t       * cell,
                     const lisp_char_t * cstr)
{
  size_t size = strlen(cstr);
  lisp_string_t * str = MALLOC_OBJECT(sizeof(lisp_string_t) + 
                                      sizeof(char) * (size+1),
                                      1);
  str->begin = 0;
  str->end   = size;
  strcpy( (char*) &str[1], cstr);
  cell->type_id = LISP_TID_STRING;
  cell->data.ptr = str;
  return 0;
}


int lisp_sprintf(lisp_vm_t         * vm,
                 lisp_cell_t       * cell,
                 const lisp_char_t * fmt,
                 ...)
{
  int ret;
  int size;
  va_list ap1, ap2;
  va_start(ap1, fmt);
  va_copy(ap2, ap1);
  size = vsnprintf(NULL, 0, fmt, ap1);
  va_end(ap1);
  lisp_string_t * str = MALLOC_OBJECT(sizeof(lisp_string_t) + 
                                      sizeof(char) * (size+1),
                                      1);
  str->begin = 0;
  str->end   = size;
  ret = vsprintf((char*)&str[1], fmt, ap2);  
  va_end(ap2);
  cell->type_id = LISP_TID_STRING;
  cell->data.ptr = str;

  return ret;
}

const char * lisp_c_string(const lisp_cell_t * cell)
{
  return (char*)((const lisp_string_t*)cell->data.ptr + 1);
}




