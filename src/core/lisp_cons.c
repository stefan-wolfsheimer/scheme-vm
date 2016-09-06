#include "lisp_vm.h"
#include "util/xmalloc.h"
#include "util/assertion.h"
#include "util/mock.h"

static inline int _lisp_make_cons(lisp_vm_t     * vm,
                                  lisp_cell_t   * cell,
                                  lisp_type_id_t  type_id);
static inline int _lisp_make_cons_car_cdr(lisp_vm_t     * vm,
                                          lisp_cell_t   * cell,
                                          lisp_type_id_t  type_id,
                                          const lisp_cell_t * car, 
                                          const lisp_cell_t * cdr);
static inline int _lisp_make_cons_root(lisp_vm_t     * vm,
                                       lisp_cell_t   * cell,
                                       lisp_type_id_t  type_id);
static inline int _lisp_make_cons_root_car_cdr(lisp_vm_t     * vm,
                                               lisp_cell_t   * cell,
                                               lisp_type_id_t  type_id,
                                               const lisp_cell_t * car, 
                                               const lisp_cell_t * cdr);

static inline int _lisp_init_cons_car_cdr(lisp_vm_t * vm,
					  lisp_cons_t * cons,
					  const lisp_cell_t * car,
					  const lisp_cell_t * cdr);
					   

static inline lisp_cons_t * _new_cons_page(lisp_vm_t * vm);
static inline int           _ensure_root_cons_table(lisp_vm_t * vm);
static inline int           _ensure_cons_table(lisp_vm_t * vm);
static inline lisp_cons_t * _insert_root_cons(lisp_vm_t * vm,
                                              lisp_cons_t * cons);
static inline lisp_cons_t * _add_root_cons(lisp_vm_t * vm);

static inline lisp_cons_t * _add_white_cons(lisp_vm_t * vm);

static inline lisp_cons_t * _insert_black_cons(lisp_vm_t * vm,	
                                               lisp_cons_t * cons);
static inline void          _ensure_not_white(lisp_vm_t   * vm,
                                              lisp_cons_t * cons);

/**********************************************************************
 * 
 * public make_cons functions 
 *
 **********************************************************************/
int lisp_make_cons(lisp_vm_t   * vm,
                   lisp_cell_t * cell)
{
  return _lisp_make_cons(vm, cell, LISP_TID_CONS);

}


int lisp_make_cons_car_cdr(lisp_vm_t * vm,
                           lisp_cell_t * cell,
                           const lisp_cell_t * car, 
                           const lisp_cell_t * cdr)
{
  MOCK_CALL(lisp_make_cons_car_cdr, int);
  return _lisp_make_cons_car_cdr(vm, cell, LISP_TID_CONS, car, cdr);
}


int lisp_make_cons_typed(lisp_vm_t     * vm,
                         lisp_cell_t   * cell,
                         lisp_type_id_t  type_id)
{
  REQUIRE(type_id & LISP_TID_CONS_MASK);
  return _lisp_make_cons(vm, cell, type_id);
}


int lisp_make_cons_typed_car_cdr(lisp_vm_t     * vm,
                                 lisp_cell_t   * cell,
                                 lisp_type_id_t  type_id,
                                 const lisp_cell_t * car, 
                                 const lisp_cell_t * cdr)
{
  REQUIRE(type_id & LISP_TID_CONS_MASK);
  return _lisp_make_cons_car_cdr(vm, cell, type_id, car, cdr);
}


int lisp_make_cons_root(lisp_vm_t   * vm,
                        lisp_cell_t * cell)
{
  return _lisp_make_cons_root(vm, cell, LISP_TID_CONS);
}

int lisp_make_cons_root_car_cdr(lisp_vm_t * vm,
                                lisp_cell_t * cell,
                                const lisp_cell_t * car, 
                                const lisp_cell_t * cdr)
{
  MOCK_CALL(lisp_make_cons_root_car_cdr, int);
  return _lisp_make_cons_root_car_cdr(vm, cell, LISP_TID_CONS, car, cdr);
}


