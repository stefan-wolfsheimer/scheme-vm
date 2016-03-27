#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/assertion.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

/*********************************************************************
 * 
 * assertion helpers
 *
 *********************************************************************/
static int success(assertion_t * assertion, int expected)
{
  if(assertion == NULL) 
  {
    if(expected) 
    {
      return 1;
    }
    else 
    {
      return 0;
    }
  }
  else 
  {
    int ret = assertion->success;
    assertion_free(assertion);
    if(expected) 
    {
      return ret;
    }
    else 
    {
      return !ret;
    }
  }
}

#define ASSERTS_CMP(__CMP_TYPE__, __A__, __B__,__OP__,__EX__)	\
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_##__CMP_TYPE__("f", 0,		\
						     #__A__,		\
						     #__B__,		\
						     (__A__),		\
						     (__B__),		\
						     (__OP__),		\
						     1),		\
		 (__EX__)));						\
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_##__CMP_TYPE__("f", 0,		\
						     #__A__,		\
						     #__B__,		\
						     (__A__),		\
						     (__B__),		\
						     (__OP__),		\
						     0),		\
		 (__EX__)));					      
  
#define ASSERTS_CMP_ARR(__CMP_TYPE__, __A__, __NA__,__B__,__NB__,__OP__,__EX__) \
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_arr_##__CMP_TYPE__("f", 0,	\
							 #__A__,	\
							 #__B__,	\
							 (__A__),	\
							 (__NA__),	\
							 (__B__),	\
							 (__NB__),	\
							 (__OP__),	\
							 1),		\
		 (__EX__)));						\
  ASSERT(tst,								\
	 success(							\
		 assertion_create_cmp_arr_##__CMP_TYPE__("f", 0,	\
							 #__A__,	\
							 #__B__,	\
							 (__A__),	\
							 (__NA__),	\
							 (__B__),	\
							 (__NB__),	\
							 (__OP__),	\
							 0),		\
		 (__EX__)));

/*********************************************************************
 * 
 * int test
 *
 *********************************************************************/
static void test_cmp_i(unit_test_t * tst)
{
  ASSERT_LT_I(tst, assertion_cmp_i(-1,  2), 0);
  ASSERT_GT_I(tst, assertion_cmp_i(2,  -1), 0);
  ASSERT_EQ_I(tst, assertion_cmp_i(-1, -1), 0);
}

static void test_assertion_create_cmp_i(unit_test_t * tst)
{
  int a = -1;
  int b =  2;
  ASSERTS_CMP(i, a, a, "==", 1);
  ASSERTS_CMP(i, a, a, "<=", 1);
  ASSERTS_CMP(i, a, a, "<",  0);
  ASSERTS_CMP(i, a, a, ">=", 1);
  ASSERTS_CMP(i, a, a, ">",  0);

  ASSERTS_CMP(i, a, b, "==", 0);
  ASSERTS_CMP(i, a, b, "<=", 1);
  ASSERTS_CMP(i, a, b, "<",  1);
  ASSERTS_CMP(i, a, b, ">=", 0);
  ASSERTS_CMP(i, a, b, ">",  0);

  ASSERTS_CMP(i, b, a, "==", 0);
  ASSERTS_CMP(i, b, a, "<=", 0);
  ASSERTS_CMP(i, b, a, "<",  0);
  ASSERTS_CMP(i, b, a, ">=", 1);
  ASSERTS_CMP(i, b, a, ">",  1);
}

/*********************************************************************
 * 
 * vecotr of int
 *
 *********************************************************************/
