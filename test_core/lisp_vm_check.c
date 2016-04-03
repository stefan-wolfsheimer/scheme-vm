#include "lisp_vm_check.h"
#include "util/xmalloc.h"
#include "util/unit_test.h"
#include "util/assertion.h"

void set_up_conses(unit_test_t * tst, 
		   lisp_vm_t   * vm,
		   lisp_size_t   n_root_conses,
		   lisp_size_t   n_black_conses,
		   lisp_size_t   n_gap_conses,
		   lisp_size_t   n_grey_conses,
		   lisp_size_t   n_white_conses)
{
  lisp_size_t   n,i,j;
  lisp_cons_t * cons;
  REQUIRE_NEQ_PTR(vm, NULL);
  while(vm->root_cons_table_size - vm->root_cons_top < n_root_conses)
  {
    vm->cons_pages = REALLOC(vm->cons_pages, 
                             sizeof(lisp_cons_t**) * (vm->n_cons_pages + 1) );
    cons = MALLOC(sizeof(lisp_cons_t) * vm->cons_page_size);
    vm->cons_pages[vm->n_cons_pages++] = cons;
    n = vm->root_cons_top + vm->cons_page_size;
    vm->root_cons_table = REALLOC(vm->root_cons_table,
                                  sizeof(lisp_root_cons_t) * n);
    j = 0;
    for(i = vm->root_cons_top; i < n; i++) 
    {
      vm->root_cons_table[i].cons = &cons[j];
      vm->root_cons_table[i].ref_count = 0;
      j++;
    }  
    vm->root_cons_table_size = n;
  }
  for(i = 0; i < n_root_conses; i++) 
  {
    vm->root_cons_table[vm->root_cons_top].ref_count = 1;
    cons = vm->root_cons_table[vm->root_cons_top].cons;
    cons->is_root       = 1;
    cons->gc_cons_index = vm->root_cons_top;
    cons->car           = lisp_nil;
    cons->cdr           = lisp_nil;
    vm->root_cons_top++;
  }

  while( vm->cons_table_size - vm->white_cons_top < 
         n_black_conses + n_gap_conses + n_grey_conses + n_white_conses)    
  {
    vm->cons_pages = REALLOC(vm->cons_pages, 
                             sizeof(lisp_cons_t**) * (vm->n_cons_pages + 1) );
    cons = MALLOC(sizeof(lisp_cons_t) * vm->cons_page_size);
    vm->cons_pages[vm->n_cons_pages++] = cons;
    n = vm->cons_table_size + vm->cons_page_size;
    vm->cons_table = REALLOC(vm->cons_table,
                             sizeof(lisp_cons_t*) * n);
    j = 0;
    for(i = vm->cons_table_size; i < n; i++) 
    {
      vm->cons_table[i] = &cons[j];
      j++;
    }  
    vm->cons_table_size = n;
  }

  for(i = 0; i < n_black_conses; i++) 
  {
    cons = vm->cons_table[vm->black_cons_top];
    cons->is_root = 0;
    cons->gc_cons_index = vm->black_cons_top;
    cons->car           = lisp_nil;
    cons->cdr           = lisp_nil;
    vm->black_cons_top++;
  }

  vm->grey_cons_begin = vm->black_cons_top;
  for(i = 0; i < n_gap_conses; i++) 
  {
    cons = vm->cons_table[vm->grey_cons_begin];
    cons->is_root = 0;
    cons->gc_cons_index = vm->grey_cons_begin;
    cons->car           = lisp_nil;
    cons->cdr           = lisp_nil;

    vm->grey_cons_begin++;
  }

  vm->grey_cons_top = vm->grey_cons_begin;
  for(i = 0; i < n_grey_conses; i++) 
  {
    cons = vm->cons_table[vm->grey_cons_top];
    cons->is_root = 0;
    cons->gc_cons_index = vm->grey_cons_top;
    cons->car           = lisp_nil;
    cons->cdr           = lisp_nil;
    vm->grey_cons_top++;
  }

  vm->white_cons_top = vm->grey_cons_top;
  for(i = 0; i < n_white_conses; i++) 
  {
    cons = vm->cons_table[vm->white_cons_top];
    cons->is_root = 0;
    cons->gc_cons_index = vm->white_cons_top;
    cons->car           = lisp_nil;
    cons->cdr           = lisp_nil;
    vm->white_cons_top++;
  }
}


static void _check_cons_page(unit_test_t * tst, 
                             lisp_cons_t * cons, 
                             lisp_vm_t   * vm,
                             size_t      * cons_page_counter) 
{
  size_t i;
  for(i = 0; i < vm->n_cons_pages; i++) 
  {
    if(cons >= vm->cons_pages[i] && 
       cons <  vm->cons_pages[i] + vm->cons_page_size) 
    {
      cons_page_counter[i]++;
    }
  }
}

