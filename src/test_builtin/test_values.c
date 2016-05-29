#include "util/unit_test.h"
#include "core/lisp_vm.h"
#include "core/lisp_eval.h"
#include "util/xmalloc.h"
#include "builtin/builtin_values.h"

/* @todo move to separate generator module */
static void make_string_list(lisp_vm_t   * vm,
			      lisp_cell_t * cell,
			      lisp_size_t   n)
{
  lisp_cell_t * tmp = MALLOC(sizeof(lisp_cell_t) * n);
  lisp_size_t i;
  for(i = 0; i < n; i++) 
  {
    lisp_sprintf(vm, &tmp[i], "%lu", i);
  }
  lisp_make_list_root(vm, cell, tmp, n);
  for(i = 0; i < n; i++) 
  {
    lisp_unset_object_root(vm, &tmp[i]);
  }
  FREE(tmp);
}


static void test_values(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm    = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env   = lisp_create_eval_env(vm);
  lisp_size_t       n     = env->max_values;
  lisp_cell_t       func_values;
  lisp_cell_t       args;
  lisp_size_t       i;
  char              buff[16];  
  ASSERT_EQ_I(tst, lisp_make_func_values(vm, &func_values), LISP_OK);
  make_string_list(vm, &args, n);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&func_values, lisp_lambda_t),
				    &args), LISP_OK);
  lisp_unset_object_root(vm, &args);
  ASSERT_EQ_U(tst, env->n_values, n);
  for(i = 0; i < n; i++) 
  {
    ASSERT(tst, LISP_IS_STRING(&env->values[i]));
    sprintf(buff, "%lu", i);
    lisp_string_t * str = LISP_AS(&env->values[i], lisp_string_t);
    ASSERT_FALSE(tst, 
		 lisp_string_cmp_c_string(str,
					  buff));
  }

  /* second pass */
  n = 2 * env->max_values;
  make_string_list(vm, &args, n);
  ASSERT_EQ_I(tst, lisp_eval_lambda(env, 
				    LISP_AS(&func_values, lisp_lambda_t),
				    &args), LISP_OK);
  lisp_unset_object_root(vm, &args);
  ASSERT_EQ_U(tst, env->n_values, n);
  for(i = 0; i < n; i++) 
  {
    ASSERT(tst, LISP_IS_STRING(&env->values[i]));
    sprintf(buff, "%lu", i);
    lisp_string_t * str = LISP_AS(&env->values[i], lisp_string_t);
    ASSERT_FALSE(tst, 
		 lisp_string_cmp_c_string(str,
					  buff));
  }
  lisp_unset_object(vm, &func_values);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_call_with_values(unit_test_t * tst)
{
  /* @todo */
}

void test_builtin_values(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "builtin_values");
  TEST(suite, test_values);
  TEST(suite, test_call_with_values);
}