int lisp_make_cons_root_typed(lisp_vm_t   * vm,  
                              lisp_cell_t * cell,
                              lisp_type_id_t  type_id)
{
  REQUIRE(type_id & LISP_TID_CONS_MASK);
  return _lisp_make_cons_root(vm, cell, type_id);
}

int lisp_make_cons_root_typed_car_cdr(lisp_vm_t     * vm,
                                      lisp_cell_t   * cell,
                                      lisp_type_id_t  type_id,
                                      const lisp_cell_t * car, 
                                      const lisp_cell_t * cdr)
{
  return _lisp_make_cons_root_car_cdr(vm, cell, type_id, car, cdr);
}

int lisp_cons_root(lisp_vm_t * vm, lisp_cons_t * cons)
{
  if(cons->is_root) 
  {
    REQUIRE_LT_U(cons->gc_cons_index, vm->root_cons_top);
    vm->root_cons_table[cons->gc_cons_index].ref_count++;
    return LISP_OK;
  }
  else 
  {
    lisp_size_t index = cons->gc_cons_index;
    REQUIRE_EQ_PTR(cons, vm->cons_table[index]);
    lisp_cons_t * tmp = _insert_root_cons(vm, cons);
    if(LISP_IS_CONS_OBJECT(&cons->car)) 
    {
      _ensure_not_white(vm, cons->car.data.cons);
    }
    if(LISP_IS_CONS_OBJECT(&cons->cdr)) 
    {
      _ensure_not_white(vm, cons->cdr.data.cons);
    }
    if(tmp == NULL) 
    {
      return LISP_ALLOC_ERROR;
    }
    if(index < vm->black_cons_top) 
    {
      --vm->black_cons_top;
      if(index < vm->black_cons_top) 
      {
	/* not the last black */
	vm->cons_table[index] = vm->cons_table[vm->black_cons_top];
	vm->cons_table[index]->gc_cons_index = index;
      }
      vm->cons_table[vm->black_cons_top] = tmp;
      return LISP_OK;
    }
    else if(index < vm->grey_cons_top) 
    {
      REQUIRE_GE_U(index, vm->grey_cons_begin);
      if(index > vm->grey_cons_begin) 
      {
	/* not the first grey */
	vm->cons_table[index] = vm->cons_table[vm->grey_cons_begin];
	vm->cons_table[index]->gc_cons_index = index;
      }
      vm->cons_table[vm->grey_cons_begin] = tmp;
      ++vm->grey_cons_begin;
      REQUIRE_LE_U(vm->grey_cons_begin, vm->grey_cons_top);
      return LISP_OK;
    }
    else 
    {
      REQUIRE_LT_U(index, vm->white_cons_top);
      --vm->white_cons_top;
      if(index != vm->white_cons_top) 
      {
	vm->cons_table[index] = vm->cons_table[vm->white_cons_top];
	vm->cons_table[index]->gc_cons_index = index;
      }
      vm->cons_table[vm->white_cons_top] = tmp;
      return LISP_OK;
    }
  }
  return LISP_OK;
}

int lisp_cons_unroot(lisp_vm_t * vm, lisp_cons_t * cons)
{
  if(cons->is_root) 
  {
    REQUIRE_LT_U(cons->gc_cons_index, vm->root_cons_top);
    REQUIRE_GT_U(vm->root_cons_table[cons->gc_cons_index].ref_count, 0);
    if(!--vm->root_cons_table[cons->gc_cons_index].ref_count) 
    {
      size_t index = cons->gc_cons_index;
      lisp_cons_t * tmp = _insert_black_cons(vm, cons);
      if(tmp == NULL) 
      {
	/* roll back */
	vm->root_cons_table[cons->gc_cons_index].ref_count++;
	return LISP_ALLOC_ERROR;
      }
      vm->root_cons_table[index] = vm->root_cons_table[vm->root_cons_top];
      vm->root_cons_table[index].cons->gc_cons_index = index;
      vm->root_cons_top--;
      vm->root_cons_table[vm->root_cons_top].cons = tmp;
      return LISP_OK;
    }
    return LISP_OK;
  }
  return LISP_OK;
}

