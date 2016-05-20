#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "lisp_vm_check.h"
#include "core/lisp_builtin.h"

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

void test_builtin(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "eval");
  TEST(suite, test_builtin_plus);
}
