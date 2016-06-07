#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include "core/lisp_eval.h" 
#include "core/lisp_symbol.h"
#include "lisp_vm_check.h"

static void test_lambda_compile_atom(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       lambda;
  lisp_cell_t       expr;
  lisp_make_integer(&expr, 1);
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &expr), LISP_OK);
  ASSERT(tst,      LISP_IS_LAMBDA(&lambda));
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&lambda, lisp_lambda_t),
				    &lisp_nil), LISP_OK);
  
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(env->values));
  lisp_unset_object(vm, &lambda);
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
  ASSERT_EQ_I(tst, lisp_register_object_type(vm,
					     "TEST",
					     lisp_test_object_destructor,
					     &id), LISP_OK);
  flags = 0;
  ASSERT_EQ_I(tst, lisp_make_test_object(&expr, &flags, id), LISP_OK);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&expr), 1u);
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &expr), LISP_OK);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&expr), 2u);
  lisp_unset_object(vm, &expr);
  ASSERT(tst,      LISP_IS_LAMBDA(&lambda));
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&lambda, lisp_lambda_t),
				    &lisp_nil), LISP_OK);
  
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT_EQ_I(tst, env->values->type_id, id);
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
  lisp_make_symbol(vm, &symb_a, "a");
  ASSERT_EQ_I(tst, lisp_lambda_compile(env, &lambda, &symb_a), 
	      LISP_OK);
  /* @todo error undefined */
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&lambda, lisp_lambda_t),
				    &lisp_nil), LISP_OK);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  /*@todo exception */
  //ASSERT(tst, lisp_eq_object(env->values, &symb_a));
  lisp_unset_object(vm, &symb_a);
  lisp_unset_object(vm, &lambda);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_lambda(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "lambda");
  TEST(suite, test_lambda_compile_atom);
  TEST(suite, test_lambda_compile_atom_object);
  TEST(suite, test_lambda_compile_nil);
  TEST(suite, test_lambda_compile_symbol);
}
