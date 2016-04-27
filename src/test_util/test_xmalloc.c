#include "util/unit_test.h"
#include "util/xmalloc.h"

static void test_memcheck_current(unit_test_t * tst)
{
  ASSERT_EQ_PTR(tst, memcheck_current(), NULL);
  memchecker_t * memcheck1 = memcheck_begin();
  ASSERT_EQ_PTR(tst, memcheck_current(), memcheck1);
  memchecker_t * memcheck2 = memcheck_begin();
  ASSERT_EQ_PTR(tst, memcheck_current(), memcheck2);
  memcheck_end();
  ASSERT_EQ_PTR(tst, memcheck_current(), memcheck1);
  memcheck_end();
  ASSERT_EQ_PTR(tst, memcheck_current(), NULL);
}

static void test_enable_without_context(unit_test_t * tst)
{
  ASSERT_FALSE(tst, memcheck_enable(1));
  ASSERT_FALSE(tst, memcheck_enable(1));
  ASSERT_FALSE(tst, memcheck_enable(1));
  ASSERT_FALSE(tst, memcheck_enable(0));
  ASSERT_FALSE(tst, memcheck_enable(0));
  ASSERT_FALSE(tst, memcheck_enable(0));
}

static void test_enable(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin();
  ASSERT(tst, memcheck->enabled);
  ASSERT(tst, memcheck_enable(1));
  ASSERT(tst, memcheck->enabled);
  ASSERT(tst, memcheck_enable(1));
  ASSERT(tst, memcheck->enabled);
  memcheck_end();
}

static void test_disable(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin();
  ASSERT(tst, memcheck->enabled);
  int old1 = memcheck_enable(0);
  ASSERT(tst, old1);
  ASSERT_FALSE(tst, memcheck->enabled);
  int old2 =  memcheck_enable(0);
  ASSERT_FALSE(tst, old2);
  ASSERT_FALSE(tst, memcheck->enabled);
  ASSERT_FALSE(tst, memcheck_enable(old2));
  ASSERT_FALSE(tst, memcheck->enabled);
  ASSERT_FALSE(tst, memcheck_enable(old1));
  ASSERT(tst, memcheck->enabled);
  ASSERT(tst, memcheck_enable(1));
  memcheck_end();
}

static void test_memcheck_register_alloc(unit_test_t * tst)
{
  memcheck_begin();
  int * ptr = malloc(sizeof(int));
  memcheck_register_alloc("myfile.c", 1, ptr);
  assertion_t * assertion = memcheck_finalize();
  memcheck_end();
  /* only one assertion */
  ASSERT_NEQ_PTR(tst, assertion, NULL);
  ASSERT_EQ_PTR(tst, assertion->next, NULL);
  unit_add_assertion(tst, assertion_invert(assertion));
  free(ptr);
}

static void test_memcheck_register_alloc_disabled(unit_test_t * tst)
{
  memcheck_begin();
  int * ptr = malloc(sizeof(int));
  int old = memcheck_enable(0);
  memcheck_register_alloc("myfile.c", 1, ptr);
  memcheck_enable(old);
  assertion_t * assertion = memcheck_finalize();
  memcheck_end();
  /* only one assertion */
  ASSERT_EQ_PTR(tst, assertion, NULL);
  free(ptr);
}


static void test_alloc_free(unit_test_t * tst)
{
  memcheck_begin();
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
  
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_double_free1(unit_test_t * tst)
{
  memcheck_begin();
  char * data1 = MALLOC(10);
  char * data2 = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data1, NULL);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  FREE(data1);
  FREE(data1);
  FREE(data2);
  FREE(data2);
  ASSERT_FALSE(tst, memcheck_end());
}

static void test_double_free2(unit_test_t * tst)
{
  memcheck_begin();
  char * data = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data, NULL);
  FREE(data);
  FREE(data);
  if(!unit_add_assertion(tst, assertion_invert(memcheck_remove_first_assertion()))) 
  {
    return;
  }
  ASSERT_MEMCHECK(tst);
  ASSERT(tst, memcheck_end());
}

static void test_not_managed1(unit_test_t * tst)
{
  memcheck_begin();
  int old = memcheck_enable(0);
  char * data1 = MALLOC(10);
  char * data2 = MALLOC(10);
  memcheck_enable(old);
  ASSERT_NEQ_PTR(tst, data1, NULL);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  FREE(data1);
  FREE(data2);
  ASSERT_FALSE(tst, memcheck_end());
}

static void test_not_managed2(unit_test_t * tst)
{
  memcheck_begin();
  int old = memcheck_enable(0);
  char * data = MALLOC(10);
  memcheck_enable(old);
  ASSERT_NEQ_PTR(tst, data, NULL);
  FREE(data);
  if(!unit_add_assertion(tst, assertion_invert(memcheck_remove_first_assertion()))) 
  {
    return;
  }

  /*char * data = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data, NULL);
  FREE(data);
  FREE(data);
  ASSERT_MEMCHECK(tst); */
  ASSERT(tst, memcheck_end());
}


static void test_alloc_expected_failure_first(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin();
  memcheck_expected_alloc(0);
  char * data1 = MALLOC(10);
  ASSERT_EQ_PTR(tst, memcheck->next_mock, NULL);
  ASSERT_EQ_PTR(tst, data1, NULL);
  char * data2 = MALLOC(10);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  memcheck_expected_alloc(0);
  char * data3 = REALLOC(data2, 20);
  ASSERT_EQ_PTR(tst, data3, NULL);
  FREE(data2);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_alloc_expected_failure_sequence(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin();
  memcheck_expected_alloc(0);
  memcheck_expected_alloc(1);
  memcheck_expected_alloc(0);
  memcheck_expected_alloc(1);
  char * data1 = MALLOC(10);
  ASSERT_EQ_PTR(tst, data1, NULL);
  ASSERT_NEQ_PTR(tst, memcheck->next_mock, NULL);
  char * data2 = REALLOC(NULL,10);
  ASSERT_NEQ_PTR(tst, data2, NULL);
  ASSERT_NEQ_PTR(tst, memcheck->next_mock, NULL);
  char * data3 = REALLOC(data2, 10);
  ASSERT_EQ_PTR(tst, data3, NULL);
  char * data4 = REALLOC(data2, 10);
  ASSERT_NEQ_PTR(tst, data4, NULL);
  ASSERT_EQ_PTR(tst, memcheck->next_mock, NULL);
  FREE(data4);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_xmalloc(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "xmalloc");
  TEST(suite, test_memcheck_current);
  TEST(suite, test_enable_without_context);
  TEST(suite, test_enable);
  TEST(suite, test_disable);
  TEST(suite, test_memcheck_register_alloc);
  TEST(suite, test_memcheck_register_alloc_disabled);
  TEST(suite, test_alloc_free);
  TEST(suite, test_double_free1);
  TEST(suite, test_double_free2);
  TEST(suite, test_not_managed1);
  TEST(suite, test_not_managed2);
  TEST(suite, test_alloc_expected_failure_first);
  TEST(suite, test_alloc_expected_failure_sequence);
}
