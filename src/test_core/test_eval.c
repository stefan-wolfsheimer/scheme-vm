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
  lisp_cell_t       expr;
  lisp_cell_t       ctarget;
  lisp_cell_t       lst[10];
  lisp_make_symbol(vm, &lst[0], "define");
  lisp_make_symbol(vm, &lst[1], "a");
  lisp_make_integer(   &lst[2], 1);
  lisp_make_list_root(vm, &expr, lst, 3);
  // (define a 1)
  ASSERT_EQ_U(tst, lisp_eval(env, &ctarget, &expr), LISP_OK);  
  ASSERT(tst, lisp_eq_object(
  			     lisp_symbol_get(vm, 
  					     LISP_AS(&lst[1], lisp_symbol_t)),
  			     &lst[2]));
  lisp_unset_object(vm, &lst[0]);
  lisp_unset_object(vm, &lst[1]);
  lisp_unset_object(vm, &lst[2]);
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
  lisp_cell_t       lst[10];
  lisp_cell_t       expr;
  lisp_cell_t       ctarget;
  lisp_make_symbol(vm, &lst[0], "+");
  lisp_make_integer(   &lst[1], 1);
  lisp_make_integer(   &lst[2], 2);

  /* ( + 1 2 ) */
  lisp_make_list_root(vm, &expr, lst, 3);
  ASSERT_EQ_U(tst, lisp_eval(env, &ctarget, &expr), LISP_OK);  
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

  TEST(suite, test_eval_atom);
  TEST(suite, test_eval_symbol);
  TEST(suite, test_eval_nil);
  TEST(suite, test_eval_cons);
 
  TEST(suite, test_eval_define_atom);
  TEST(suite, test_eval_builtin);
}
