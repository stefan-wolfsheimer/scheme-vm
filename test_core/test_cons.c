#include "util/xmalloc.h"
#include "util/unit_test.h"
#include "core/lisp_vm.h" 
#include "lisp_vm_check.h"
#include <stdio.h>
#include <ctype.h>
/* @todo test cons of objects, cow objects and atoms (other than nil) 
 * @todo test set_car, set_cdr
 */

static lisp_cell_t   _get_cons(lisp_vm_t   * vm, lisp_size_t   i);

/* c=0: root 
 * c=1: black, 
 * c=2: grey
 * c=3: white 
*/
static lisp_cons_t * _get_different_cons(lisp_vm_t   * vm, 
                                         lisp_size_t c,
                                         lisp_cons_t * cons1, lisp_cons_t * cons2);


/* test combinations of 
   make_cons / make_root_cons of
   (a.a) 
   with a in (nil, self, root, black, grey, white) -> 6 combinations
   and 
   (a.b) 
   with a!=b and a,b in (nil, root, black, grey, white) -> 25-1 combinations
   total = 6 + 24 = 30
*/
#define N_CAR_CONS_COMBINATIONS 30
static void __set_different_root_cons()
{
}

static int _get_car_cdr_combination(lisp_vm_t    * vm,
                                    lisp_cons_t  * cons, /* can be NULL */
                                    lisp_cell_t  * car,
                                    lisp_cell_t  * cdr,
                                    lisp_size_t    i)
{
  car->type_id = LISP_TID_NIL;
  cdr->type_id = LISP_TID_NIL;
  car->data.cons = NULL;
  cdr->data.cons = NULL;

  if(i == 0) 
  {
    /* (nil.nil) */
    return 1;
  }
  else if(i == 1) 
  {
    /* (self.self) */
    if(cons) 
    {
      car->type_id = LISP_TID_CONS;
      car->data.cons = cons;
      cdr->type_id = LISP_TID_CONS;
      cdr->data.cons = cons;
      return 1;
    }
    else 
    {
      return 0;
    }
  }
  else if(i < 6) 
  {
    car->data.cons = _get_different_cons(vm, i-1, cons, NULL);
    if(car->data.cons) 
    {
      cdr->data.cons = car->data.cons;
      car->type_id = LISP_TID_CONS;
      cdr->type_id = LISP_TID_CONS;
      return 1;
    }
    else 
    {
      return 0;
    }
  }
  else if(i < 30) 
  {
    lisp_size_t ta = ((i+1)-6) / 5;
    lisp_size_t tb = ((i+1)-6) % 5;
    if(ta >= 1 && ta < 5) 
    {
      car->data.cons = _get_different_cons(vm, ta-1, cons, NULL);
      if(car->data.cons) 
      {
         car->type_id = LISP_TID_CONS;
      }
      else 
      {
         return 0;
      }
    }
    if(tb >= 1 && tb < 5) 
    {
      cdr->data.cons = _get_different_cons(vm, tb-1, cons, car->data.cons);
      if(cdr->data.cons) 
      {
         cdr->type_id = LISP_TID_CONS;
      }
      else 
      {
         return 0;
      }
    }
  }
  return 0;
}

static lisp_cell_t _get_cons(lisp_vm_t * vm, lisp_size_t   i)
{
  lisp_cell_t ret;
  ret.type_id   = LISP_TID_CONS;
  ret.data.cons = vm->cons_table[i];
  return ret;
}

static lisp_cons_t * _get_different_cons(lisp_vm_t   * vm, 
                                         lisp_size_t c,
                                         lisp_cons_t * cons1, 
                                         lisp_cons_t * cons2)
{
  lisp_size_t i = 0;  
  lisp_size_t end = 0;

  if(c == 0) 
  {
    /* root */
    while(i < vm->root_cons_table_size) 
    {
      if(vm->root_cons_table[i].cons != cons1 && 
         vm->root_cons_table[i].cons != cons2)
      {
        return vm->root_cons_table[i].cons;
      }
      i++;
    }
    return NULL;
  }
  else if(c == 1)
  {
    i     = 0;
    end   = vm->black_cons_top;
  }
  else if(c == 2) 
  {
    i     = vm->grey_cons_begin;
    end   = vm->grey_cons_top;
  }
  else if(c == 3) 
  {
    i     = vm->grey_cons_top;
    end   = vm->white_cons_top;
  }
  while(i < end)
  {
    if(vm->cons_table[i] != cons1 && 
       vm->cons_table[i] != cons2) 
    {
      return vm->cons_table[i];
    }
    i++;
  }
  return NULL;
}