static void test_cmp_arr_i(unit_test_t * tst)
{
  const int a1[3] = { 1, 2, 3};
  const int a2[3] = { 1, 2, 4};
  const int a3[3] = { 4, 5, 6};

  ASSERT_LT_I(tst, assertion_cmp_arr_i(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_i(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_i(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_i(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_i(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_i(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_i(a1, 3, a1, 3), 0);
}

static void test_assertion_create_cmp_arr_i(unit_test_t * tst)
{
  const int a[3] = { 1, 2, 3};
  const int b[3] = { 4, 5, 6};
  ASSERTS_CMP_ARR(i, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(i, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(i, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(i, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(i, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(i, b, 3, a, 3, ">",  1);
}


/*********************************************************************
 * 
 * unsigned int 
 *
 *********************************************************************/
static void test_cmp_u(unit_test_t * tst)
{
  ASSERT_LT_I(tst, assertion_cmp_u(1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_u(2, 1), 0);
  ASSERT_EQ_I(tst, assertion_cmp_u(1, 1), 0);
}

static void test_assertion_create_cmp_u(unit_test_t * tst)
{
  unsigned int a =  1;
  unsigned int b =  2;
  ASSERTS_CMP(u, a, a, "==", 1);
  ASSERTS_CMP(u, a, a, "<=", 1);
  ASSERTS_CMP(u, a, a, "<",  0);
  ASSERTS_CMP(u, a, a, ">=", 1);
  ASSERTS_CMP(u, a, a, ">",  0);

  ASSERTS_CMP(u, a, b, "==", 0);
  ASSERTS_CMP(u, a, b, "<=", 1);
  ASSERTS_CMP(u, a, b, "<",  1);
  ASSERTS_CMP(u, a, b, ">=", 0);
  ASSERTS_CMP(u, a, b, ">",  0);

  ASSERTS_CMP(u, b, a, "==", 0);
  ASSERTS_CMP(u, b, a, "<=", 0);
  ASSERTS_CMP(u, b, a, "<",  0);
  ASSERTS_CMP(u, b, a, ">=", 1);
  ASSERTS_CMP(u, b, a, ">",  1);
}

/*********************************************************************
 * 
 * vector of unsigned int 
 *
 *********************************************************************/
static void test_cmp_arr_u(unit_test_t * tst)
{
  const unsigned a1[3] = { 1, 2, 3};
  const unsigned a2[3] = { 1, 2, 4};
  const unsigned a3[3] = { 4, 5, 6};

  ASSERT_LT_I(tst, assertion_cmp_arr_u(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_u(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_u(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_u(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_u(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_u(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_u(a1, 3, a1, 3), 0);
}

static void test_assertion_create_cmp_arr_u(unit_test_t * tst)
{
  const unsigned int a[3] = { 1, 2, 3};
  const unsigned int b[3] = { 4, 5, 6};

  ASSERTS_CMP_ARR(u, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(u, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(u, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(u, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(u, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(u, b, 3, a, 3, ">",  1);
}

/*********************************************************************
 * 
 * pointer
 *
 *********************************************************************/
static void test_cmp_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  ASSERT_LT_I(tst, assertion_cmp_ptr(arr + 1, arr + 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_ptr(arr + 2, arr + 1), 0);
  ASSERT_EQ_I(tst, assertion_cmp_ptr(arr + 1, arr + 1), 0);
  FREE(arr);
}

static void test_assertion_create_cmp_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a = arr + 1;
  const void * b = arr + 2;

  ASSERTS_CMP(u, a, a, "==", 1);
  ASSERTS_CMP(u, a, a, "<=", 1);
  ASSERTS_CMP(u, a, a, "<",  0);
  ASSERTS_CMP(u, a, a, ">=", 1);
  ASSERTS_CMP(u, a, a, ">",  0);

  ASSERTS_CMP(u, a, b, "==", 0);
  ASSERTS_CMP(u, a, b, "<=", 1);
  ASSERTS_CMP(u, a, b, "<",  1);
  ASSERTS_CMP(u, a, b, ">=", 0);
  ASSERTS_CMP(u, a, b, ">",  0);

  ASSERTS_CMP(u, b, a, "==", 0);
  ASSERTS_CMP(u, b, a, "<=", 0);
  ASSERTS_CMP(u, b, a, "<",  0);
  ASSERTS_CMP(u, b, a, ">=", 1);
  ASSERTS_CMP(u, b, a, ">",  1);

  FREE(arr);
}

/*********************************************************************
 * 
 * vector of pointer
 *
 *********************************************************************/
static void test_cmp_arr_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a1[3] = { arr + 1, arr + 2, arr + 3};
  const void * a2[3] = { arr + 1, arr + 2, arr + 4};
  const void * a3[3] = { arr + 4, arr + 5, arr + 6};

  ASSERT_LT_I(tst, assertion_cmp_arr_ptr(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_ptr(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_ptr(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_ptr(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_ptr(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_ptr(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_ptr(a1, 3, a1, 3), 0);

  FREE(arr);
}

static void test_assertion_create_cmp_arr_ptr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  const void * a[3] = { arr + 1, arr + 2, arr + 3};
  const void * b[3] = { arr + 4, arr + 5, arr + 6};

  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(ptr, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(ptr, b, 3, a, 3, ">",  1);

  FREE(arr);
}


/*********************************************************************
 * 
 * c-string
 *
 *********************************************************************/
static void test_cmp_cstr(unit_test_t * tst)
{
  int * arr = MALLOC(10);
  ASSERT_LT_I(tst, assertion_cmp_cstr("abc", "def"), 0);
  ASSERT_GT_I(tst, assertion_cmp_cstr("def", "abc"), 0);
  ASSERT_EQ_I(tst, assertion_cmp_cstr("abc", "abc"), 0);
  FREE(arr);
}

static void test_assertion_create_cmp_cstr(unit_test_t * tst)
{
  const char * a = "abc";
  const char * b = "def";
  ASSERTS_CMP(cstr, a, a, "==", 1);
  ASSERTS_CMP(cstr, a, a, "<=", 1);
  ASSERTS_CMP(cstr, a, a, "<",  0);
  ASSERTS_CMP(cstr, a, a, ">=", 1);
  ASSERTS_CMP(cstr, a, a, ">",  0);

  ASSERTS_CMP(cstr, a, b, "==", 0);
  ASSERTS_CMP(cstr, a, b, "<=", 1);
  ASSERTS_CMP(cstr, a, b, "<",  1);
  ASSERTS_CMP(cstr, a, b, ">=", 0);
  ASSERTS_CMP(cstr, a, b, ">",  0);

  ASSERTS_CMP(cstr, b, a, "==", 0);
  ASSERTS_CMP(cstr, b, a, "<=", 0);
  ASSERTS_CMP(cstr, b, a, "<",  0);
  ASSERTS_CMP(cstr, b, a, ">=", 1);
  ASSERTS_CMP(cstr, b, a, ">",  1);
}

/*********************************************************************
 * 
 * vector of c-string
 *
 *********************************************************************/
static void test_cmp_arr_cstr(unit_test_t * tst)
{
  const char * a1[3] = { "1", "2", "3"};
  const char * a2[3] = { "1", "2", "4"};
  const char * a3[3] = { "4", "5", "6"};

  ASSERT_LT_I(tst, assertion_cmp_arr_cstr(a1, 2, a1, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_cstr(a1, 3, a2, 3), 0);
  ASSERT_LT_I(tst, assertion_cmp_arr_cstr(a1, 3, a3, 3), 0);

  ASSERT_GT_I(tst, assertion_cmp_arr_cstr(a1, 3, a1, 2), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_cstr(a2, 3, a1, 3), 0);
  ASSERT_GT_I(tst, assertion_cmp_arr_cstr(a3, 3, a1, 3), 0);

  ASSERT_EQ_I(tst, assertion_cmp_arr_cstr(a1, 3, a1, 3), 0);
}

static void test_assertion_create_cmp_arr_cstr(unit_test_t * tst)
{
  const char * a[3] = { "1", "2", "3"};
  const char * b[3] = { "4", "5", "6"};

  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, "==", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, "<=", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, a, 3, ">",  0);

  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, "==", 0);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, "<=", 1);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, "<",  1);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, ">=", 0);
  ASSERTS_CMP_ARR(cstr, a, 3, b, 3, ">",  0);

  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, "==", 0);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, "<=", 0);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, "<",  0);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, ">=", 1);
  ASSERTS_CMP_ARR(cstr, b, 3, a, 3, ">",  1);
}

/*********************************************************************
 * 
 * print
 *
 *********************************************************************/
#ifdef HAS_FMEMOPEN
#define COLOR_RED      "\x1B[31m"
#define COLOR_NORMAL   "\x1B[0m"
#define COLOR_GREEN    "\x1B[32m"
FILE *fmemopen (void *__s, size_t __len, const char *__modes);
static void test_assertion_print_exception(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_message("myfile.c", 27, "EXPECT","EXPLAIN\nLINE2", 1);
  assertion->success      = 0;
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  FILE * fp;
  fp = fmemopen(buffer, 1024, "w");
  assertion_print(fp, assertion, 0);
  fclose(fp);
  ASSERT_EQ_CSTR(tst,
		 "[FAILED]    myfile.c:27: exception <EXPECT>\n"
		 "                         EXPLAIN\n"
		 "                         LINE2\n",
		 buffer);
  FREE(buffer);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_print_assertion_cmp_i(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_cmp_i("myfile.c", 
						   27,
						   "a",
						   "b",
						   1,
						   2,
						   "==",
						   0);
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  FILE * fp;
  fp = fmemopen(buffer, 1024, "w");
  assertion_print(fp, assertion, 1);
  fclose(fp);
  ASSERT_EQ_CSTR(tst,
                 COLOR_RED"[FAILED]"COLOR_NORMAL
		   "    myfile.c:27: assertion <a==b>\n"
		   "                         with a==1\n"
		   "                              b==2\n",
		 buffer);
  FREE(buffer);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_assertion_print_assertion_cmp_u(unit_test_t * tst)
{
  memcheck_begin();
  assertion_t * assertion = assertion_create_cmp_u("myfile.c", 
						   27,
						   "a",
						   "2",
						   1,
						   2,
						   "!=",
						   0);
  char * buffer = MALLOC(1024);
  int i;
  for(i =0; i < 1024; i++) 
  {
    buffer[i] = 0;
  }
  FILE * fp;
  fp = fmemopen(buffer, 1024, "w");
  assertion_print(fp, assertion, 1);
  fclose(fp);
  ASSERT_EQ_CSTR(tst,
                 COLOR_GREEN"[PASSED]"COLOR_NORMAL
		   "    myfile.c:27: assertion <a!=2>\n"
		   "                         with a==1\n",
		 buffer);
  FREE(buffer);
  assertion_free(assertion);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


#endif

void test_assertion(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "assertion");
  TEST(suite, test_create_assertion);
  TEST(suite, test_assertion_invert);

  TEST(suite, test_cmp_i);
  TEST(suite, test_assertion_create_cmp_i);

  TEST(suite, test_cmp_arr_i);
  TEST(suite, test_assertion_create_cmp_arr_i);

  TEST(suite, test_cmp_u);
  TEST(suite, test_assertion_create_cmp_u);

  TEST(suite, test_cmp_arr_u);
  TEST(suite, test_assertion_create_cmp_arr_u);

  TEST(suite, test_cmp_ptr);
  TEST(suite, test_assertion_create_cmp_ptr);

  TEST(suite, test_cmp_arr_ptr);
  TEST(suite, test_assertion_create_cmp_arr_ptr);

  TEST(suite, test_cmp_cstr);
  TEST(suite, test_assertion_create_cmp_cstr);

  TEST(suite, test_cmp_arr_cstr);
  TEST(suite, test_assertion_create_cmp_arr_cstr);

  TEST(suite, test_assertion_print_exception);
  TEST(suite, test_assertion_print_assertion_cmp_i);
  TEST(suite, test_assertion_print_assertion_cmp_u);
}