lisp_ref_count_t lisp_root_refcount(lisp_vm_t         * vm,
				    const lisp_cell_t * cell)
{
  if(LISP_IS_CONS_OBJECT(cell) && cell->data.cons->is_root) 
  {
    REQUIRE_LT_U(cell->data.cons->gc_cons_index, vm->root_cons_top);
    REQUIRE_GT_U(vm->root_cons_table[cell->data.cons->gc_cons_index].ref_count,
		 0);
    return vm->root_cons_table[cell->data.cons->gc_cons_index].ref_count;
  }
  else 
  {
    return 0;
  }
}

int lisp_cons_set_car_cdr(lisp_vm_t * vm, 
			  lisp_cons_t * cons, 
			  const lisp_cell_t * car,
			  const lisp_cell_t * cdr)
{
  /* @todo check color!!!! */
  if(car != NULL) 
  {
    if(lisp_unset_object(vm, &cons->car)) return LISP_ALLOC_ERROR;
  }
  if(cdr != NULL) 
  {
    if(lisp_unset_object(vm, &cons->cdr)) return LISP_ALLOC_ERROR;
  }
  return _lisp_init_cons_car_cdr(vm,
				 cons,
				 car,
				 cdr);
}



/*********************************************************
 * 
 * private details 
 *
 *********************************************************/

/*
  +----------------------------+ <-               
  | disposed_conses_1          |
  +----------------------------+ <- 0             
  | black conses               |
  +----------------------------+ <- black_top     
  |                            |
  +----------------------------+ <- begin_grey
  | grey conses                |
  +----------------------------+ <- grey_top      
  | white conses               |
  +----------------------------+ <- white_top     
  | disposed_conses_2          |
  +----------------------------+ <- cons_table_size
*/

int _lisp_init_cons_car_cdr(lisp_vm_t         * vm,
			    lisp_cons_t       * cons,
			    const lisp_cell_t * car,
			    const lisp_cell_t * cdr)
{
  if(car != NULL) 
  {
    if(LISP_IS_ATOM(car)) 
    {
      cons->car.type_id = car->type_id;
      cons->car.data    = car->data;
    }
    else if(LISP_IS_OBJECT(car)) 
    {
      ++LISP_REFCOUNT(car);
      cons->car.type_id = car->type_id;
      cons->car.data    = car->data;
    }
    else 
    {
      REQUIRE(LISP_IS_CONS_OBJECT(car));
      if(cons->is_root || cons->gc_cons_index < vm->black_cons_top) 
      {
	_ensure_not_white(vm, car->data.cons);
      }
      cons->car.type_id = car->type_id;
      cons->car.data    = car->data;
    }
  }
  if(cdr != NULL) 
  {
    if(LISP_IS_ATOM(cdr)) 
    {
      cons->cdr.type_id = cdr->type_id;
      cons->cdr.data    = cdr->data;
    }
    else if(LISP_IS_OBJECT(cdr)) 
    {
      ++LISP_REFCOUNT(cdr);
      cons->cdr.type_id = cdr->type_id;
      cons->cdr.data    = cdr->data;
    }
    else 
    {
      REQUIRE(LISP_IS_CONS_OBJECT(cdr));
      if(cons->is_root || cons->gc_cons_index < vm->black_cons_top) 
      {
	_ensure_not_white(vm, cdr->data.cons);
      }
      cons->cdr.type_id   = cdr->type_id;
      cons->cdr.data.cons = cdr->data.cons;
    }
  }
  return LISP_OK;
}

