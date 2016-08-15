#include "lisp_vm_check.h"
#include "test_core/lisp_assertion.h"
#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include "core/lisp_symbol.h" 


/* @todo test copy symbol object */
static void test_create_symbol(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);

  lisp_cell_t symb_abc_1;
  lisp_size_t n = HASH_TABLE_SIZE(&vm->symbols);
  lisp_make_symbol(vm, &symb_abc_1, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_1), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 1);

  lisp_cell_t symb_def_1;
  lisp_make_symbol(vm, &symb_def_1, "def");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_def_1), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 2);
  ASSERT_FALSE(tst, lisp_eq_object(&symb_abc_1, &symb_def_1));

  lisp_cell_t symb_abc_2;
  lisp_make_symbol(vm, &symb_abc_2, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_1), 2);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_2), 2);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 2);
  ASSERT_EQ_PTR(tst, 
		LISP_AS(&symb_abc_1, lisp_symbol_t), 
		LISP_AS(&symb_abc_2, lisp_symbol_t));
  ASSERT_FALSE(tst, lisp_eq_object(&symb_abc_1, &symb_def_1));
  ASSERT(tst, lisp_eq_object(&symb_abc_1, &symb_abc_2));

  lisp_unset_object(vm, &symb_abc_1);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb_abc_2), 1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 2);
  lisp_unset_object(vm, &symb_abc_2);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 1);
  lisp_unset_object(vm, &symb_def_1);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n);
  
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_print_symbol(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);

  lisp_cell_t symb;
  char buff[10];
  lisp_make_symbol(ctx->vm, &symb, "abc");
  ASSERT_EQ_U(tst, lisp_object_to_c_str(ctx->vm, NULL, 0, &symb), 3);
  ASSERT_EQ_U(tst, lisp_object_to_c_str(ctx->vm, buff, 3, &symb), 3);
  ASSERT_EQ_CSTR(tst, buff, "ab");
  ASSERT_EQ_U(tst, lisp_object_to_c_str(ctx->vm, buff, 4, &symb), 3);
  ASSERT_EQ_CSTR(tst, buff, "abc");
  lisp_free_unit_context(ctx);  
}

static void test_symbol_init_closure(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_closure_t closure;
  lisp_init_closure(vm, &closure);
  ASSERT_EQ_PTR(tst, closure.next, NULL);
  ASSERT_EQ_PTR(tst, closure.prev, NULL);
  ASSERT_EQ_PTR(tst, closure.symbol, NULL);
  ASSERT_FALSE(tst,  closure.is_attached);

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();

}
static void test_symbol_init_closure_append(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t symb;
  lisp_closure_t closure[3];
  lisp_size_t n = HASH_TABLE_SIZE(&vm->symbols);
  lisp_make_symbol(vm, &symb, "abc");
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&symb), 1);
  lisp_init_closure_append(vm, &closure[0], LISP_AS(&symb, lisp_symbol_t));
  ASSERT(tst,        closure[0].is_attached);
  ASSERT_EQ_PTR(tst, closure[0].next,   NULL);
  ASSERT_EQ_PTR(tst, closure[0].prev,   NULL);
  ASSERT_EQ_PTR(tst, closure[0].symbol, LISP_AS(&symb, lisp_symbol_t));
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&symb), 2);
  ASSERT_EQ_PTR(tst, &closure[0], LISP_AS(&symb, lisp_symbol_t)->first_closure);
  ASSERT_EQ_PTR(tst, &closure[0], LISP_AS(&symb, lisp_symbol_t)->last_closure);

  lisp_init_closure_append(vm, &closure[1], LISP_AS(&symb, lisp_symbol_t));
  ASSERT(tst,        closure[1].is_attached);
  ASSERT_EQ_PTR(tst, closure[1].symbol, LISP_AS(&symb, lisp_symbol_t));
  ASSERT_EQ_PTR(tst, closure[0].next,   &closure[1]);
  ASSERT_EQ_PTR(tst, closure[0].prev,   NULL);
  ASSERT_EQ_PTR(tst, closure[1].next,   NULL);
  ASSERT_EQ_PTR(tst, closure[1].prev,   &closure[0]);
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&symb), 3);
  ASSERT_EQ_PTR(tst, &closure[0], LISP_AS(&symb, lisp_symbol_t)->first_closure);
  ASSERT_EQ_PTR(tst, &closure[1], LISP_AS(&symb, lisp_symbol_t)->last_closure);


  lisp_init_closure_append(vm, &closure[2], LISP_AS(&symb, lisp_symbol_t));
  ASSERT(tst,        closure[2].is_attached);
  ASSERT_EQ_PTR(tst, closure[2].symbol, LISP_AS(&symb, lisp_symbol_t));
  ASSERT_EQ_PTR(tst, closure[0].next,   &closure[1]);
  ASSERT_EQ_PTR(tst, closure[0].prev,   NULL);
  ASSERT_EQ_PTR(tst, closure[1].next,   &closure[2]);
  ASSERT_EQ_PTR(tst, closure[1].prev,   &closure[0]);
  ASSERT_EQ_PTR(tst, closure[2].next,   NULL);
  ASSERT_EQ_PTR(tst, closure[2].prev,   &closure[1]);
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&symb), 4);
  ASSERT_EQ_PTR(tst, &closure[0], LISP_AS(&symb, lisp_symbol_t)->first_closure);
  ASSERT_EQ_PTR(tst, &closure[2], LISP_AS(&symb, lisp_symbol_t)->last_closure);

  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 1);
  lisp_unset_object(vm, &symb);
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 1);
  
  ASSERT_IS_OK(tst, lisp_symbol_release_closure(vm, &closure[0]));
  ASSERT_IS_OK(tst, lisp_symbol_release_closure(vm, &closure[1]));
  ASSERT_IS_OK(tst, lisp_symbol_release_closure(vm, &closure[2]));
  //ASSERT_IS_OK(tst, lisp_symbol_release_closure(vm, &closure[3]));

  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 0);

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
  lisp_size_t n = HASH_TABLE_SIZE(&vm->symbols);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
                                              NULL,
					      &id));

  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT_EQ_U(tst,   HASH_TABLE_SIZE(&vm->symbols), n + 1);
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
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), n + 1);
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
  lisp_size_t n = HASH_TABLE_SIZE(&vm->symbols);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
                                              NULL,
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
  ASSERT_EQ_U(tst,   HASH_TABLE_SIZE(&vm->symbols), n + 1);
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &abc));
  ASSERT_EQ_U(tst,   HASH_TABLE_SIZE(&vm->symbols), n + 0);

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
  TEST(suite, test_print_symbol);
  TEST(suite, test_symbol_init_closure);
  TEST(suite, test_symbol_init_closure_append);
  TEST(suite, test_symbol_set);
  TEST(suite, test_symbol_unset);
  TEST(suite, test_symbol_copy);
}
