#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include "lisp_vm_check.h"

/* @todo test copy symbol object */
static void test_create_symbol(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);

  lisp_cell_t symb_abc_1;
  lisp_make_symbol(vm, &symb_abc_1, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_1), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 1);

  lisp_cell_t symb_def_1;
  lisp_make_symbol(vm, &symb_def_1, "def");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_def_1), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 2);
  ASSERT_FALSE(tst, lisp_eq_object(&symb_abc_1, &symb_def_1));

  lisp_cell_t symb_abc_2;
  lisp_make_symbol(vm, &symb_abc_2, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_1), 2);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_2), 2);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 2);
  ASSERT_EQ_PTR(tst, symb_abc_1.data.ptr, symb_abc_2.data.ptr);
  ASSERT_FALSE(tst, lisp_eq_object(&symb_abc_1, &symb_def_1));
  ASSERT(tst, lisp_eq_object(&symb_abc_1, &symb_abc_2));

  lisp_unset_object(vm, &symb_abc_1);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_2), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 2);
  lisp_unset_object(vm, &symb_abc_2);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 1);
  lisp_unset_object(vm, &symb_def_1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 0);
  
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_symbol_set(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t          * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t          abc;
  lisp_cell_t          obj;
  lisp_type_id_t       id = 0;
  int                  flag = TEST_OBJECT_STATE_UNINIT;
  lisp_test_object_t * obj_ptr;
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
					      &id));

  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT_EQ_U(tst,   HASH_TABLE_SIZE(&vm->symbols), 1);
  ASSERT_EQ_PTR(tst, lisp_symbol_get(vm, LISP_AS(&abc, lisp_symbol_t)), NULL);
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_UNINIT);
  ASSERT_FALSE(tst,  lisp_make_test_object(&obj, &flag, id));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_INIT);
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&obj), 1u);
  ASSERT_FALSE(tst,  lisp_symbol_set(vm, LISP_AS(&abc, lisp_symbol_t), &obj));
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&obj), 2u);
  obj_ptr = LISP_AS(&obj, lisp_test_object_t);
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &obj));
  ASSERT_EQ_PTR(tst, 
		LISP_AS(lisp_symbol_get(vm, 
					LISP_AS(&abc, lisp_symbol_t)),
			lisp_test_object_t),
		obj_ptr);
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &abc));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_INIT);
  /* still in hash because symbol is bound */
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), 1);
  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT_FALSE(tst,  lisp_symbol_set(vm, LISP_AS(&abc, lisp_symbol_t), 
				     &lisp_nil));  
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_FREE);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_symbol_unset(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t          * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t          abc;
  lisp_cell_t          obj;
  lisp_type_id_t       id = 0;
  int                  flag = TEST_OBJECT_STATE_UNINIT;
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
					      &id));

  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_UNINIT);
  ASSERT_FALSE(tst,  lisp_make_test_object(&obj, &flag, id));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_INIT);
  ASSERT_FALSE(tst,  lisp_symbol_set(vm, LISP_AS(&abc, lisp_symbol_t), &obj));
  ASSERT(tst,        lisp_eq_object(&obj, 
				    lisp_symbol_get(vm, 
						    LISP_AS(&abc, 
							    lisp_symbol_t))));
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &obj));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_INIT);
  ASSERT_FALSE(tst,  
	       lisp_symbol_unset(vm, 
				 LISP_AS(&abc, 
					 lisp_symbol_t)));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_FREE);
  ASSERT_EQ_PTR(tst, lisp_symbol_get(vm, LISP_AS(&abc, lisp_symbol_t)), NULL);
  ASSERT_EQ_U(tst,   HASH_TABLE_SIZE(&vm->symbols), 1);
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &abc));
  ASSERT_EQ_U(tst,   HASH_TABLE_SIZE(&vm->symbols), 0);

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_symbol_copy(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t abc;
  lisp_cell_t abc_copy;
  lisp_cell_t value;
  lisp_make_integer(&value, 1);
  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&abc), 1);
  ASSERT_FALSE(tst,  lisp_copy_object(vm, &abc_copy, &abc));
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&abc), 2);
  ASSERT_FALSE(tst,  lisp_symbol_set(vm, LISP_AS(&abc, lisp_symbol_t), &value));
  ASSERT(tst,        lisp_eq_object(&value, 
				    lisp_symbol_get(vm, 
						    LISP_AS(&abc, 
							    lisp_symbol_t))));

  lisp_unset_object(vm, &abc);
  lisp_unset_object(vm, &abc_copy);
  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT(tst,        lisp_eq_object(&value, 
				    lisp_symbol_get(vm, 
						    LISP_AS(&abc, 
							    lisp_symbol_t))));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();

}

void test_symbol(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "symbol");
  TEST(suite, test_create_symbol);
  TEST(suite, test_symbol_set);
  TEST(suite, test_symbol_unset);
  TEST(suite, test_symbol_copy);
}
