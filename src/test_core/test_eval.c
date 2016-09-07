#include "lisp_vm_check.h"
#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/mock.h"
#include "core/lisp_eval.h" 
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"
#include "test_core/lisp_assertion.h"

static void test_create_eval_env(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_free_unit_context(ctx);
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

static void test_push_integer(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  size_t i;
  for(i = 0; i < ctx->env->max_stack_size; i++) 
  {
    ASSERT_IS_OK(tst, lisp_push_integer(ctx->env, i));
  }
  ASSERT_IS_STACK_OVERFLOW(tst, lisp_push_integer(ctx->env, i));
  lisp_free_unit_context(ctx);
}

static void test_push_integer_alloc_error(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  memcheck_expected_alloc(0);
  ASSERT_IS_ALLOC_ERROR(tst, lisp_push_integer(ctx->env, 1));
  lisp_free_unit_context(ctx);
}


void test_eval(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "eval");
  TEST(suite, test_create_eval_env);
  TEST(suite, test_create_eval_env_failure);
  TEST(suite, test_push_integer);
  TEST(suite, test_push_integer_alloc_error);
}