static void test_cons_set_up(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  lisp_size_t    n_root_conses = 10;
  lisp_size_t    n_black_conses = 12;
  lisp_size_t    n_gap_conses = 3;
  lisp_size_t    n_grey_conses = 7;
  lisp_size_t    n_white_conses = 5;
  set_up_conses(tst, 
                 vm,
                 n_root_conses,
                 n_black_conses,
                 n_gap_conses,
                 n_grey_conses,
                 n_white_conses);
  ASSERT(tst,      
         lisp_vm_check(tst, vm));
  ASSERT_EQ_U(tst, 
              lisp_n_root_cons(vm), n_root_conses);
  ASSERT_EQ_U(tst, 
              lisp_n_black_cons(vm), n_black_conses);
  ASSERT_EQ_U(tst, 
              lisp_n_grey_cons(vm), n_grey_conses);
  ASSERT_EQ_U(tst, 
              lisp_n_white_cons(vm), n_white_conses);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_get_different_root_cons(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_size_t    n_root_conses = 10;
  set_up_conses(tst, vm, n_root_conses, 0, 0, 0, 0);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 
                                         0,
                                         NULL, 
                                         NULL), 
                lisp_get_root_cons(vm, 0).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 
                                         0,
                                         lisp_get_root_cons(vm, 0).data.cons, 
                                         NULL), 
                lisp_get_root_cons(vm, 1).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 
                                         0,
                                         lisp_get_root_cons(vm, 0).data.cons,
                                         lisp_get_root_cons(vm, 0).data.cons 
                                              ),
                lisp_get_root_cons(vm, 1).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 
                                         0,
                                         lisp_get_root_cons(vm, 0).data.cons,
                                         lisp_get_root_cons(vm, 1).data.cons 
                                         ),
                lisp_get_root_cons(vm, 2).data.cons);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_get_different_cons(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_size_t    n_black_conses  = 10;
  lisp_size_t    n_gap_conses    = 10;
  lisp_size_t    n_grey_conses   = 10;
  lisp_size_t    n_white_conses  = 10;
  set_up_conses(tst, 
                 vm,
                 0,
                 n_black_conses,
                 n_gap_conses,
                 n_grey_conses,
                 n_white_conses);
  /* black */
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 1,
                                         NULL, 
                                         NULL), 
                _get_cons(vm, 0).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 1,
                                         _get_cons(vm, 0).data.cons, 
                                         NULL), 
                _get_cons(vm, 1).data.cons);

  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 1,
                                         _get_cons(vm, 0).data.cons, 
                                         _get_cons(vm, 0).data.cons), 
                _get_cons(vm, 1).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 1,
                                         _get_cons(vm, 0).data.cons, 
                                         _get_cons(vm, 1).data.cons), 
                _get_cons(vm, 2).data.cons);
  /* grey */
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 2,
                                         NULL, 
                                         NULL), 
                _get_cons(vm, 20).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 2,
                                         _get_cons(vm, 20).data.cons, 
                                         NULL), 
                _get_cons(vm, 21).data.cons);

  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 2,
                                         _get_cons(vm, 20).data.cons, 
                                         _get_cons(vm, 20).data.cons), 
                _get_cons(vm, 21).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 2,
                                         _get_cons(vm, 20).data.cons, 
                                         _get_cons(vm, 21).data.cons), 
                _get_cons(vm, 22).data.cons);

  /* white */
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 3,
                                         NULL, 
                                         NULL), 
                _get_cons(vm, 30).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 3,
                                         _get_cons(vm, 30).data.cons, 
                                         NULL), 
                _get_cons(vm, 31).data.cons);

  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 3,
                                         _get_cons(vm, 30).data.cons, 
                                         _get_cons(vm, 30).data.cons), 
                _get_cons(vm, 31).data.cons);
  ASSERT_EQ_PTR(tst, _get_different_cons(vm, 3,
                                         _get_cons(vm, 30).data.cons, 
                                         _get_cons(vm, 31).data.cons), 
                _get_cons(vm, 32).data.cons);

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static int test_new_root_cons_prepared(unit_test_t  * tst,
                                       lisp_size_t    n_root_conses,
                                       lisp_size_t    n_black_conses,
                                       lisp_size_t    n_gap_conses,
                                       lisp_size_t    n_grey_conses,
                                       lisp_size_t    n_white_conses)
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_size_t c;
  int ret  = 1;
  lisp_cell_t root_cons;
  for(c = 0; c < N_CAR_CONS_COMBINATIONS; c++) 
  {
    lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
    set_up_conses(tst, vm,
                   n_root_conses,
                   n_black_conses,
                   n_gap_conses,
                   n_grey_conses,
                   n_white_conses);
    lisp_cell_t    cons, car, cdr;
    if(_get_car_cdr_combination(vm, NULL, &car, &cdr, c)) 
    {
      /* make cons */
      int delta_root  = 1;
      int delta_black = 0;
      int delta_grey  = 0;
      int delta_white = 0;
      if(LISP_IS_NIL(&car) && LISP_IS_NIL(&cdr)) 
      {
        ret&= CHECK_FALSE(tst, lisp_make_cons_root(vm, &cons));
        ret&= CHECK(tst, LISP_IS_NIL(&cons.data.cons->car));
        ret&= CHECK(tst, LISP_IS_NIL(&cons.data.cons->cdr));
      }
      else 
      {
        if(lisp_is_white_cons(vm, &car)) 
        {
          delta_white -= 1;
          delta_grey  += 1;
        }
        if(car.data.cons != cdr.data.cons && 
           lisp_is_white_cons(vm, &cdr)) 
        {
          delta_white -= 1;
          delta_grey  += 1;
        }
        ret&= CHECK_FALSE(tst, lisp_make_cons_root_car_cdr(vm, 
                                                           &cons, 
                                                           &car, &cdr));
      }
      ret&= CHECK(tst,       LISP_IS_CONS(&cons));
      ret&= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 1);
      ret&= CHECK_EQ_U(tst,  lisp_n_root_cons(vm),  n_root_conses  + delta_root);
      ret&= CHECK_EQ_U(tst,  lisp_n_black_cons(vm), n_black_conses + delta_black);
      ret&= CHECK_EQ_U(tst,  lisp_n_grey_cons(vm),  n_grey_conses  + delta_grey);
      ret&= CHECK_EQ_U(tst,  lisp_n_white_cons(vm), n_white_conses + delta_white);
      ret&= CHECK(tst,       lisp_vm_check(tst, vm));

      /* increase root count */
      ret&= CHECK_FALSE(tst, lisp_copy_object_as_root(vm, &root_cons, &cons));
      ret&= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 2);
      ret&= CHECK_EQ_U(tst,  lisp_n_root_cons(vm),  n_root_conses  + delta_root);
      ret&= CHECK_EQ_U(tst,  lisp_n_black_cons(vm), n_black_conses + delta_black);
      ret&= CHECK_EQ_U(tst,  lisp_n_grey_cons(vm),  n_grey_conses  + delta_grey);
      ret&= CHECK_EQ_U(tst,  lisp_n_white_cons(vm), n_white_conses + delta_white);
    
      /* decrease root count */
      ret&= CHECK_FALSE(tst, lisp_unset_object_root(vm, &root_cons));
      ret&= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 1);
      ret&= CHECK_EQ_U(tst, lisp_n_root_cons(vm),  n_root_conses  + delta_root);
      ret&= CHECK_EQ_U(tst, lisp_n_black_cons(vm), n_black_conses + delta_black);
      ret&= CHECK_EQ_U(tst, lisp_n_grey_cons(vm),  n_grey_conses  + delta_grey);
      ret&= CHECK_EQ_U(tst, lisp_n_white_cons(vm), n_white_conses + delta_white);

      /* decrease root count */
      delta_root--;
      delta_black++;
      ret&= CHECK_FALSE(tst, lisp_cons_unroot(vm, cons.data.cons));
      ret&= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 0);
      ret&= CHECK(tst,       lisp_is_black_cons(vm, &cons));
      ret&= CHECK_EQ_U(tst, lisp_n_root_cons(vm),  n_root_conses  + delta_root);
      ret&= CHECK_EQ_U(tst, lisp_n_black_cons(vm), n_black_conses + delta_black);
      ret&= CHECK_EQ_U(tst, lisp_n_grey_cons(vm),  n_grey_conses  + delta_grey);
      ret&= CHECK_EQ_U(tst, lisp_n_white_cons(vm), n_white_conses + delta_white);
      ret&= CHECK(tst,       lisp_vm_check(tst, vm));
    }
    lisp_free_vm(vm);
  }
  ret&= CHECK_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
  return ret;
}

