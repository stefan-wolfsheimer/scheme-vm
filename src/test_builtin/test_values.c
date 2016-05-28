#include "util/unit_test.h"
#include "core/lisp_vm.h"
#include "core/lisp_eval.h"
#include "util/xmalloc.h"
#include "builtin/lisp_values.h"

static void test_values(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm    = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env   = lisp_create_eval_env(vm);
  lisp_size_t       n     = env->max_values;
  lisp_cell_t     * stack = MALLOC(sizeof(lisp_cell_t) * ( 2 * n + 1));
  lisp_size_t       i;
  char              buff[16];  
  lisp_make_integer(&stack[0], n);
  for(i = 1; i <= n; i++) 
  {
    lisp_sprintf(vm, &stack[i], "%lu", i);
  }
  ASSERT_EQ_I(tst, lisp_builtin_values(env, stack), LISP_OK);
  for(i = 0; i <= n; i++) 
  {
    lisp_unset_object_root(vm, &stack[i]);
  }
  ASSERT_EQ_U(tst, env->n_values, n);
  for(i = 0; i < n; i++) 
  {
    ASSERT(tst, LISP_IS_STRING(&env->values[i]));
    sprintf(buff, "%lu", i+1);
    lisp_string_t * str = LISP_AS(&env->values[i], lisp_string_t);
    ASSERT_FALSE(tst, 
		 lisp_string_cmp_c_string(str,
					  buff));
  }

  /* second pass */
  n = 2 * env->max_values;
  lisp_make_integer(&stack[0], n);
  for(i = 1; i <= n; i++) 
  {
    lisp_make_integer(&stack[i], i);
  }
  ASSERT_EQ_I(tst, lisp_builtin_values(env, stack), LISP_OK);
  for(i = 0; i <= n; i++) 
  {
    lisp_unset_object_root(vm, &stack[i]);
  }
  ASSERT_EQ_U(tst, env->n_values, n);
  for(i = 0; i < n; i++) 
  {
    ASSERT(tst, LISP_IS_INTEGER(&env->values[i]));
    ASSERT_EQ_I(tst, env->values[i].data.integer, i+1);
  }
  FREE(stack);
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
