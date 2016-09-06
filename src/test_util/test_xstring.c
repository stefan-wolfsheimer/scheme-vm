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

static void _helper_alloc_va_sprintf(unit_test_t * tst, const char * fmt, ...)
{
  memcheck_begin();
  va_list   val;
  va_start(val, fmt);
  char * ret = alloc_va_sprintf(fmt, val);
  va_end(val);
  ASSERT_EQ_CSTR(tst, ret, "abc 1 abc");
  FREE(ret);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_alloc_va_sprintf(unit_test_t * tst)
{
  _helper_alloc_va_sprintf(tst, "abc %d %s", 1, "abc");
}

static void test_alloc_join(unit_test_t * tst)
{
  const char * arr[] = { "first", "second", "", "third" };
  {
    char * str = alloc_join("", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "firstsecondthird");
    FREE(str);
  }
  {
    char * str = alloc_join(",", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "first,second,,third");
    FREE(str);
  }
  {
    char * str = alloc_join(",", arr, 0);
    ASSERT_EQ_CSTR(tst, str, "");
    FREE(str);
  }
}

static void test_alloc_join_cstr(unit_test_t * tst)
{
  const char * arr[] = { "first", "second", "", "third" };
  {
    char * str = alloc_join_cstr("", "<%s>", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "<first><second><><third>");
    FREE(str);
  }
  {
    char * str = alloc_join_cstr(",", "<%s>", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "<first>,<second>,<>,<third>");
    FREE(str);
  }
  {
    char * str = alloc_join_cstr(",", "<%s>", arr, 0);
    ASSERT_EQ_CSTR(tst, str, "");
    FREE(str);
  }
}

static void test_alloc_join_i(unit_test_t * tst)
{
  const int arr[] = { -1,2,3,4 };
  {
    char * str = alloc_join_i("", "<%d>", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "<-1><2><3><4>");
    FREE(str);
  }
  {
    char * str = alloc_join_i(",", "<%d>", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "<-1>,<2>,<3>,<4>");
    FREE(str);
  }
  {
    char * str = alloc_join_i(",", "<%d>", arr, 0);
    ASSERT_EQ_CSTR(tst, str, "");
    FREE(str);
  }
}

static void test_alloc_join_u(unit_test_t * tst)
{
  const unsigned arr[] = { 1,2,3,4 };
  {
    char * str = alloc_join_u("", "<%d>", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "<1><2><3><4>");
    FREE(str);
  }
  {
    char * str = alloc_join_u(",", "<%d>", arr, 4);
    ASSERT_EQ_CSTR(tst, str, "<1>,<2>,<3>,<4>");
    FREE(str);
  }
  {
    char * str = alloc_join_u(",", "<%d>", arr, 0);
    ASSERT_EQ_CSTR(tst, str, "");
    FREE(str);
  }
}

static void test_alloc_join_ptr(unit_test_t * tst)
{
  const void * arr[] = { "a", "b", "c", "d" };
  {
    char * str = alloc_join_ptr("", "<%p>", arr, 4);
    char * expected = alloc_sprintf("<%p><%p><%p><%p>",
                                    arr[0],
                                    arr[1],
                                    arr[2],
                                    arr[3]);
    ASSERT_EQ_CSTR(tst, str, expected);
    FREE(str);
    FREE(expected);
  }
  {
    char  * str = alloc_join_ptr(",", "<%p>", arr, 4);
    char * expected = alloc_sprintf("<%p>,<%p>,<%p>,<%p>",
                                    arr[0],
                                    arr[1],
                                    arr[2],
                                    arr[3]);
    ASSERT_EQ_CSTR(tst, str, expected);
    FREE(expected);
    FREE(str);
  }
  {
    char * str = alloc_join_ptr(",", "<%p>", arr, 0);
    ASSERT_EQ_CSTR(tst, str, "");
    FREE(str);
  }
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
  TEST(suite, test_alloc_va_sprintf);
  TEST(suite, test_sprintf_fail);
  TEST(suite, test_alloc_join);
  TEST(suite, test_alloc_join_cstr);
  TEST(suite, test_alloc_join_i);
  TEST(suite, test_alloc_join_u);
  TEST(suite, test_alloc_join_ptr);
}
