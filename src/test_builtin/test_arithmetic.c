#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "builtin/lisp_arithmetic.h"
#include "core/lisp_eval.h"

static void test_plus(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t stack[10];
  lisp_make_integer(&stack[0], 0);
  ASSERT_EQ_I(tst, lisp_builtin_plus(env, &stack[0]), LISP_OK);
  ASSERT(tst, LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 0);

  lisp_make_integer(&stack[0], 1);
  lisp_make_integer(&stack[1], 1);
  ASSERT_EQ_I(tst, lisp_builtin_plus(env, &stack[0]), LISP_OK);
  ASSERT(tst, LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 1);

  lisp_make_integer(&stack[0], 3);
  lisp_make_integer(&stack[1], 1);
  lisp_make_integer(&stack[2], 2);
  lisp_make_integer(&stack[3], 3);
  ASSERT_EQ_I(tst, lisp_builtin_plus(env, &stack[0]), LISP_OK);
  ASSERT(tst, LISP_IS_INTEGER(env->values));
  ASSERT_EQ_I(tst, env->values->data.integer, 6);

  lisp_make_integer(&stack[0], 3);
  lisp_make_integer(&stack[1], 1);
  stack[2] = lisp_nil;
  lisp_make_integer(&stack[3], 3);
  ASSERT_EQ_I(tst, lisp_builtin_plus(env, &stack[0]), LISP_TYPE_ERROR);
  /*@todo test if target is proper exception */
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_builtin_arithmetic(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "builtin_arithmetic");
  TEST(suite, test_plus);
}
