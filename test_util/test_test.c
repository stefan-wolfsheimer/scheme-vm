#include "util/unit_test.h"
#include <stdio.h>

static void test_suite_with_no_tests(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_create_suite(ctx, "suite2");
  unit_run(stdout, ctx);
  ASSERT_EQ(tst, ctx->n_suites_passed,0);
  ASSERT_EQ(tst, ctx->n_suites_failed,0);
  ASSERT_EQ(tst, ctx->n_suites_without_tests,2);
  ASSERT_EQ(tst, ctx->n_tests_passed,0);
  ASSERT_EQ(tst, ctx->n_tests_failed,0);
  ASSERT_EQ(tst, ctx->n_tests_without_assertions,0);
  ASSERT_EQ(tst, ctx->n_assertions_passed,0);
  ASSERT_EQ(tst, ctx->n_assertions_failed,0);
  unit_free_context(ctx);
}

static void _test_no_failures(unit_test_t * tst)
{
  ASSERT_EQ(tst, 1,1);
  ASSERT(tst, 1);
}

static void _test_with_failures(unit_test_t * tst)
{
  ASSERT_EQ(tst, 1,1);
  ASSERT(tst, 0);
}

static void _test_no_assertions(unit_test_t * tst)
{

}

static void test_no_failures(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_suite_t * suite = unit_create_suite(ctx, "suite2");
  TEST(suite, _test_no_failures);  
  TEST(suite, _test_no_assertions);  
  unit_run(stdout, ctx);
  ASSERT_EQ(tst, ctx->n_suites_passed,1);
  ASSERT_EQ(tst, ctx->n_suites_failed,0);
  ASSERT_EQ(tst, ctx->n_suites_without_tests,1);

  ASSERT_EQ(tst, ctx->n_tests_passed,1);
  ASSERT_EQ(tst, ctx->n_tests_failed,0);
  ASSERT_EQ(tst, ctx->n_tests_without_assertions,1);

  ASSERT_EQ(tst, ctx->n_assertions_passed,2);
  ASSERT_EQ(tst, ctx->n_assertions_failed,0);
  unit_free_context(ctx);
}


static void test_with_failures(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_create_suite(ctx, "suite1");
  unit_suite_t * suite = unit_create_suite(ctx, "suite2");
  TEST(suite, _test_no_failures);  
  TEST(suite, _test_with_failures);  
  TEST(suite, _test_no_assertions);  
  unit_run(stdout, ctx);
  ASSERT_EQ(tst, ctx->n_suites_passed,0);
  ASSERT_EQ(tst, ctx->n_suites_failed,1);
  ASSERT_EQ(tst, ctx->n_suites_without_tests,1);

  ASSERT_EQ(tst, ctx->n_tests_passed,1);
  ASSERT_EQ(tst, ctx->n_tests_failed,1);
  ASSERT_EQ(tst, ctx->n_tests_without_assertions,1);

  ASSERT_EQ(tst, ctx->n_assertions_passed,3);
  ASSERT_EQ(tst, ctx->n_assertions_failed,1);
  unit_free_context(ctx);
}


static void test_unsigned_int(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  unit_test_t * tst1   = unit_create_test(suite, "test1", test_unsigned_int);

  /* EQ == */
  ASSERT_EQ_U(tst, 1, 1);
  ASSERT(tst,       CHECK_EQ_U(tst1, 1, 1));
  ASSERT_FALSE(tst, CHECK_EQ_U(tst1, 1, 2));
  ASSERT_FALSE(tst, CHECK_EQ_U(tst1, 2, 1));

  /* NEQ != */
  ASSERT_NEQ_U(tst, 1, 2);
  ASSERT_FALSE(tst, CHECK_NEQ_U(tst1, 1, 1));
  ASSERT(tst,       CHECK_NEQ_U(tst1, 1, 2));
  ASSERT(tst,       CHECK_NEQ_U(tst1, 2, 1));


  /* GT > */
  ASSERT_GT_U(tst, 2, 1);
  ASSERT_FALSE(tst, CHECK_GT_U(tst1, 1, 1));
  ASSERT_FALSE(tst, CHECK_GT_U(tst1, 1, 2));
  ASSERT(tst,       CHECK_GT_U(tst1, 2, 1));

  /* GE >= */
  ASSERT_GE_U(tst, 2, 1);
  ASSERT_GE_U(tst, 2, 2);
  ASSERT(tst,       CHECK_GE_U(tst1, 1, 1));
  ASSERT_FALSE(tst, CHECK_GE_U(tst1, 1, 2));
  ASSERT(tst,       CHECK_GE_U(tst1, 2, 1));

     
  /* LT < */
  ASSERT_LT_U(tst, 1, 2);
  ASSERT_FALSE(tst, CHECK_LT_U(tst1, 1, 1));
  ASSERT(tst,       CHECK_LT_U(tst1, 1, 2));
  ASSERT_FALSE(tst, CHECK_LT_U(tst1, 2, 1));



  /* LE <= */
  ASSERT_LE_U(tst, 1, 2);
  ASSERT_LE_U(tst, 1, 1);
  ASSERT(tst,       CHECK_LE_U(tst1, 1, 1));
  ASSERT(tst,       CHECK_LE_U(tst1, 1, 2));
  ASSERT_FALSE(tst, CHECK_LE_U(tst1, 2, 1));

  unit_free_context(ctx);
}