static int _has_no_white_children(unit_test_t * tst,
                                  lisp_vm_t   * vm,
                                  lisp_cons_t * cons)
{
  int ret = 1;
  if(LISP_IS_CONS_OBJECT(&cons->car)) 
  {
    if(!cons->car.data.cons->is_root)
    {
      ret &= CHECK_LT_U(tst, 
                        cons->car.data.cons->gc_cons_index, 
                        vm->grey_cons_top);
        
    }
  }
  if(LISP_IS_CONS_OBJECT(&cons->cdr)) 
  {
    if(!cons->cdr.data.cons->is_root)
    {
      ret &= CHECK_LT_U(tst, 
                        cons->cdr.data.cons->gc_cons_index,
                        vm->grey_cons_top);
        
    }
  }
  return ret;
}

int lisp_vm_check(unit_test_t * tst, lisp_vm_t * vm)
{
  size_t i;
  int ret = 1;
  size_t * cons_page_counter = MALLOC(sizeof(size_t)*vm->n_cons_pages);

  ret &= CHECK_LE_U(tst, vm->black_cons_top,  vm->grey_cons_begin);
  ret &= CHECK_LE_U(tst, vm->grey_cons_begin, vm->grey_cons_top);
  ret &= CHECK_LE_U(tst, vm->grey_cons_top,   vm->white_cons_top);
  ret &= CHECK_LE_U(tst, vm->white_cons_top,  vm->cons_table_size);
  ret &= CHECK_LE_U(tst, vm->root_cons_top,   vm->root_cons_table_size);
  for(i = 0; i < vm->n_cons_pages; i++) 
  {
    cons_page_counter[i] = 0;
  }
  for(i = 0; i < vm->cons_table_size; i++) 
  {
    if(i < vm->black_cons_top || 
       ( i >= vm->grey_cons_begin && i < vm->white_cons_top) )
    {
      ret &= CHECK_FALSE(tst, vm->cons_table[i]->is_root);
      ret &= CHECK_EQ_U(tst, vm->cons_table[i]->gc_cons_index, i);
    }
    _check_cons_page(tst, vm->cons_table[i], vm, cons_page_counter);
    if(i < vm->black_cons_top)
    {
      ret &= CHECK(tst, 
                   _has_no_white_children(tst,
                                          vm,
                                          vm->cons_table[i]));
    }
  }
  for(i = 0; i < vm->root_cons_table_size; i++) 
  {
    if(i < vm->root_cons_top) 
    {
      ret &= CHECK(tst, vm->root_cons_table[i].cons->is_root);
      ret &= CHECK_EQ_U(tst, vm->root_cons_table[i].cons->gc_cons_index, i);
      ret &= CHECK(tst, 
                   _has_no_white_children(tst,
                                          vm,
                                          vm->root_cons_table[i].cons));

    }
    _check_cons_page(tst, 
                     vm->root_cons_table[i].cons, 
                     vm,
                     cons_page_counter);
  }
  for(i = 0; i < vm->n_cons_pages; i++) 
  {
    ret &= CHECK_EQ_U(tst, 
                      cons_page_counter[i], 
                      vm->cons_page_size);
  }
  FREE(cons_page_counter);
  return ret;
}

int lisp_is_root_cons(lisp_vm_t * vm, const lisp_cell_t * cell)
{
  return 
    LISP_IS_CONS_OBJECT(cell) && 
    cell->data.cons->is_root;
}

int lisp_is_black_cons(lisp_vm_t * vm, const lisp_cell_t * cell)
{
  return 
    LISP_IS_CONS_OBJECT(cell) && 
    cell->data.cons->gc_cons_index <  vm->black_cons_top;
}

int lisp_is_white_cons(lisp_vm_t * vm, const lisp_cell_t * cell)
{
  return 
    LISP_IS_CONS_OBJECT(cell) && 
    cell->data.cons->gc_cons_index >= vm->grey_cons_top &&
    cell->data.cons->gc_cons_index <  vm->white_cons_top;
}

int lisp_is_grey_cons(lisp_vm_t * vm, const lisp_cell_t * cell)
{
  return 
    LISP_IS_CONS_OBJECT(cell) && 
    cell->data.cons->gc_cons_index >= vm->grey_cons_begin &&
    cell->data.cons->gc_cons_index <  vm->grey_cons_top;
}


lisp_size_t lisp_n_root_cons(lisp_vm_t * vm)
{
  return vm->root_cons_top;
}

lisp_size_t lisp_n_black_cons(lisp_vm_t * vm)
{
  return vm->black_cons_top;
}

lisp_size_t lisp_n_gap_cons(lisp_vm_t * vm)
{
  return vm->grey_cons_begin - vm->black_cons_top;
}


lisp_size_t lisp_n_grey_cons(lisp_vm_t * vm)
{
  return vm->grey_cons_top - vm->grey_cons_begin ;
}

lisp_size_t lisp_n_white_cons(lisp_vm_t * vm)
{
  return vm->white_cons_top - vm->grey_cons_top;
}

lisp_cell_t lisp_get_root_cons( lisp_vm_t * vm, lisp_size_t i)
{
  lisp_cell_t ret;
  ret.type_id = LISP_TID_CONS;
  ret.data.cons = vm->root_cons_table[i].cons;
  return ret;
}

lisp_cell_t lisp_get_white_cons( lisp_vm_t * vm, lisp_size_t i)
{
  lisp_cell_t ret;
  ret.type_id = LISP_TID_CONS;
  ret.data.cons = vm->cons_table[i - vm->grey_cons_top];
  return ret;
}