static void test_make_cons_root(unit_test_t * tst)
{
  ASSERT(tst, test_new_root_cons_prepared(tst,
                                          0, /* n_root_conses */
                                          0, /* n_black_conses */
                                          0, /* n_gap_conses, */
                                          0, /* n_grey_conses, */
                                          0  /* n_white_conses */
                                          ));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 0, 0, 0, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 0, 0, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 2, 0, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 0, 0, 2, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 0, 2, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 0, 2, 2, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 2, 2, 0));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 0, 0, 2));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 2, 0, 2));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 0, 0, 2, 2));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 0, 2, 2));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 0, 2, 2, 2));
  ASSERT(tst, test_new_root_cons_prepared(tst, 1, 2, 2, 2, 2));
}

static int test_make_cons_prepared(unit_test_t  * tst,
                                   lisp_size_t    n_root_conses,
                                   lisp_size_t    n_black_conses,
                                   lisp_size_t    n_gap_conses,
                                   lisp_size_t    n_grey_conses,
                                   lisp_size_t    n_white_conses)
{
  /* @todo add tests with lisp_copy_object_root() */
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_size_t c;
  int ret  = 1;
  for(c = 0; c < N_CAR_CONS_COMBINATIONS; c++) 
  {
    lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
    set_up_conses(tst, vm,
                   n_root_conses,
                   n_black_conses,
                   n_gap_conses,
                   n_grey_conses,
                   n_white_conses);
    lisp_cell_t    cons, car, cdr;
    if(_get_car_cdr_combination(vm, NULL, &car, &cdr, c)) 
    {
      /* make cons */
      int delta_root  = 0;
      int delta_black = 0;
      int delta_grey  = 0;
      int delta_white = 1;

      if(LISP_IS_NIL(&car) && LISP_IS_NIL(&cdr)) 
      {
        ret&= CHECK_FALSE(tst, lisp_make_cons(vm, &cons));
        ret&= CHECK(tst, LISP_IS_NIL(&cons.data.cons->car));
        ret&= CHECK(tst, LISP_IS_NIL(&cons.data.cons->cdr));
      }
      else 
      {
        /* @todo implement lisp_make_cons_car_cdr 
         *       and add similar test with set_car and set_cdr 
         */
        ret&= CHECK_FALSE(tst, lisp_make_cons_car_cdr(vm, 
                                                      &cons, 
                                                      &car, &cdr));
      }
      ret&= CHECK_EQ_U(tst, lisp_n_root_cons(vm),  n_root_conses  + delta_root);
      ret&= CHECK_EQ_U(tst, lisp_n_black_cons(vm), n_black_conses + delta_black);
      ret&= CHECK_EQ_U(tst, lisp_n_grey_cons(vm),  n_grey_conses  + delta_grey);
      ret&= CHECK_EQ_U(tst, lisp_n_white_cons(vm), n_white_conses + delta_white);
      ret&= CHECK(tst,       lisp_vm_check(tst, vm));
      ret&= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 0);
      ret&= CHECK(tst,       lisp_vm_check(tst, vm));
    }
    lisp_free_vm(vm);
  }

  ret&= CHECK_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
  return ret;
}


