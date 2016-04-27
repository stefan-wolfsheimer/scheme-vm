#include "util/unit_test.h"
#include "core/lisp_vm.h" 
#include "util/xmalloc.h"
#include "util/hash_table.h"
#include "lisp_vm_check.h"
#include <stdio.h>

static void test_type_ids(unit_test_t * tst)
{
  //  printf("%d\n", (0x00 ^ 0xff));
  //ASSERT_FALSE(tst, LISP_IS_ATOM_TID(0x00));
}

static void test_alloc_object(unit_test_t * tst)
{
  memcheck_begin();
  void * obj = MALLOC_OBJECT(3,1);
  FREE_OBJECT(obj);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_alloc_object_fail(unit_test_t * tst)
{
  memcheck_begin();
  memcheck_expected_alloc(0);
  void * obj = MALLOC_OBJECT(3,1);
  ASSERT_EQ_PTR(tst, obj, NULL);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_alloc_vm_fail(unit_test_t * tst)
{
  /* Test if all malloc failures are captured in lisp_create_vm
   * expected allocs to initialize the vm
   */
  size_t expected_mallocs = 0;
  lisp_vm_t    * vm;
  size_t         i,j;
  {
    memchecker_t * memcheck = memcheck_begin();
    vm = lisp_create_vm(&lisp_vm_default_param);
    expected_mallocs = memcheck->n_chunks;
    ASSERT_NEQ_PTR(tst, vm, NULL);
    lisp_free_vm(vm);
    ASSERT_MEMCHECK(tst);
    memcheck_end();
  }

  for(i = 0; i < expected_mallocs; i++) 
  {
    memchecker_t * memcheck = memcheck_begin();
    for(j = 0; j < i; j++) 
    {
      memcheck_expected_alloc(1);
    }
    memcheck_expected_alloc(0);
    vm = lisp_create_vm(&lisp_vm_default_param);
    ASSERT_EQ_PTR(tst, vm, NULL);
    if(i + 1 == expected_mallocs) 
    {
      ASSERT_EQ_PTR(tst, memcheck->next_mock, NULL);
    }
    ASSERT_MEMCHECK(tst);
    memcheck_end();
  }
  {
    memcheck_begin();
    for(j = 0; j < expected_mallocs; j++) 
    {
      memcheck_expected_alloc(1);
    }
    memcheck_expected_alloc(0);
    vm = lisp_create_vm(&lisp_vm_default_param);
    ASSERT_NEQ_PTR(tst, vm, NULL);
    lisp_free_vm(vm);
    ASSERT_MEMCHECK(tst);
    memcheck_end();
  }
}

static void test_register_type(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t     * vm;
  lisp_type_id_t  id = 0;
  lisp_cell_t     obj;
  int             flag;

  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
					      &id));
  ASSERT(tst,       (LISP_TID_OBJECT_MASK & id));
  ASSERT_FALSE(tst, lisp_make_test_object(&obj,
					  &flag,
					  id));
  ASSERT(tst, LISP_IS_OBJECT(&obj));
  lisp_unset_object(vm, &obj);
  ASSERT(tst, LISP_IS_NIL(&obj));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_object_without_explicit_destructor(unit_test_t * tst)
{
  memcheck_begin();
  lisp_cell_t    obj, copy;
  int            flags;
  lisp_vm_t    * vm;
  lisp_type_id_t  id = 0;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      NULL,
					      &id));
  flags = 0;
  ASSERT_FALSE(tst, lisp_make_test_object(&obj, &flags, id));
  ASSERT_EQ_U(tst,  LISP_REFCOUNT(&obj), 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &obj));
    

  ASSERT_FALSE(tst, lisp_make_test_object(&obj, &flags, id));
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy, &obj));
  ASSERT_EQ_U(tst,  LISP_REFCOUNT(&obj), 2u);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &obj));
  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy));
  lisp_unset_object(vm, &obj);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_copy_object(unit_test_t * tst)
{
  memcheck_begin();
  lisp_cell_t    obj;
  lisp_cell_t    copy;
  int            flags;
  lisp_vm_t    * vm;
  lisp_type_id_t  id = 0;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
					      &id));
  flags = 0;
  ASSERT_FALSE(tst, lisp_make_test_object(&obj, &flags, id));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 1);
  ASSERT_FALSE(tst, lisp_copy_object(vm, &copy, &obj));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 2);
  ASSERT(tst, LISP_REFCOUNT(&copy) == 2);
  ASSERT_EQ_PTR(tst, obj.data.ptr, copy.data.ptr);
  lisp_unset_object(vm, &obj);
  ASSERT(tst, LISP_REFCOUNT(&copy) == 1);
  lisp_unset_object(vm, &copy);
    
  ASSERT_FALSE(tst, lisp_make_test_object(&obj, &flags, id));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 1);
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy, &obj));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 2);
  ASSERT(tst, LISP_REFCOUNT(&copy) == 2);
  ASSERT_EQ_PTR(tst, obj.data.ptr, copy.data.ptr);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &obj));
  ASSERT(tst, LISP_REFCOUNT(&copy) == 1);
  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy));
    
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void _test_init_objects_to_copy(unit_test_t * tst,
				       lisp_vm_t   * vm,
				       lisp_cell_t   from[],
				       int           flags[])
{
  lisp_type_id_t  id = 0;
  lisp_cell_t     car;
  lisp_cell_t     cdr;
  flags[0] = flags[1] = flags[2] = flags[3] = flags[4] = 0;
  set_up_conses(tst, vm, 0, 0, 0, 0, 10);
  CHECK_FALSE(tst, lisp_register_object_type(vm,
					     "TEST",
					     lisp_test_object_destructor,
					     &id));
  car = lisp_get_white_cons(vm, 0);
  cdr = lisp_get_white_cons(vm, 1);
  from[0] = lisp_nil;
  CHECK_FALSE(tst, lisp_make_test_object(&from[1], &flags[1], id));
  from[2] = lisp_get_white_cons(vm, 2);
  from[3] = lisp_get_white_cons(vm, 3);
  CHECK_FALSE(tst, lisp_cons_set_car_cdr(vm, 
					 from[3].data.cons, 
					 &car,
					 &cdr));
  CHECK_FALSE(tst, lisp_make_test_object(&from[4], &flags[4], id));
}

