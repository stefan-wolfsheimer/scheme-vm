#include "util/unit_test.h"
#include "util/xmalloc.h"

static void test_alloc_free(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  char * data = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data, NULL);
  FREE(data);

  char * data2 = MALLOC(10);
  data2 = REALLOC(data2, 10);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  FREE(data2);

  char * data3 = MALLOC(10);
  data3 = REALLOC(data3, 12);
  ASSERT_NEQ_PTR(tst, data3, NULL);
  FREE(data3);
  
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}


static void test_alloc_expected_failure_first(unit_test_t * tst) 
{
  memchecker_t * memcheck;
  memcheck = memcheck_begin(0);
  memcheck_expected_alloc(memcheck, 0);
  char * data1 = MALLOC(10);
  ASSERT_EQ_PTR(tst, memcheck->next_mock, NULL);
  ASSERT_EQ_PTR(tst, data1, NULL);
  char * data2 = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  memcheck_expected_alloc(memcheck, 0);
  char * data3 = REALLOC(data2, 20);
  ASSERT_EQ_PTR(tst, data3, NULL);
  FREE(data2);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_alloc_expected_failure_sequence(unit_test_t * tst) 
{
  memchecker_t * memcheck;
  memcheck = memcheck_begin(0);
  memcheck_expected_alloc(memcheck, 0);
  memcheck_expected_alloc(memcheck, 1);
  memcheck_expected_alloc(memcheck, 0);
  memcheck_expected_alloc(memcheck, 1);
  char * data1 = MALLOC(10);
  ASSERT_EQ_PTR(tst, data1, NULL);
  ASSERT_NEQ_PTR(tst, memcheck->next_mock, NULL);
  char * data2 = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  ASSERT_NEQ_PTR(tst, memcheck->next_mock, NULL);
  char * data3 = REALLOC(data2, 10);
  ASSERT_EQ_PTR(tst, data3, NULL);
  char * data4 = REALLOC(data2, 10);
  ASSERT_NEQ_PTR(tst, data4, NULL);
  ASSERT_EQ_PTR(tst, memcheck->next_mock, NULL);
  FREE(data4);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

void test_xmalloc(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "xmalloc");
  TEST(suite, test_alloc_free);
  TEST(suite, test_alloc_expected_failure_first);
  TEST(suite, test_alloc_expected_failure_sequence);
}