int _lisp_make_cons(lisp_vm_t     * vm,
                    lisp_cell_t   * cell,
                    lisp_type_id_t  type_id)
{
  if(_ensure_cons_table(vm)) 
  {
    return LISP_ALLOC_ERROR;
  }
  register lisp_cons_t * cons = vm->cons_table[vm->white_cons_top];
  cons->is_root = 0;
  cons->gc_cons_index = vm->white_cons_top;
  cons->car = lisp_nil;
  cons->cdr = lisp_nil;
  vm->white_cons_top++;
  cell->type_id = type_id;
  cell->data.cons = cons;
  return LISP_OK;
}

static inline int _lisp_make_cons_car_cdr(lisp_vm_t     * vm,
                                          lisp_cell_t   * cell,
                                          lisp_type_id_t  type_id,
					  const lisp_cell_t * car, 
					  const lisp_cell_t * cdr)
{
  if(_ensure_cons_table(vm)) 
  {
    return LISP_ALLOC_ERROR;
  }
  register lisp_cons_t * cons = vm->cons_table[vm->white_cons_top];
  cons->is_root = 0;
  cons->gc_cons_index = vm->white_cons_top;
  if(lisp_copy_object(vm, &cons->car, car)) return LISP_ALLOC_ERROR;
  if(lisp_copy_object(vm, &cons->cdr, cdr)) return LISP_ALLOC_ERROR;
  vm->white_cons_top++;
  cell->type_id = type_id;
  cell->data.cons = cons;
  return LISP_OK;
}

int _lisp_make_cons_root(lisp_vm_t     * vm,
                         lisp_cell_t   * cell,
                         lisp_type_id_t  type_id)
{
  lisp_cons_t * cons = _add_root_cons(vm);
  if(cons == NULL) 
  {
    return LISP_ALLOC_ERROR;
  }
  vm->root_cons_table[vm->root_cons_top].ref_count = 1;
  cons->car = lisp_nil;
  cons->cdr = lisp_nil;
  cell->type_id = type_id;
  cell->data.cons = cons;
  return LISP_OK;
}


int _lisp_make_cons_root_car_cdr(lisp_vm_t     * vm,
                                 lisp_cell_t   * cell,
                                 lisp_type_id_t  type_id,
                                 const lisp_cell_t * car, 
                                 const lisp_cell_t * cdr)
{ 
  lisp_cons_t * cons = _add_root_cons(vm);
  if(cons == NULL) 
  {
    return LISP_ALLOC_ERROR;
  }
  vm->root_cons_table[vm->root_cons_top].ref_count = 1;
  cons->car = lisp_nil;
  cons->cdr = lisp_nil;
  int ret = _lisp_init_cons_car_cdr(vm, cons, car, cdr);
  cell->type_id = type_id;
  cell->data.cons = cons;
  return ret;
}


static inline lisp_cons_t * _new_cons_page(lisp_vm_t * gc) 
{
  lisp_cons_t * cons_array;
  lisp_cons_t ** pages;
  pages = REALLOC(gc->cons_pages, 
		  sizeof(lisp_cons_t**) * (gc->n_cons_pages + 1) );
  if(pages == NULL) return NULL;
  gc->cons_pages = pages;
  cons_array = MALLOC(sizeof(lisp_cons_t) * gc->cons_page_size);
  if(cons_array == NULL) return NULL;
  gc->cons_pages[gc->n_cons_pages++] = cons_array;
  return cons_array;
}

