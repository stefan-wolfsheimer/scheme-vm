#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/mock.h"
#include "core/lisp_vm.h" 
#include "core/lisp_eval.h" 
#include "core/lisp_symbol.h"
#include "core/lisp_exception.h"
#include "core/lisp_lambda.h"
#include "lisp_vm_check.h"


static int lisp_compile_eval(struct lisp_eval_env_t   * env,
                             struct lisp_cell_t       * lambda,
                             int(*creator)(struct lisp_eval_env_t * env,
                                           struct lisp_cell_t * lambda))
{
  int ret = creator(env, lambda);
  if(ret != LISP_OK) 
  {
    return ret;
  }
  REQUIRE(LISP_IS_LAMBDA(lambda));
  ret = lisp_eval_lambda(env, 
                         LISP_AS(lambda, lisp_lambda_t),
                         0);
  return ret;
}

static int lisp_compile_atom(struct lisp_eval_env_t * env,
                             struct lisp_cell_t * lambda)
{
  lisp_cell_t       expr;
  lisp_make_integer(&expr, 1);
  return lisp_lambda_compile(env, lambda, &expr);
}


static void test_lambda_mock_0_args(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t        * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t  * env = lisp_create_eval_env(vm);
  lisp_lambda_mock_t mock;
  lisp_init_lambda_mock(&mock, vm, 0);
  ASSERT_FALSE(tst,  lisp_lambda_mock_function(env, NULL, 0));
  ASSERT_EQ_U(tst,   env->n_values, 0u);
  lisp_free_lambda_mock(&mock);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lambda_mock_10_args(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t        * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t  * env = lisp_create_eval_env(vm);
  lisp_lambda_mock_t mock;
  lisp_init_lambda_mock(&mock, vm, 10);
  ASSERT_FALSE(tst,  lisp_lambda_mock_function(env, NULL, 0));
  ASSERT_EQ_U(tst,   env->n_values, 0u);
  lisp_free_lambda_mock(&mock);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lisp_make_builtin_lambda(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t          * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t    * env = lisp_create_eval_env(vm);
  lisp_cell_t          lambda;
  lisp_lambda_mock_t   mock;
  ASSERT_EQ_I(tst, lisp_make_builtin_lambda(vm,
                                            &lambda,
                                            0,
                                            NULL,
                                            lisp_lambda_mock_function),
              LISP_OK);
  lisp_init_lambda_mock(&mock, vm, 1);
  lisp_make_integer(&mock.values[0], 23);
  mock_register(lisp_lambda_mock_function, NULL, &mock, NULL);
  lisp_push_integer(env, 1);
  lisp_push_integer(env, 2);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env,
                                    LISP_AS(&lambda, lisp_lambda_t),
                                    2),
	      LISP_OK);

  /* test call */
  ASSERT_EQ_U(tst, mock.n_args, 2);
  ASSERT(tst, LISP_IS_INTEGER(&mock.args[0]));
  ASSERT_EQ_I(tst,  mock.args[0].data.integer, 1);
  ASSERT(tst, LISP_IS_INTEGER(&mock.args[1]));
  ASSERT_EQ_I(tst,  mock.args[1].data.integer, 2);

  /* test result */
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 23);
  

  /* retire mock */
  ASSERT_EQ_U(tst, mock_retire_all(), 0u);

  lisp_free_lambda_mock(&mock);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lambda_compile_atom(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t     lambda;
  lisp_compile_eval(env, &lambda, lisp_compile_atom);

  /* test byte code */
  /* @todo more assertions */
  ASSERT(tst,      LISP_IS_LAMBDA(&lambda));
  
  /* test result */
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(env->values));

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lambda_compile_atom_object(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_type_id_t    id = 0;
  int               flags;
  lisp_cell_t       lambda;
  lisp_cell_t       expr;
  lisp_test_object_t  * ptr;
  ASSERT_EQ_I(tst, lisp_register_object_type(vm,
					     "TEST",
					     lisp_test_object_destructor,
					     &id), LISP_OK);
  flags = 0;
  ASSERT_EQ_I(tst, lisp_make_test_object(&expr, &flags, id), LISP_OK);
  ptr = LISP_AS(&expr, lisp_test_object_t);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&expr), 1u);
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &expr), LISP_OK);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&expr), 2u);
  lisp_unset_object(vm, &expr);
  ASSERT(tst,      LISP_IS_LAMBDA(&lambda));
  ASSERT(tst,      LISP_IS_OBJECT(LISP_CAR(&lambda)));
  ASSERT(tst,      LISP_IS_OBJECT(LISP_CADDR(&lambda)));
  ASSERT_EQ_U(tst, LISP_REFCOUNT(LISP_CADDR(&lambda)), 1u);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
                                    LISP_AS(&lambda, lisp_lambda_t),
                                    0), LISP_OK);
  
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT_EQ_I(tst, env->values->type_id, id);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(env->values), 2u);
  ASSERT_EQ_PTR(tst, LISP_AS(env->values, lisp_test_object_t), ptr);
  lisp_unset_object(vm, &lambda);
  lisp_free_eval_env(env);

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lambda_compile_nil(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       lambda;
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &lisp_nil), 
	      LISP_COMPILATION_ERROR);
  /* @todo test exception */
  lisp_unset_object(vm, &lambda);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lambda_compile_symbol(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       lambda;
  lisp_cell_t       symb_a;
  lisp_cell_t       value;
  lisp_make_integer(&value, 23);
  lisp_make_symbol(vm, &symb_a, "a");
  lisp_symbol_set(vm, LISP_AS(&symb_a, lisp_symbol_t), &value);
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &symb_a), LISP_OK);
  lisp_unset_object(vm, &symb_a);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&lambda, lisp_lambda_t),
                                    0), LISP_OK);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      lisp_eq_object(env->values, &value));
  lisp_unset_object(vm, &lambda);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_lambda_compile_symbol_undefined(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       lambda;
  lisp_cell_t       symb_a;
  lisp_make_symbol(vm, &symb_a, "a");
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &symb_a), 
	      LISP_OK);
  lisp_unset_object(vm, &symb_a);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&lambda, lisp_lambda_t),
                                    0), LISP_UNDEFINED);
  lisp_unset_object(vm, &lambda);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst, LISP_IS_EXCEPTION(env->values));
  ASSERT_EQ_I(tst, LISP_AS(env->values, lisp_exception_t)->error_code,
	      LISP_UNDEFINED);
  /* @todo run continuation when ready */
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_compile_cons_builtin_arg_arg(unit_test_t * tst) 
{
  memcheck_begin();
  lisp_vm_t         * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t   * env = lisp_create_eval_env(vm);
  lisp_lambda_mock_t  mock;
  lisp_cell_t         lst[10];
  lisp_cell_t         expr;
  lisp_cell_t         lambda;
  ASSERT_EQ_I(tst, lisp_make_builtin_lambda(vm,
                                            &lst[0],
                                            0,
                                            NULL,
                                            lisp_lambda_mock_function),
              LISP_OK);
  lisp_make_integer(   &lst[1], 1);
  lisp_make_integer(   &lst[2], 2);
  lisp_make_list_root(vm, &expr, lst, 3);  

  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &expr), 
	      LISP_OK);


  lisp_init_lambda_mock(&mock, vm, 1);
  lisp_make_integer(&mock.values[0], 23);
  mock_register(lisp_lambda_mock_function, NULL, &mock, NULL);

  ASSERT_EQ_I(tst, lisp_eval_lambda(env,
                                    LISP_AS(&lambda, lisp_lambda_t),
                                    0),
	      LISP_OK);

  /* test call */
  ASSERT_EQ_U(tst, mock.n_args, 2);
  ASSERT(tst, lisp_eq_object(&mock.args[0],&lst[1]));
  ASSERT(tst, lisp_eq_object(&mock.args[1],&lst[2]));

  /* test result */
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 23);

  ASSERT_EQ_U(tst, env->stack_top, 0u);

  ASSERT_EQ_U(tst, mock_retire_all(), 0u);
  lisp_free_lambda_mock(&mock);

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_lambda(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "lambda");
  TEST(suite, test_lambda_mock_0_args);
  TEST(suite, test_lambda_mock_10_args);

  TEST(suite, test_lisp_make_builtin_lambda);
  TEST(suite, test_lambda_compile_atom);
  TEST(suite, test_lambda_compile_atom_object);
  TEST(suite, test_lambda_compile_nil);
  TEST(suite, test_lambda_compile_symbol);
  TEST(suite, test_lambda_compile_symbol_undefined);
  TEST(suite, test_compile_cons_builtin_arg_arg);
}