static void test_make_cons(unit_test_t * tst)
{
  ASSERT(tst, test_make_cons_prepared(tst,
                                      0, /* n_root_conses */
                                      0, /* n_black_conses */
                                      0, /* n_gap_conses, */
                                      0, /* n_grey_conses, */
                                      0  /* n_white_conses */
                                      ));
  ASSERT(tst, test_make_cons_prepared(tst, 1, 0, 0, 0, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 1, 2, 0, 0, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 1, 2, 2, 0, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 1, 0, 0, 2, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 1, 2, 0, 2, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 1, 0, 2, 2, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 2, 2, 2, 0));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 2, 0, 0, 2));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 2, 2, 0, 2));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 0, 0, 2, 2));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 2, 0, 2, 2));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 0, 2, 2, 2));
  ASSERT(tst, test_make_cons_prepared(tst, 0, 2, 2, 2, 2));
}

static void test_copy_cons(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t    cons;
  lisp_cell_t    copied_cons;
  size_t         n_white_conses = 10;
  set_up_conses(tst, 
                 vm,
                 10,
                 10,
                 0,
                 10,
                 n_white_conses);

  /* make a cons */
  ASSERT_FALSE(tst, lisp_make_cons(vm, &cons));
  ASSERT(tst,       LISP_IS_CONS(&cons));
  ASSERT_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 0);
  ASSERT_EQ_U(tst,  lisp_n_white_cons(vm), n_white_conses + 1);
  ASSERT(tst,       lisp_vm_check(tst, vm));
  /* make a copy */
  ASSERT_FALSE(tst, lisp_copy_object(vm, &copied_cons, &cons));
  ASSERT(tst,       LISP_IS_CONS(&copied_cons));
  ASSERT_EQ_U(tst,  lisp_cons_root_refcount(vm, &copied_cons), 0);
  ASSERT_EQ_U(tst,  lisp_n_white_cons(vm), n_white_conses + 1);
  ASSERT(tst,       lisp_vm_check(tst, vm));
  /* @todo copy a root cons, copy a cons to root set */
}

