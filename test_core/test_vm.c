#include "util/unit_test.h"
#include "core/lisp_vm.h" 
#include "util/xmalloc.h"
#include "util/hash_table.h"
#include "lisp_vm_check.h"
#include <stdio.h>

typedef struct test_object_t
{
  int * flags;
} test_object_t;

static void test_destructor(lisp_vm_t * vm, void * ptr)
{
  FREE_OBJECT(ptr);
}

static int make_test_object(lisp_cell_t   * target, 
			    int           * flags,
			    lisp_type_id_t  id)
{
  test_object_t * obj = MALLOC_OBJECT(sizeof(test_object_t), 1);
  if(obj)
  {
    target->type_id = id;
    target->data.ptr = obj;
    return LISP_OK;
  }
  else 
  {
    return LISP_ALLOC_ERROR;
  }
}

static void test_alloc_object(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  void * obj = MALLOC_OBJECT(3,1);
  FREE_OBJECT(obj);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_alloc_object_fail(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  memcheck_expected_alloc(memcheck, 0);
  void * obj = MALLOC_OBJECT(3,1);
  ASSERT_EQ_PTR(tst, obj, NULL);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_alloc_vm_fail(unit_test_t * tst)
{
  /* Test if all malloc failures are captured in lisp_create_vm
   * expected allocs to initialize the vm
   */
  size_t expected_mallocs = 0;
  memchecker_t * memcheck;
  lisp_vm_t    * vm;
  size_t         i,j;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  expected_mallocs = memcheck->n_chunks;
  ASSERT_NEQ_PTR(tst, vm, NULL);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
  for(i = 0; i < expected_mallocs; i++) 
  {
    memcheck = memcheck_begin(0);
    for(j = 0; j < i; j++) 
    {
      memcheck_expected_alloc(memcheck, 1);
    }
    memcheck_expected_alloc(memcheck, 0);
    vm = lisp_create_vm(&lisp_vm_default_param);
    ASSERT_EQ_PTR(tst, vm, NULL);
    if(i + 1 == expected_mallocs) 
    {
      ASSERT_EQ_PTR(tst, memcheck->next_mock, NULL);
    }
    ASSERT_MEMCHECK(tst, memcheck);
    memcheck_finalize(1);
  }
  memcheck = memcheck_begin(0);
  for(j = 0; j < expected_mallocs; j++) 
  {
    memcheck_expected_alloc(memcheck, 1);
  }
  memcheck_expected_alloc(memcheck, 0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_register_type(unit_test_t * tst)
{
  memchecker_t  * memcheck;
  lisp_vm_t     * vm;
  lisp_type_id_t  id = 0;
  lisp_cell_t     obj;
  int             flag;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      test_destructor,
					      &id));
  ASSERT(tst, (LISP_TID_OBJECT_MASK & id));
  ASSERT_FALSE(tst, make_test_object(&obj,
				     &flag,
				     id));
  ASSERT(tst, LISP_IS_OBJECT(&obj));
  lisp_unset_object(vm, &obj);
  ASSERT(tst, LISP_IS_NIL(&obj));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_object_without_explicit_destructor(unit_test_t * tst)
{
  lisp_cell_t    obj, copy;
  int            flags;
  memchecker_t * memcheck;
  lisp_vm_t    * vm;
  lisp_type_id_t  id = 0;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      NULL,
					      &id));
  flags = 0;
  ASSERT_FALSE(tst, make_test_object(&obj, &flags, id));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &obj));


  ASSERT_FALSE(tst, make_test_object(&obj, &flags, id));
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy, &obj));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 2);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &obj));
  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy));


  lisp_unset_object(vm, &obj);

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}


