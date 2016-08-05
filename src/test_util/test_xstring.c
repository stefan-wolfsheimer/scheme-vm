#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/xstring.h"
#include "util/mock.h"
#include <string.h>

static void test_snprintf(unit_test_t * tst) 
{
  char buff[4];
  buff[3] = 'X';
  ASSERT_EQ_U(tst, snprintf(NULL, 0, "%d", 123), 3);
  ASSERT_EQ_U(tst, snprintf(buff, 3, "%d", 123), 3);
  ASSERT_EQ_CSTR(tst, buff, "12");
  ASSERT_EQ_I(tst, buff[2], 0);
  ASSERT_EQ_I(tst, buff[3], 'X');
  ASSERT_EQ_U(tst, snprintf(buff, 4, "%d", 123), 3);
  ASSERT_EQ_I(tst, buff[3], 0);
}

static void test_strcpy_null(unit_test_t * tst)
{
  memcheck_begin();
  memcheck_expected_alloc(0);
  ASSERT_EQ_PTR(tst, alloc_strcpy(NULL), NULL);
  ASSERT_EQ_U(tst, memcheck_retire_mocks(),1)
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_strcpy(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc";
  char * cpy = alloc_strcpy(from);
  ASSERT_NEQ_PTR(tst, cpy, from);
  ASSERT_EQ_CSTR(tst, cpy, from);
  FREE(cpy);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_strcpy_fail(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc";
  memcheck_expected_alloc(0);
  char * cpy = alloc_strcpy(from);
  ASSERT_EQ_PTR(tst, cpy, NULL);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


static void test_sprintf_empty(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc";
  char * cpy = alloc_sprintf(from);
  ASSERT_NEQ_PTR(tst, cpy, NULL);
  ASSERT_NEQ_PTR(tst, cpy, from);
  ASSERT_EQ_CSTR(tst, cpy, from);
  FREE(cpy);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_sprintf(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc %d %s";
  char * cpy = alloc_sprintf(from, 1, from);
  ASSERT_NEQ_PTR(tst, cpy, NULL);
  ASSERT_NEQ_PTR(tst, cpy, from);
  ASSERT_EQ_CSTR(tst, cpy, "abc 1 abc %d %s");
  FREE(cpy);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_sprintf_fail(unit_test_t * tst)
{
  memcheck_begin();
  const char * from = "abc %d %s";
  memcheck_expected_alloc(0);
  char * cpy = alloc_sprintf(from, 1, from);
  ASSERT_EQ_PTR(tst, cpy, NULL);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_xstring(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "xstring");
  TEST(suite, test_snprintf);
  TEST(suite, test_strcpy_null);
  TEST(suite, test_strcpy);
  TEST(suite, test_strcpy_fail);
  TEST(suite, test_sprintf_empty);
  TEST(suite, test_sprintf);
  TEST(suite, test_sprintf_fail);

}