static int test_color2root_prepared(unit_test_t * tst, 
                                    lisp_size_t   n_root,
                                    lisp_size_t   n_black,
                                    lisp_size_t   n_gap,
                                    lisp_size_t   n_grey,
                                    lisp_size_t   n_white,
                                    lisp_size_t   i)
{
  /* @todo add cases with set-car / set-cdr */
  int            ret      = 1;
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t    * vm;
  lisp_cell_t    cons;
  lisp_size_t    n_root2  = n_root; 
  lisp_size_t    n_black2 = n_black;
  lisp_size_t    n_gap2   = n_gap;
  lisp_size_t    n_grey2  = n_grey;
  lisp_size_t    n_white2 = n_white;

  lisp_size_t    n_root3  = n_root; 
  lisp_size_t    n_black3 = n_black;
  lisp_size_t    n_gap3   = n_gap;
  lisp_size_t    n_grey3  = n_grey;
  lisp_size_t    n_white3 = n_white;
  lisp_cell_t    root_cons;

  vm = lisp_create_vm(&lisp_vm_default_param);
  set_up_conses(tst, vm, n_root, n_black, n_gap, n_grey, n_white);
  cons = _get_cons(vm, i);
  ret &=  CHECK(tst, LISP_IS_CONS_OBJECT(&cons));
  if(lisp_is_black_cons(vm, &cons)) 
  {
    n_root2++;  
    n_black2--;
    n_gap2++;
  }
  else if(lisp_is_grey_cons(vm, &cons)) 
  {
    n_root2++;
    n_grey2--;
    n_gap2++;
    n_black3++;
    n_grey3--;
  }
  else if(lisp_is_white_cons(vm, &cons)) 
  {
    n_root2++;
    n_white2--;
    n_black3++;
    if(n_gap) 
    {
      n_gap3--;
    }
    n_white3--;
  }
  ret &= CHECK_EQ_U(tst,  lisp_n_root_cons(vm),    n_root);
  ret &= CHECK_EQ_U(tst,  lisp_n_black_cons(vm),   n_black);
  ret &= CHECK_EQ_U(tst,  lisp_n_gap_cons(vm),     n_gap);
  ret &= CHECK_EQ_U(tst,  lisp_n_grey_cons(vm),    n_grey);
  ret &= CHECK_EQ_U(tst,  lisp_n_white_cons(vm),   n_white);
  ret &= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 0);

  /* copy cons into root set */
  ret&= CHECK_FALSE(tst,  lisp_copy_object_as_root(vm, &root_cons, &cons));
  ret &= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 1);
  ret &= CHECK_EQ_U(tst,  lisp_n_root_cons(vm),    n_root2);
  ret &= CHECK_EQ_U(tst,  lisp_n_black_cons(vm),   n_black2);
  ret &= CHECK_EQ_U(tst,  lisp_n_gap_cons(vm),     n_gap2);
  ret &= CHECK_EQ_U(tst,  lisp_n_grey_cons(vm),    n_grey2);
  ret &= CHECK_EQ_U(tst,  lisp_n_white_cons(vm),   n_white2);
  ret &= CHECK(tst,       lisp_vm_check(tst, vm));

  /* decrease root cons count */
  ret&= CHECK_FALSE(tst,  lisp_unset_object_root(vm, &root_cons));
  ret &= CHECK_EQ_U(tst,  lisp_cons_root_refcount(vm, &cons), 0);
  ret &= CHECK_EQ_U(tst,  lisp_n_root_cons(vm),    n_root3);
  ret &= CHECK_EQ_U(tst,  lisp_n_black_cons(vm),   n_black3);
  ret &= CHECK_EQ_U(tst,  lisp_n_gap_cons(vm),     n_gap3);
  ret &= CHECK_EQ_U(tst,  lisp_n_grey_cons(vm),    n_grey3);
  ret &= CHECK_EQ_U(tst,  lisp_n_white_cons(vm),   n_white3);
  ret &= CHECK(tst,       lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ret &= CHECK_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
  return ret;
}

