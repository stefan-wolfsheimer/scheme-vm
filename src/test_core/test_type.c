#include "util/unit_test.h"
#include "core/lisp_type.h" 
#include "util/xmalloc.h"
//#include "util/hash_table.h"
//#include "lisp_vm_check.h"
#include <stdio.h>

static void test_lisp_error_message(unit_test_t * tst)
{
  memcheck_begin();
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_OK),
                 "OK");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_ALLOC_ERROR),
                 "ALLOC_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_TYPE_ERROR),
                 "TYPE_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_EVAL_ERROR),
                 "EVAL_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_UNSUPPORTED),
                 "UNSUPPORTED");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_RANGE_ERROR),
                 "RANGE_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_COMPILATION_ERROR),
                 "COMPILATION_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_STACK_OVERFLOW), 
                 "STACK_OVERFLOW");
  int i;
  for(i = LISP_STACK_OVERFLOW+1; i < LISP_STACK_OVERFLOW+2; i++) 
  {
    ASSERT_EQ_CSTR(tst,
                   lisp_error_message(i), 
                   "???");
  }
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(255), 
                 "???");
  memcheck_end();
}

void test_type(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "type");
  TEST(suite, test_lisp_error_message);
}
