#include "test_core/lisp_assertion.h"

static void test_check_return_codes(unit_test_t * tst)
{
  unit_context_t * ctx   = unit_create_context();
  unit_suite_t   * suite = unit_create_suite(ctx, "suite1");
  unit_test_t    * tst2  = unit_create_test(suite, "check_return_codes",
                                            test_check_return_codes);

  ASSERT(tst,       CHECK_IS_OK(tst2, LISP_OK));
  ASSERT_FALSE(tst, CHECK_IS_OK(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_ALLOC_ERROR(tst2, LISP_ALLOC_ERROR));
  ASSERT_FALSE(tst, CHECK_IS_ALLOC_ERROR(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_TYPE_ERROR(tst2, LISP_TYPE_ERROR));
  ASSERT_FALSE(tst, CHECK_IS_TYPE_ERROR(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_EVAL_ERROR(tst2, LISP_EVAL_ERROR));
  ASSERT_FALSE(tst, CHECK_IS_EVAL_ERROR(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_UNSUPPORTED(tst2, LISP_UNSUPPORTED));
  ASSERT_FALSE(tst, CHECK_IS_UNSUPPORTED(tst2, LISP_OK));

  ASSERT(tst,       CHECK_IS_UNDEFINED(tst2, LISP_UNDEFINED));
  ASSERT_FALSE(tst, CHECK_IS_UNDEFINED(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_RANGE_ERROR(tst2, LISP_RANGE_ERROR));
  ASSERT_FALSE(tst, CHECK_IS_RANGE_ERROR(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_COMPILATION_ERROR(tst2, LISP_COMPILATION_ERROR));
  ASSERT_FALSE(tst, CHECK_IS_COMPILATION_ERROR(tst2, LISP_UNSUPPORTED));

  ASSERT(tst,       CHECK_IS_STACK_OVERFLOW(tst2, LISP_STACK_OVERFLOW));
  ASSERT_FALSE(tst, CHECK_IS_STACK_OVERFLOW(tst2, LISP_UNSUPPORTED));

  unit_free_context(ctx);
}

static void test_assert_return_codes(unit_test_t * tst) 
{
  ASSERT_IS_OK(tst, LISP_OK);
  ASSERT_IS_ALLOC_ERROR(tst, LISP_ALLOC_ERROR);
  ASSERT_IS_TYPE_ERROR(tst, LISP_TYPE_ERROR);
  ASSERT_IS_EVAL_ERROR(tst, LISP_EVAL_ERROR);
  ASSERT_IS_UNSUPPORTED(tst, LISP_UNSUPPORTED);
  ASSERT_IS_UNDEFINED(tst, LISP_UNDEFINED);
  ASSERT_IS_RANGE_ERROR(tst, LISP_RANGE_ERROR);
  ASSERT_IS_COMPILATION_ERROR(tst, LISP_COMPILATION_ERROR);
  ASSERT_IS_UNDEFINED(tst, LISP_UNDEFINED);
  ASSERT_IS_STACK_OVERFLOW(tst, LISP_STACK_OVERFLOW);
}

static void test_assertion_is_ok_failure(unit_test_t * tst) 
{
  ASSERT_IS_OK(tst, LISP_UNSUPPORTED);
}

static void test_assertion_is_alloc_error_failure(unit_test_t * tst) 
{
  ASSERT_IS_ALLOC_ERROR(tst, LISP_UNSUPPORTED);
}

static void test_assertion_is_type_error_failure(unit_test_t * tst) 
{
  ASSERT_IS_TYPE_ERROR(tst, LISP_UNSUPPORTED);
}

static void test_assertion_is_eval_error_failure(unit_test_t * tst) 
{
  ASSERT_IS_EVAL_ERROR(tst, LISP_UNSUPPORTED);
}

static void test_assertion_is_unsupported_failure(unit_test_t * tst) 
{
  ASSERT_IS_UNSUPPORTED(tst, LISP_OK);
}

static void test_assertion_is_undefined_failure(unit_test_t * tst) 
{
  ASSERT_IS_UNDEFINED(tst, LISP_OK);
}

static void test_assertion_is_range_error_failure(unit_test_t * tst) 
{
  ASSERT_IS_RANGE_ERROR(tst, LISP_OK);
}

static void test_assertion_is_compilation_error_failure(unit_test_t * tst) 
{
  ASSERT_IS_COMPILATION_ERROR(tst, LISP_OK);
}

static void test_assertion_is_stack_overflow_failure(unit_test_t * tst) 
{
  ASSERT_IS_STACK_OVERFLOW(tst, LISP_OK);
}


static void test_assert_return_code_failures(unit_test_t * tst) 
{
  unit_context_t * ctx = unit_create_context();
  unit_suite_t * suite = unit_create_suite(ctx, "suite1");
  size_t n_tests = 9;
  TEST(suite, test_assertion_is_ok_failure);
  TEST(suite, test_assertion_is_alloc_error_failure);
  TEST(suite, test_assertion_is_type_error_failure);
  TEST(suite, test_assertion_is_eval_error_failure);
  TEST(suite, test_assertion_is_unsupported_failure);
  TEST(suite, test_assertion_is_undefined_failure);
  TEST(suite, test_assertion_is_range_error_failure);
  TEST(suite, test_assertion_is_compilation_error_failure);
  TEST(suite, test_assertion_is_stack_overflow_failure);

  unit_run(stdout, ctx);
  ASSERT_EQ_U(tst, ctx->n_suites_passed, 0);
  ASSERT_EQ_U(tst, ctx->n_suites_failed, 1);
  ASSERT_EQ_U(tst, ctx->n_tests_passed, 0);
  ASSERT_EQ_U(tst, ctx->n_tests_failed, n_tests);
  ASSERT_EQ_U(tst, ctx->n_tests_without_assertions, 0);
  ASSERT_EQ_U(tst, ctx->n_assertions_passed, 0);
  ASSERT_EQ_U(tst, ctx->n_assertions_failed, n_tests);
  unit_free_context(ctx);
}

void test_lisp_assertion(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "lisp_assertion");
  TEST(suite, test_check_return_codes);
  TEST(suite, test_assert_return_codes);
  TEST(suite, test_assert_return_code_failures);
}
