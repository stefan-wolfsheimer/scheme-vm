#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h"
#include "builtin/builtin_compile.h"

static void test_create_function(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t       compile_func;
  lisp_make_func_compile(vm, &compile_func);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_builtin_compile(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "builtin_compile");
  TEST(suite, test_create_function);
}
