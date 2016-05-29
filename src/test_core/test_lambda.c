#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include <stdio.h>


void test_lambda(unit_context_t * ctx)
{
  unit_create_suite(ctx, "lambda");
}