static inline int _ensure_root_cons_table(lisp_vm_t * vm)
{
  REQUIRE(vm->root_cons_top <= vm->root_cons_table_size);
  if(vm->root_cons_top == vm->root_cons_table_size) 
  {
    size_t n,i,j;
    lisp_cons_t * cons_array = _new_cons_page(vm);
    if(cons_array == NULL) return LISP_ALLOC_ERROR;
    n = vm->root_cons_top + vm->cons_page_size;
    lisp_root_cons_t * tmp = REALLOC(vm->root_cons_table,
				     sizeof(lisp_root_cons_t)*n);
    if(tmp == NULL) return LISP_ALLOC_ERROR;
    j = 0;
    for(i = vm->root_cons_top; i < n; i++) 
    {
      tmp[i].cons = &cons_array[j];
      tmp[i].ref_count = 0;
      j++;
    }  
    vm->root_cons_table      = tmp;
    vm->root_cons_table_size = n;
  }
  return LISP_OK;
}

static inline int _ensure_cons_table(lisp_vm_t * vm)
{
  REQUIRE_LE_U(vm->white_cons_top, vm->cons_table_size);
  if(vm->white_cons_top == vm->cons_table_size) 
  {
    size_t n,i,j;
    lisp_cons_t * cons_array = _new_cons_page(vm);
    if(cons_array == NULL) return LISP_ALLOC_ERROR;
    n = vm->white_cons_top + vm->cons_page_size;
    lisp_cons_t ** tmp = REALLOC(vm->cons_table,
				sizeof(lisp_cons_t*)*n);
    if(tmp == NULL) return LISP_ALLOC_ERROR;
    j = 0;
    for(i = vm->white_cons_top; i < n; i++) 
    {
      tmp[i] = &cons_array[j];
      j++;
    }  
    vm->cons_table      = tmp;
    vm->cons_table_size = n;
  }
  return LISP_OK;
}

static inline lisp_cons_t * _insert_root_cons(lisp_vm_t * vm,
					      lisp_cons_t * cons)
{
  if(_ensure_root_cons_table(vm)) 
  {
    return NULL;
  }
  lisp_cons_t * ret = vm->root_cons_table[vm->root_cons_top].cons;
  cons->gc_cons_index = vm->root_cons_top;
  cons->is_root = 1;
  vm->root_cons_table[vm->root_cons_top].cons = cons;
  vm->root_cons_table[vm->root_cons_top].ref_count = 1;
  vm->root_cons_top++;
  return ret;
}

static inline lisp_cons_t * _add_root_cons(lisp_vm_t * vm)
{
  if(_ensure_root_cons_table(vm)) 
  {
    return NULL;
  }
  lisp_cons_t * ret = vm->root_cons_table[vm->root_cons_top].cons;
  vm->root_cons_table[vm->root_cons_top].ref_count = 1;
  ret->is_root       = 1;
  ret->gc_cons_index = vm->root_cons_top;
  vm->root_cons_top++;
  return ret;
}

static inline lisp_cons_t * _add_white_cons(lisp_vm_t * vm)
{
  REQUIRE_LE_U(vm->white_cons_top, vm->cons_table_size);
  if(_ensure_cons_table(vm))
  {
    /* cannot allocate */
    return NULL;
  }
  lisp_cons_t * ret = vm->cons_table[vm->white_cons_top];
  ret->is_root = 0;
  ret->gc_cons_index = vm->white_cons_top;
  vm->white_cons_top++;
  return ret;
}