static void test_black2root(unit_test_t * tst)
{
  lisp_size_t offset = 0;
  ASSERT(tst, test_color2root_prepared(tst, 0, 1, 0, 0, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 1, 0, 0, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 0, 5, 0, 0, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 5, 0, 0, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 0, 1, 2, 0, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 1, 2, 0, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 0, 5, 2, 0, 0, offset + 2));
  ASSERT(tst, test_color2root_prepared(tst, 2, 5, 2, 0, 0, offset + 2));
  ASSERT(tst, test_color2root_prepared(tst, 0, 1, 2, 2, 2, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 1, 2, 2, 2, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 0, 5, 2, 2, 2, offset + 2));
  ASSERT(tst, test_color2root_prepared(tst, 2, 5, 2, 2, 2, offset + 2));
}

static void test_grey2root(unit_test_t * tst)
{
  lisp_size_t offset = 0;
  /*                                        r  b  .  g  w  */ 
  ASSERT(tst, test_color2root_prepared(tst, 0, 0, 0, 1, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 0, 0, 1, 0, offset + 0));
  offset = 4;
  ASSERT(tst, test_color2root_prepared(tst, 0, 0, 4, 5, 0, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 0, 2, 2, 5, 0, offset + 1));
  ASSERT(tst, test_color2root_prepared(tst, 0, 2, 2, 5, 0, offset + 2));
  ASSERT(tst, test_color2root_prepared(tst, 1, 2, 2, 5, 5, offset + 2));
}

static void test_white2root(unit_test_t * tst)
{
  lisp_size_t offset = 0;
  /*                                        r  b  .  g  w  */ 
  ASSERT(tst, test_color2root_prepared(tst, 0, 0, 0, 0, 1, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 0, 0, 0, 1, offset + 0));
  offset = 4;
  ASSERT(tst, test_color2root_prepared(tst, 0, 0, 4, 0, 5, offset + 0));
  ASSERT(tst, test_color2root_prepared(tst, 2, 2, 2, 0, 5, offset + 1));
  ASSERT(tst, test_color2root_prepared(tst, 2, 2, 1, 1, 5, offset + 2));
}

void test_cons(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "cons");
  TEST(suite, test_cons_set_up);
  TEST(suite, test_get_different_root_cons);
  TEST(suite, test_get_different_cons);
  TEST(suite, test_make_cons_root);
  TEST(suite, test_make_cons);
  TEST(suite, test_copy_cons);
  TEST(suite, test_black2root);
  TEST(suite, test_grey2root);
  TEST(suite, test_white2root);
}
