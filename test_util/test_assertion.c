#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/assertion.h"

static void test_create_assertion(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create("myfile.c", 27);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT_FALSE(tst, assertion->success);
  ASSERT_FALSE(tst, assertion->is_exception);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_invert(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_invert(assertion_create("myfile.c", 27));
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT(tst, assertion->success);
  ASSERT_FALSE(tst, assertion->is_exception);

  assertion = assertion_invert(assertion);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT_FALSE(tst, assertion->success);
  ASSERT_FALSE(tst, assertion->is_exception);

  assertion->is_exception = 1;
  assertion = assertion_invert(assertion);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  ASSERT_EQ_CSTR(tst, assertion->file, "myfile.c");
  ASSERT_EQ_I(tst, assertion->line, 27);
  ASSERT(tst, assertion->success);
  ASSERT(tst, assertion->is_exception);

  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_assertion(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "assertion");
  TEST(suite, test_create_assertion);
  TEST(suite, test_assertion_invert);
}