static inline lisp_cons_t * _insert_black_cons(lisp_vm_t * vm,	
                                               lisp_cons_t * cons)
{
  REQUIRE_LE_U(vm->black_cons_top, vm->grey_cons_begin);
  REQUIRE_LE_U(vm->white_cons_top, vm->cons_table_size);
  lisp_cons_t * ret;
  if(vm->black_cons_top == vm->grey_cons_begin) 
  {
    if(_ensure_cons_table(vm))
    {
      /* cannot allocate */
      return NULL;
    }
    ret = vm->cons_table[vm->white_cons_top];
    if(vm->white_cons_top != vm->grey_cons_top) 
    {
      vm->cons_table[vm->white_cons_top] = vm->cons_table[vm->grey_cons_top];
      vm->cons_table[vm->white_cons_top]->gc_cons_index = vm->white_cons_top;
      
    }
    if(vm->grey_cons_begin != vm->grey_cons_top) 
    {
      vm->cons_table[vm->grey_cons_top] = vm->cons_table[vm->grey_cons_begin];
      vm->cons_table[vm->grey_cons_top]->gc_cons_index = vm->grey_cons_top;
    }
    cons->is_root = 0;
    cons->gc_cons_index = vm->grey_cons_begin;
    vm->cons_table[vm->grey_cons_begin] = cons;
    vm->black_cons_top++;
    vm->grey_cons_begin++;
    vm->grey_cons_top++;
    vm->white_cons_top++;
    return ret;
  }
  else 
  {
    ret = vm->cons_table[vm->black_cons_top];
    cons->is_root = 0;
    cons->gc_cons_index = vm->black_cons_top;
    vm->cons_table[vm->black_cons_top++] = cons;
    return ret;
  }
}

static inline void _ensure_not_white(lisp_vm_t   * vm,
                                     lisp_cons_t * cons)
{
  if(!cons->is_root && 
     cons->gc_cons_index >= vm->grey_cons_top) 
  {
    REQUIRE_LT_U(cons->gc_cons_index, vm->white_cons_top);
    if(cons->gc_cons_index > vm->grey_cons_top) 
    {
      REQUIRE_EQ_PTR(cons, vm->cons_table[cons->gc_cons_index]);
      lisp_cons_t * tmp = vm->cons_table[vm->grey_cons_top];
      tmp->gc_cons_index = cons->gc_cons_index;
      vm->cons_table[cons->gc_cons_index] = tmp;
      cons->gc_cons_index = vm->grey_cons_top;
      vm->cons_table[vm->grey_cons_top] = cons;
    }
    vm->grey_cons_top++;
  }
}

int lisp_make_list(lisp_vm_t         * vm,
		   lisp_cell_t       * cell,
		   const lisp_cell_t * elems,
		   lisp_size_t         n)
{
  MOCK_CALL(lisp_make_list, int);
  if(n == 0) 
  {
    *cell = lisp_nil;
    return LISP_OK;
  }
  else 
  {
    *cell = lisp_nil;
    lisp_cell_t tmp;
    int ret = LISP_OK;
    while(n > 0) 
    {
      ret = lisp_make_cons_car_cdr(vm, &tmp, &elems[n-1], &lisp_nil);
      if(ret) 
      {
	return ret;
      }
      *LISP_CDR(&tmp) = *cell;
      *cell = tmp;
      n--;
    }
    return LISP_OK;
  }
}

int lisp_make_list_root(lisp_vm_t         * vm,
			lisp_cell_t       * cell,
			const lisp_cell_t * elems,
			lisp_size_t         n)
{
  *cell = lisp_nil;
  if(n == 0) 
  {
    return LISP_OK;
  }
  else 
  {
    lisp_cell_t rest = lisp_nil;
    int ret = lisp_make_list(vm, &rest, &elems[1], n-1);
    if(ret) 
    {
      return ret;
    }
    ret = lisp_make_cons_root_car_cdr(vm, cell, elems, &rest);
    if(ret)
    {
      return ret;
    }
    return LISP_OK;
  }
}

int lisp_make_list_root_typed(lisp_vm_t         * vm,
                              lisp_cell_t       * cell,
                              lisp_type_id_t      type_id,
                              const lisp_cell_t * elems,
                              lisp_size_t         n)
{
  *cell = lisp_nil;
  if(n == 0)
  {
    return LISP_OK;
  }
  else
  {
    lisp_cell_t rest = lisp_nil;
    int ret = lisp_make_list(vm, &rest, &elems[1], n-1);
    if(ret)
    {
      return ret;
    }
    ret = lisp_make_cons_root_typed_car_cdr(vm, cell, type_id, elems, &rest);
    if(ret)
    {
      return ret;
    }
    return LISP_OK;
  }
}
