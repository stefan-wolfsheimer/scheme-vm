#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_eval.h" 
#include "lisp_vm_check.h"

static void test_create_eval_env(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_create_eval_env_failure(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  memcheck_expected_alloc(0);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  ASSERT_EQ_PTR(tst, env, NULL);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_builtin_plus(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t target;
  lisp_cell_t stack[10];
  lisp_make_integer(&stack[0], 0);
  ASSERT_EQ_I(tst, lisp_builtin_plus(vm, &target, &stack[0]), LISP_OK);
  ASSERT(tst, LISP_IS_INTEGER(&target));
  ASSERT_EQ_I(tst, target.data.integer, 0);

  lisp_make_integer(&stack[0], 1);
  lisp_make_integer(&stack[1], 1);
  ASSERT_EQ_I(tst, lisp_builtin_plus(vm, &target, &stack[0]), LISP_OK);
  ASSERT(tst, LISP_IS_INTEGER(&target));
  ASSERT_EQ_I(tst, target.data.integer, 1);

  lisp_make_integer(&stack[0], 3);
  lisp_make_integer(&stack[1], 1);
  lisp_make_integer(&stack[2], 2);
  lisp_make_integer(&stack[3], 3);
  ASSERT_EQ_I(tst, lisp_builtin_plus(vm, &target, &stack[0]), LISP_OK);
  ASSERT(tst, LISP_IS_INTEGER(&target));
  ASSERT_EQ_I(tst, target.data.integer, 6);

  lisp_make_integer(&stack[0], 3);
  lisp_make_integer(&stack[1], 1);
  stack[2] = lisp_nil;
  lisp_make_integer(&stack[3], 3);
  ASSERT_EQ_I(tst, lisp_builtin_plus(vm, &target, &stack[0]), LISP_TYPE_ERROR);
  /*@todo test if target is proper exception */
  lisp_free_vm(vm);
  memcheck_end();
}

static void test_eval_atom(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t cinteger;
  lisp_cell_t cstring;
  lisp_cell_t ctarget;
  ctarget = lisp_nil;

  lisp_make_integer(&cinteger, 1);
  ASSERT_FALSE(tst, lisp_eval(env, &ctarget, &cinteger));
  ASSERT(tst, LISP_IS_ATOM(&ctarget));
  ASSERT(tst, LISP_IS_INTEGER(&ctarget));
  lisp_unset_object(vm, &ctarget);


  lisp_make_string(vm, &cstring, "abc");
  ASSERT_FALSE(tst, lisp_eval(env, &ctarget, &cstring));
  ASSERT(tst, LISP_IS_OBJECT(&ctarget));
  ASSERT(tst, LISP_IS_STRING(&ctarget));
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&ctarget),2);
  lisp_unset_object(vm, &cstring);
  lisp_unset_object(vm, &ctarget);

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_eval_symbol(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t          * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t    * env = lisp_create_eval_env(vm);
  lisp_cell_t          abc;
  lisp_cell_t          obj;
  lisp_type_id_t       id = 0;
  int                  flag = TEST_OBJECT_STATE_UNINIT;
  lisp_test_object_t * obj_ptr;
  size_t nsymb = HASH_TABLE_SIZE(&vm->symbols);
  ASSERT_FALSE(tst, lisp_register_object_type(vm,
					      "TEST",
					      lisp_test_object_destructor,
					      &id));

  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));

  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_UNINIT);
  ASSERT_FALSE(tst,  lisp_make_test_object(&obj, &flag, id));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_INIT);
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&obj), 1u);
  ASSERT_FALSE(tst,  lisp_symbol_set(vm, LISP_AS(&abc, lisp_symbol_t), &obj));
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&obj), 2u);
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &obj));
  ASSERT_FALSE(tst,  lisp_eval(env, &obj, &abc));
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(&obj), 2u);
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &abc));
  /* still in hash because symbol is bound */
  ASSERT_EQ_U(tst, HASH_TABLE_SIZE(&vm->symbols), nsymb + 1);
  
  ASSERT_FALSE(tst,  lisp_make_symbol(vm, &abc, "abc"));
  ASSERT_FALSE(tst,  
	       lisp_symbol_unset(vm, 
				 LISP_AS(&abc, 
					 lisp_symbol_t)));
  /* @todo eval undefined symbol */
  ASSERT_FALSE(tst,  lisp_unset_object(vm, &obj));
  ASSERT_EQ_I(tst,   flag, TEST_OBJECT_STATE_FREE);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_eval_nil(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t          * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t    * env = lisp_create_eval_env(vm);
  lisp_cell_t          obj;
  ASSERT_EQ_I(tst,  lisp_eval(env, &obj, &lisp_nil), LISP_EVAL_ERROR);
  ASSERT(tst, LISP_IS_EVAL_ERROR(&obj));
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_eval_cons(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t cons;
  lisp_cell_t ctarget;
  ASSERT_FALSE(tst, lisp_make_cons(vm, &cons));
  ASSERT_EQ_U(tst, lisp_eval(env, &ctarget, &cons), LISP_UNSUPPORTED);  

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}



static void test_eval_define_atom(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       cons1;
  lisp_cell_t       cons2;
  lisp_cell_t       cons3;
  lisp_cell_t       ctarget;
  lisp_cell_t       integer_value;
  lisp_cell_t       symb_a;
  lisp_cell_t       symb_define;
  
  lisp_make_integer(&integer_value, 1);
  lisp_make_symbol(vm, &symb_a, "a");
  lisp_make_symbol(vm, &symb_define, "define");
  /* @todo lisp_make_list with vararg */
  lisp_make_cons_car_cdr(vm, &cons3, &integer_value,      &lisp_nil);
  lisp_make_cons_car_cdr(vm, &cons2, &symb_a,             &cons3);
  lisp_make_cons_car_cdr(vm, &cons1, &symb_define, &cons2);
  ASSERT_EQ_U(tst, lisp_eval(env, &ctarget, &cons1), LISP_OK);  

  ASSERT(tst,  lisp_eq_object(
			      lisp_symbol_get(vm, 
					      LISP_AS(&symb_a, lisp_symbol_t)),
			      &integer_value));
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_eval_builtin(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       cons1;
  lisp_cell_t       cons2;
  lisp_cell_t       cons3;
  lisp_cell_t       ctarget;
  lisp_cell_t       integer_1;
  lisp_cell_t       integer_2;
  lisp_cell_t       symb_plus;
  /* ( + 1 2 ) */
  lisp_make_integer(&integer_1, 1);
  lisp_make_integer(&integer_2, 2);
  lisp_make_symbol(vm, &symb_plus, "+");

  lisp_make_cons_car_cdr(vm, &cons3, &integer_2, &lisp_nil);
  lisp_make_cons_car_cdr(vm, &cons2, &integer_1, &cons3);
  lisp_make_cons_car_cdr(vm, &cons1, &symb_plus, &cons2);
  ASSERT(tst, LISP_IS_SYMBOL(&LISP_AS(&cons1, lisp_cons_t)->car));
  ASSERT_EQ_U(tst, lisp_eval(env, &ctarget, &cons1), LISP_OK);  
  ASSERT(tst, LISP_IS_INTEGER(&ctarget));
  ASSERT_EQ_I(tst, ctarget.data.integer, 3);

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_eval(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "eval");
  TEST(suite, test_create_eval_env);
  TEST(suite, test_create_eval_env_failure);

  TEST(suite, test_builtin_plus);

  TEST(suite, test_eval_atom);
  TEST(suite, test_eval_symbol);
  TEST(suite, test_eval_nil);
  TEST(suite, test_eval_cons);
 
  TEST(suite, test_eval_define_atom);
  TEST(suite, test_eval_builtin);
}