static void test_ptr(unit_test_t * tst)
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  unit_test_t * tst1   = unit_create_test(suite, "test1", test_unsigned_int);
  int * v = malloc(sizeof(int)*3);

  /* EQ == */
  ASSERT_EQ_PTR(tst, v+1, v+1);
  ASSERT(tst,       CHECK_EQ_PTR(tst1, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_EQ_PTR(tst1, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_EQ_PTR(tst1, v+2, v+1));

  /* NEQ != */
  ASSERT_NEQ_PTR(tst, v+1, v+2);
  ASSERT_FALSE(tst, CHECK_NEQ_PTR(tst1, v+1, v+1));
  ASSERT(tst,       CHECK_NEQ_PTR(tst1, v+1, v+2));
  ASSERT(tst,       CHECK_NEQ_PTR(tst1, v+2, v+1));


  /* GT > */
  ASSERT_GT_PTR(tst, v+2, v+1);
  ASSERT_FALSE(tst, CHECK_GT_PTR(tst1, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_GT_PTR(tst1, v+1, v+2));
  ASSERT(tst,       CHECK_GT_PTR(tst1, v+2, v+1));

  /* GE >= */
  ASSERT_GE_PTR(tst, v+2, v+1);
  ASSERT_GE_PTR(tst, v+2, v+2);
  ASSERT(tst,       CHECK_GE_PTR(tst1, v+1, v+1));
  ASSERT_FALSE(tst, CHECK_GE_PTR(tst1, v+1, v+2));
  ASSERT(tst,       CHECK_GE_PTR(tst1, v+2, v+1));

     
  /* LT < */
  ASSERT_LT_PTR(tst, v+1, v+2);
  ASSERT_FALSE(tst, CHECK_LT_PTR(tst1, v+1, v+1));
  ASSERT(tst,       CHECK_LT_PTR(tst1, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_LT_PTR(tst1, v+2, v+1));



  /* LE <= */
  ASSERT_LE_PTR(tst, v+1, v+2);
  ASSERT_LE_PTR(tst, v+1, v+1);
  ASSERT(tst,       CHECK_LE_PTR(tst1, v+1, v+1));
  ASSERT(tst,       CHECK_LE_PTR(tst1, v+1, v+2));
  ASSERT_FALSE(tst, CHECK_LE_PTR(tst1, v+2, v+1));

  free(v);
  unit_free_context(ctx);
}


static void test_string_list(unit_test_t * tst)
{
  ASSERT_EQ_CSTR_LIST(tst, ((const char*[]){}), 0,
                           ((const char*[]){}), 0);

  ASSERT_EQ_CSTR_LIST(tst, ((const char*[]){ "abc", "def", "ghi" }), 3,
                           ((const char*[]){ "abc", "def", "ghi" }), 3);

  ASSERT_NEQ_CSTR_LIST(tst, ((const char*[]){ "abc", "xxx", "ghi" }), 3,
                            ((const char*[]){ "abc", "def", "ghi" }), 3);

  ASSERT_NEQ_CSTR_LIST(tst, ((const char*[]){ "abc", "def", "ghi" }), 3,
                            ((const char*[]){ "abc", "def" }), 2);

  ASSERT_NEQ_CSTR_LIST(tst, ((const char*[]){ "abc", "def" }), 2,
                            ((const char*[]){ "abc", "def", "ghi" }), 3);

}

void test_test(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "test");
  TEST(suite, test_suite_with_no_tests);
  TEST(suite, test_no_failures);
  TEST(suite, test_with_failures);
  TEST(suite, test_unsigned_int);
  TEST(suite, test_ptr);
  TEST(suite, test_string_list);
}