static void test_copy_object(unit_test_t * tst)
{
  lisp_cell_t    obj;
  lisp_cell_t    copy;
  int            flags;
  memchecker_t * memcheck;
  lisp_vm_t    * vm;
  lisp_type_id_t  id = 0;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      test_destructor,
					      &id));
  flags = 0;
  ASSERT_FALSE(tst, make_test_object(&obj, &flags, id));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 1);
  ASSERT_FALSE(tst, lisp_copy_object(vm, &copy, &obj));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 2);
  ASSERT(tst, LISP_REFCOUNT(&copy) == 2);
  ASSERT_EQ_PTR(tst, obj.data.ptr, copy.data.ptr);
  lisp_unset_object(vm, &obj);
  ASSERT(tst, LISP_REFCOUNT(&copy) == 1);
  lisp_unset_object(vm, &copy);

  ASSERT_FALSE(tst, make_test_object(&obj, &flags, id));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 1);
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy, &obj));
  ASSERT(tst, LISP_REFCOUNT(&obj) == 2);
  ASSERT(tst, LISP_REFCOUNT(&copy) == 2);
  ASSERT_EQ_PTR(tst, obj.data.ptr, copy.data.ptr);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &obj));
  ASSERT(tst, LISP_REFCOUNT(&copy) == 1);
  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_copy_n_objects(unit_test_t * tst)
{

  lisp_cell_t    from[4];
  lisp_cell_t    to[4];
  int            flags[4];
  memchecker_t * memcheck;
  lisp_vm_t    * vm;
  lisp_type_id_t  id = 0;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      test_destructor,
					      &id));
  flags[0] = flags[1] = flags[2] = flags[3] = 0;
  from[0] = lisp_nil;
  ASSERT_FALSE(tst, make_test_object(&from[1], &flags[1], id));
  ASSERT_FALSE(tst, lisp_make_cons(vm, &from[2]));
  ASSERT_FALSE(tst, lisp_make_cons_car_cdr(vm, &from[3], &from[2], &lisp_nil));

  ASSERT_FALSE(tst, lisp_copy_n_objects(vm, to, from, 4));
  ASSERT(tst, LISP_IS_ATOM(&to[0]));
  ASSERT(tst, LISP_IS_OBJECT(&to[1]));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&to[2]));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&to[3]));
  ASSERT(tst, LISP_REFCOUNT(&to[1]) == 2);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[0]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[1]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[2]));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &from[3]));
  ASSERT(tst, LISP_REFCOUNT(&to[1]) == 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &to[1]));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_copy_object_as_root_and_unset_root(unit_test_t * tst)
{
  lisp_cell_t    cons, car, cdr, copy, copy2;
  memchecker_t * memcheck;
  lisp_vm_t    * vm;

  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  car = lisp_nil;
  cdr = lisp_nil;
  ASSERT_FALSE(tst, lisp_make_cons_car_cdr(vm, &cons, &car, &cdr));
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy,  &cons));
  ASSERT_FALSE(tst, lisp_copy_object_as_root(vm, &copy2, &cons));
  ASSERT(tst, lisp_is_root_cons(vm, &copy));
  ASSERT(tst, lisp_is_root_cons(vm, &copy2));
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  ASSERT_EQ_U(tst, lisp_cons_root_refcount(vm, &cons), 2);
  ASSERT(tst, lisp_vm_check(tst, vm));

  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy));
  ASSERT_EQ_U(tst, lisp_cons_root_refcount(vm, &cons), 1);
  ASSERT(tst, lisp_is_root_cons(vm, &copy2));
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  ASSERT_FALSE(tst, lisp_unset_object_root(vm, &copy2));
  ASSERT_EQ_U(tst, lisp_cons_root_refcount(vm, &cons), 0);
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_copy_object_as_root_fail(unit_test_t * tst)
{
  lisp_cell_t    cons, car, cdr, copy;
  memchecker_t * memcheck;
  lisp_vm_t    * vm;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_NEQ_PTR(tst, vm, NULL);
  car = lisp_nil;
  cdr = lisp_nil;
  ASSERT_FALSE(tst, lisp_make_cons_car_cdr(vm, &cons, &car, &cdr));
  ASSERT_FALSE(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));
  memcheck_expected_alloc(memcheck, 0);
  ASSERT(tst, lisp_copy_object_as_root(vm, &copy, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_unset_root_fail(unit_test_t * tst)
{
  memchecker_t * memcheck;
  lisp_vm_t    * vm;
  lisp_cell_t    cons;
  memcheck = memcheck_begin(0);
  vm = lisp_create_vm(&lisp_vm_default_param);
  set_up_conses(tst, vm, 10, 0, 0, 0, 0);
  ASSERT(tst, lisp_vm_check(tst, vm));
  cons = lisp_get_root_cons(vm, 0);
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  memcheck_expected_alloc(memcheck, 0);
  ASSERT(tst, lisp_unset_object_root(vm, &cons));
  ASSERT(tst, lisp_is_root_cons(vm, &cons));
  ASSERT(tst, lisp_vm_check(tst, vm));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}



/**************************************************************/
/* @todo create mock object type and move test_create_string 
   to separate file */
static void test_create_string(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_make_string(vm, &str, "abc");
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str), "abc");
  ASSERT(        tst, LISP_IS_OBJECT(&str));
  ASSERT(        tst, LISP_REFCOUNT(&str) == 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &str));
  ASSERT(        tst, LISP_IS_NIL(&str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_sprintf(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t str;
  lisp_sprintf(vm, &str, "%d %x %s", 1,0xff, "abc");
  ASSERT(tst, LISP_IS_OBJECT(&str));
  ASSERT_EQ_CSTR(tst, lisp_c_string(&str), "1 ff abc");
  ASSERT(tst, LISP_REFCOUNT(&str) == 1);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &str));
  ASSERT(tst, LISP_IS_NIL(&str));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_copy_string(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
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
  LISP_SET(vm, &copy2, &copy1);
  ASSERT(tst, LISP_REFCOUNT(&copy1) == 2);
  ASSERT_FALSE(tst, lisp_unset_object(vm, &copy1));
  ASSERT_FALSE(tst, lisp_unset_object(vm, &copy2));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_create_symbol(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);

  lisp_cell_t symb_abc_1;
  lisp_create_symbol(vm, &symb_abc_1, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_1), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 1);

  lisp_cell_t symb_def_1;
  lisp_create_symbol(vm, &symb_def_1, "def");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_def_1), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 2);

  lisp_cell_t symb_abc_2;
  lisp_create_symbol(vm, &symb_abc_2, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_1), 2);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_2), 2);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 2);
  ASSERT_EQ_PTR(tst, symb_abc_1.data.ptr, symb_abc_2.data.ptr);

  LISP_UNSET(vm, &symb_abc_1);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_2), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 2);
  LISP_UNSET(vm, &symb_abc_2);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 1);
  LISP_UNSET(vm, &symb_def_1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 0);
  
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

void test_vm(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "vm");
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

  /* @todo move the following */
  TEST(suite, test_create_string);
  TEST(suite, test_sprintf);
  TEST(suite, test_copy_string);
  TEST(suite, test_create_symbol);
}