static void test_copy_n_objects(unit_test_t * tst)
{
  memcheck_begin();
  lisp_cell_t    from[5];
  lisp_cell_t    to[5];
  int            flags[5];
  lisp_vm_t    * vm;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  /* 1. init vector */
  _test_init_objects_to_copy(tst, vm, from, flags);

  /* 2. copy 5 objects */
  ASSERT_FALSE(tst, lisp_copy_n_objects(vm, to, from, 5));
  ASSERT(tst, LISP_IS_ATOM(&to[0]));
  ASSERT(tst, LISP_IS_OBJECT(&to[1]));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&to[2]));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&to[3]));
  ASSERT(tst, LISP_IS_OBJECT(&to[4]));
  ASSERT(tst, LISP_REFCOUNT(&to[1]) == 2);
  ASSERT(tst, LISP_REFCOUNT(&to[4]) == 2);
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 3. unset the from objects */
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[0]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[1]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[2]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[3]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[4]));
  ASSERT(tst, LISP_REFCOUNT(&to[1]) == 1);
  ASSERT(tst, LISP_REFCOUNT(&to[4]) == 1);
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 4. unset the to objects */
  ASSERT_FALSE(tst, lisp_unset_object(vm, &to[1]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &to[4]));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_copy_object_as_root_and_unset_root(unit_test_t * tst)
{
  memcheck_begin();
  lisp_cell_t    cons, car, cdr, copy, copy2;
  lisp_vm_t    * vm;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);

  /* 1. create non-root cons */
  car = lisp_nil;
  cdr = lisp_nil;
  ASSERT_FALSE(tst, lisp_make_cons(vm, &car));
  ASSERT_FALSE(tst, lisp_make_cons(vm, &cdr));
  ASSERT_FALSE(tst, lisp_make_cons_car_cdr(vm, &cons, &car, &cdr));
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 2. copy cons as root first time */
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy,  &cons));
  ASSERT_FALSE(tst, lisp_is_white_cons(vm, &car));
  ASSERT_FALSE(tst, lisp_is_white_cons(vm, &cdr));
  ASSERT_EQ_U(tst, lisp_root_refcount(vm, &cons), 1);
  ASSERT(tst, lisp_vm_check(tst, vm));
	       
  /* 3. copy cons as root second time */
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy2, &cons));
  ASSERT(tst, lisp_is_root_cons(vm, &copy));
  ASSERT(tst, lisp_is_root_cons(vm, &copy2));
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  ASSERT_FALSE(tst, lisp_is_white_cons(vm, &car));
  ASSERT_FALSE(tst, lisp_is_white_cons(vm, &cdr));
  ASSERT_EQ_U(tst, lisp_root_refcount(vm, &cons), 2);
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 4. unset copy */
  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy));
  ASSERT_EQ_U(tst, lisp_root_refcount(vm, &cons), 1);
  ASSERT(tst, lisp_is_root_cons(vm, &copy2));
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  ASSERT_FALSE(tst, lisp_is_white_cons(vm, &car));
  ASSERT_FALSE(tst, lisp_is_white_cons(vm, &cdr));
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 5. unset copy2 */
  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy2));
  ASSERT_EQ_U(tst, lisp_root_refcount(vm, &cons), 0);
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_copy_object_as_root_fail(unit_test_t * tst)
{
  memcheck_begin();
  lisp_cell_t    cons, car, cdr, copy;
  lisp_vm_t    * vm;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);

  /* 1. create cons */
  car = lisp_nil;
  cdr = lisp_nil;
  ASSERT_FALSE(tst, lisp_make_cons(vm, &car));
  ASSERT_FALSE(tst, lisp_make_cons(vm, &cdr));
  ASSERT_FALSE(tst, lisp_make_cons_car_cdr(vm, &cons, &car, &cdr));
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 2. copy object as root with expected failure */
  memcheck_expected_alloc(0);
  ASSERT(tst, lisp_copy_object_as_root(vm, &copy, &cons));
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_copy_n_object_as_root(unit_test_t * tst)
{
  memcheck_begin();
  lisp_cell_t    from[5];
  lisp_cell_t    to[5];
  int            flags[5];
  lisp_vm_t    * vm;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  /* 1. init vector */
  _test_init_objects_to_copy(tst, vm, from, flags);

  /* 2. copy 5 objects */
  ASSERT_FALSE(tst, lisp_copy_n_objects_as_root(vm, to, from, 5));
  ASSERT(tst, LISP_IS_ATOM(&to[0]));
  ASSERT(tst, LISP_IS_OBJECT(&to[1]));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&to[2]));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&to[3]));
  ASSERT(tst, !lisp_is_white_cons(vm, &to[3].data.cons->car));
  ASSERT(tst, !lisp_is_white_cons(vm, &to[3].data.cons->cdr));
  ASSERT(tst, LISP_IS_OBJECT(&to[4]));
  ASSERT(tst, LISP_REFCOUNT(&to[1]) == 2);
  ASSERT(tst, LISP_REFCOUNT(&to[4]) == 2);
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 3. unset the from objects */
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[0]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[1]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[2]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[3]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[4]));
  ASSERT(tst, LISP_REFCOUNT(&to[1]) == 1);
  ASSERT(tst, LISP_REFCOUNT(&to[4]) == 1);
  ASSERT(tst, lisp_vm_check(tst, vm));

  /* 4. unset the to objects */
  ASSERT_FALSE(tst, lisp_unset_object(vm, &to[1]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &to[4]));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_copy_n_object_as_root_fail(unit_test_t * tst)
{
  memcheck_begin();
  /* @todo implement the same as above but with failure mock */
  lisp_cell_t    from[5];
  lisp_cell_t    to[5];
  int            flags[5];
  lisp_vm_t    * vm;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  /* 1. init vector */
  _test_init_objects_to_copy(tst, vm, from, flags);

  memcheck_expected_alloc(0);
  /* 2. copy 5 objects */
  ASSERT(tst, lisp_copy_n_objects_as_root(vm, to, from, 5));
  ASSERT(tst, LISP_IS_NIL(&to[0]));
  ASSERT(tst, LISP_IS_NIL(&to[1]));
  ASSERT(tst, LISP_IS_NIL(&to[2]));
  ASSERT(tst, LISP_IS_NIL(&to[3]));
  ASSERT(tst, LISP_IS_NIL(&to[4]));
  ASSERT(tst, !lisp_is_white_cons(vm, &to[2]));
  ASSERT(tst, !lisp_is_white_cons(vm, &to[3]));
  ASSERT(tst, LISP_REFCOUNT(&from[1]) == 1);
  ASSERT(tst, LISP_REFCOUNT(&from[4]) == 1);

  /* 3. unset the objects */
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[1]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[4]));


  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_unset_root_fail(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t    * vm;
  lisp_cell_t    cons;
  vm = lisp_create_vm(&lisp_vm_default_param);

  /* 1. create 10 root conses */
  set_up_conses(tst, vm, 10, 0, 0, 0, 0);
  ASSERT(tst, lisp_vm_check(tst, vm));
  
  /* 2. unroot first root cons with expected allocation error */
  cons = lisp_get_root_cons(vm, 0);
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  memcheck_expected_alloc(0);
  ASSERT(tst, lisp_unset_object_root(vm, &cons));
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}



