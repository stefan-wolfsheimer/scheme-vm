#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "builtin/builtin_arithmetic.h"
#include "core/lisp_eval.h"

/* @todo move to separate generator module */
static void make_integer_list(lisp_vm_t   * vm,
			      lisp_cell_t * cell,
			      lisp_size_t   n,
			      const int   * values)
{
  lisp_cell_t * tmp = MALLOC(sizeof(lisp_cell_t) * n);
  lisp_size_t i;
  for(i = 0; i < n; i++) 
  {
    lisp_make_integer(&tmp[i], values[i]);
  }
  lisp_make_list_root(vm, cell, tmp, n);
  FREE(tmp);
}

static void test_plus_empty(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       func_plus;
  lisp_cell_t       args = lisp_nil;
  ASSERT_EQ_I(tst, lisp_make_func_plus(vm, &func_plus), LISP_OK);
  ASSERT(tst,      LISP_IS_LAMBDA(&func_plus));
  ASSERT_EQ_I(tst, lisp_eval_lambda(env,
				    LISP_AS(&func_plus, lisp_lambda_t),
				    &args),
	      LISP_OK);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst, LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 0);   
  ASSERT_EQ_I(tst, lisp_unset_object(vm, &func_plus), LISP_OK);

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_plus_int_int(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       func_plus;
  lisp_cell_t       args;
  const int         values[2] = { 1, 2};
  make_integer_list(vm, &args, 2, values);
  ASSERT_EQ_I(tst, lisp_make_func_plus(vm, &func_plus), LISP_OK);
  ASSERT(tst,      LISP_IS_LAMBDA(&func_plus));
  ASSERT_EQ_I(tst, lisp_eval_lambda(env,
				    LISP_AS(&func_plus, lisp_lambda_t),
				    &args),
	      LISP_OK);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT(tst, LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 3);   
  ASSERT_EQ_I(tst, lisp_unset_object(vm, &func_plus), LISP_OK);

  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_builtin_arithmetic(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "builtin_arithmetic");
  TEST(suite, test_plus_empty);
  TEST(suite, test_plus_int_int);
}
