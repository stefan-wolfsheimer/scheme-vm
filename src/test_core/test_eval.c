#include "lisp_vm_check.h"
#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/mock.h"
#include "core/lisp_eval.h" 
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"

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
  lisp_eval_env_t * env;
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  memcheck_expected_alloc(0);
  env = lisp_create_eval_env(vm);
  ASSERT_EQ_PTR(tst, env, NULL);

  memcheck_expected_alloc(1);
  memcheck_expected_alloc(0);
  env = lisp_create_eval_env(vm);
  ASSERT_EQ_PTR(tst, env, NULL);
  
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

#if 0
static void test_eval_symbol(unit_test_t * tst) 
{
  /* ABC */
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
  ASSERT_FALSE(tst,  lisp_eval(env, &abc));
  ASSERT_EQ_U(tst,   env->n_values, 1u);
  ASSERT_EQ_U(tst,   LISP_REFCOUNT(env->values), 2u);
  ASSERT_FALSE(tst,  lisp_unset_object_root(vm, env->values));
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
  /* NIL */
  memcheck_begin();
  lisp_vm_t          * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t    * env = lisp_create_eval_env(vm);
  ASSERT_EQ_I(tst,  lisp_eval(env, &lisp_nil), LISP_EVAL_ERROR);
  ASSERT_EQ_U(tst,  env->n_values, 1u);
  ASSERT(tst,       LISP_IS_EVAL_ERROR(env->values));
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_eval_cons(unit_test_t * tst) 
{
  /* (NIL . NIL) */
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t cons;
  ASSERT_FALSE(tst, lisp_make_cons(vm, &cons));
  ASSERT_EQ_U(tst, lisp_eval(env, &cons), LISP_UNSUPPORTED);  

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_eval_builtin(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t         * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t   * env = lisp_create_eval_env(vm);
#if 0
  // @todo refactor
  lisp_lambda_mock_t  mock;
  lisp_cell_t         lst[10];
  lisp_cell_t         args;
  lisp_cell_t         lambda;
  lisp_init_lambda_mock(&mock, vm, 1);
  lisp_make_integer(&mock.values[0], 23);
  mock_register(lisp_lambda_mock_function, NULL, &mock, NULL);  
  ASSERT_FALSE(tst, lisp_make_builtin_lambda(vm,
					     &lambda,
					     2,
					     NULL, /* @todo argument symbols */
                                             0,
                                             NULL,
					     lisp_lambda_mock_function));
  lisp_make_integer(   &lst[0], 1);
  lisp_make_integer(   &lst[1], 2);
  lisp_make_list_root(vm, &args, lst, 2);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env,
				     LISP_AS(&lambda, lisp_lambda_t),
				     &args),
	      LISP_OK);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&lambda), 1u);
  lisp_unset_object_root(vm, &lambda);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 23);

  ASSERT_EQ_U(tst, mock_retire_all(), 0u);
  lisp_free_lambda_mock(&mock);
#endif
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_eval_registered_builtin(unit_test_t * tst) 
{
  /* @todo make independent of "+" with own test function */
  /* (+ 1 (+ 2 3) ) */
  memcheck_begin();
  lisp_vm_t        * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t  * env = lisp_create_eval_env(vm);
  lisp_cell_t        lst1[3], lst2[3];
  lisp_cell_t        expr1, expr2;
  lisp_lambda_mock_t mock1, mock2;
  lisp_init_lambda_mock(&mock1, vm, 1);
  lisp_make_integer(&mock1.values[0], 23);
  lisp_init_lambda_mock(&mock2, vm, 1);
  lisp_make_integer(&mock2.values[0], 42);
  ASSERT_EQ_I(tst, lisp_register_builtin_function(env,
						  "MOCK",
						  lisp_lambda_mock_function),
	      LISP_OK);

  lisp_make_symbol(vm,    &lst1[0], "MOCK");
  lisp_make_integer(      &lst1[1], 2);
  lisp_make_integer(      &lst1[2], 3);
  lisp_make_list(vm,      &expr1, lst1, 3);
  
  lisp_make_symbol(vm,    &lst2[0], "MOCK");
  lisp_make_integer(      &lst2[1], 1);
  lisp_copy_object(vm,    &lst2[2], &expr1);
  lisp_make_list_root(vm, &expr2, lst2, 3);

  mock_register(lisp_lambda_mock_function, NULL, &mock1, NULL);  
  mock_register(lisp_lambda_mock_function, NULL, &mock2, NULL);  
  ASSERT_EQ_U(tst, lisp_eval(env, &expr2), LISP_OK);  
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 42);

  ASSERT_EQ_U(tst, mock1.n_args, 2u);
  ASSERT(tst,      LISP_IS_INTEGER(&mock1.args[0]));
  ASSERT_EQ_I(tst, mock1.args[0].data.integer, 2);
  ASSERT(tst,      LISP_IS_INTEGER(&mock1.args[1]));
  ASSERT_EQ_I(tst, mock1.args[1].data.integer, 3);

  ASSERT_EQ_U(tst, mock2.n_args, 2u);
  ASSERT(tst,      LISP_IS_INTEGER(&mock2.args[0]));
  ASSERT_EQ_I(tst, mock2.args[0].data.integer, 1);
  ASSERT(tst,      LISP_IS_INTEGER(&mock2.args[1]));
  ASSERT_EQ_I(tst, mock2.args[1].data.integer, 23);

  ASSERT_EQ_U(tst, mock_retire_all(), 0u);
  lisp_free_lambda_mock(&mock1);
  lisp_free_lambda_mock(&mock2);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}
#endif 

void test_eval(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "eval");
  TEST(suite, test_create_eval_env);
  TEST(suite, test_create_eval_env_failure);
}