/**************************************************************/
/* @todo create mock object type and move test_create_string 
   to separate file */
static void test_create_string(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_make_string(vm, &str, "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str), "abc");
  ASSERT(        tst, LISP_IS_OBJECT(&str));
  ASSERT(        tst, LISP_REFCOUNT(&str) == 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &str));
  ASSERT(        tst, LISP_IS_NIL(&str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_sprintf(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_sprintf(vm, &str, "%d %x %s", 1,0xff, "abc");
  ASSERT(tst, LISP_IS_OBJECT(&str));
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str), "1 ff abc");
  ASSERT(tst, LISP_REFCOUNT(&str) == 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &str));
  ASSERT(tst, LISP_IS_NIL(&str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_copy_string(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str, copy1, copy2;
  lisp_make_string(vm, &str, "abc");
  ASSERT(tst, LISP_REFCOUNT(&str) == 1);
  lisp_copy_object(vm, &copy1, &str);
  ASSERT(tst, LISP_REFCOUNT(&str) == 2);
  ASSERT(tst, LISP_REFCOUNT(&copy1) == 2);
  lisp_copy_object(vm, &copy2, &copy1);
  ASSERT(tst, LISP_REFCOUNT(&str) == 3);
  ASSERT(tst, LISP_REFCOUNT(&copy1) == 3);
  ASSERT(tst, LISP_REFCOUNT(&copy2) == 3);
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str),   "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy1), "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy2), "abc");  
  ASSERT_FALSE(tst, lisp_unset_object(vm, &str));
  ASSERT(tst, LISP_IS_NIL(&str));
  ASSERT(tst, LISP_REFCOUNT(&copy1) == 2);
  ASSERT(tst, LISP_REFCOUNT(&copy2) == 2);
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy1), "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&copy2), "abc");  
  ASSERT_FALSE(tst, lisp_unset_object(vm, &copy2));
  ASSERT(tst, LISP_REFCOUNT(&copy1) == 1);
  ASSERT_FALSE(tst, lisp_copy_object(vm, &copy2, &copy1));
  ASSERT(tst, LISP_REFCOUNT(&copy1) == 2);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &copy1));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &copy2));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_vm(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "vm");
  TEST(suite, test_type_ids);
  TEST(suite, test_alloc_object);
  TEST(suite, test_alloc_object_fail);
  TEST(suite, test_alloc_vm_fail); 
  TEST(suite, test_register_type);
  TEST(suite, test_object_without_explicit_destructor);
  TEST(suite, test_copy_object);
  TEST(suite, test_copy_n_objects);
  TEST(suite, test_copy_object_as_root_and_unset_root);
  TEST(suite, test_copy_object_as_root_fail);
  TEST(suite, test_unset_root_fail);
  TEST(suite, test_copy_n_object_as_root);
  TEST(suite, test_copy_n_object_as_root_fail);


  /* @todo move the following to another file */
  TEST(suite, test_create_string);
  TEST(suite, test_sprintf);
  TEST(suite, test_copy_string);
}
