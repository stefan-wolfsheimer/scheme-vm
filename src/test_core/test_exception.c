#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_exception.h" 
#include "core/lisp_vm.h" 
#include "core/lisp_lambda.h"
#include "test_core/lisp_assertion.h"
#include "test_core/context.h"

static void test_make_exception_without_lambda(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t  * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t  exception;
  ASSERT_IS_OK(tst,
               lisp_make_exception(vm,
                                   &exception,
                                   32,
                                   NULL,
                                   42,
                                   "error %s %d ",
                                   "msg",
                                   1));

  ASSERT_EQ_I(tst,
              lisp_exception_code(&exception),
              32);

  ASSERT_EQ_CSTR(tst,
                 lisp_c_string(LISP_AS(lisp_exception_message(&exception),
                                       lisp_string_t)),
                 "error msg 1 ");

  ASSERT(tst,
         LISP_IS_NIL(lisp_exception_lambda(&exception)));

  ASSERT_EQ_I(tst,
              lisp_exception_pc(&exception),
              42);

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_make_exception(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t lambda;
  lisp_cell_t exception;

  ASSERT_IS_OK(tst,
               lisp_lambda_compile(ctx->env, &lambda,
                                   INTEGER(ctx, 1)));
  ASSERT_IS_OK(tst,
               lisp_make_exception(ctx->vm,
                                   &exception,
                                   32,
                                   LISP_AS(&lambda, lisp_lambda_t),
                                   42,
                                   "error %s %d ",
                                   "msg",
                                   1));

  ASSERT_EQ_I(tst,
              lisp_exception_code(&exception), 32);

  ASSERT_EQ_CSTR(tst,
                 lisp_c_string(LISP_AS(lisp_exception_message(&exception),
                                       lisp_string_t)),
                 "error msg 1 ");

  ASSERT(tst,
         LISP_IS_LAMBDA(lisp_exception_lambda(&exception)));

  ASSERT_EQ_PTR(tst,
                LISP_AS(lisp_exception_lambda(&exception),
                        lisp_lambda_t),
                LISP_AS(&lambda,
                        lisp_lambda_t));
  ASSERT_EQ_I(tst,
              lisp_exception_pc(&exception),
              42);

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &exception));

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &lambda));

  lisp_free_unit_context(ctx);
}

static void _helper_make_va_exception(unit_test_t * tst, const char * fmt, ...)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t lambda;
  lisp_cell_t exception;
  va_list     va;
  ASSERT_IS_OK(tst,
               lisp_lambda_compile(ctx->env, &lambda,
                                   INTEGER(ctx, 1)));

  va_start(va, fmt);
  ASSERT_IS_OK(tst,
               lisp_make_va_exception(ctx->vm,
                                      &exception,
                                      32,
                                      LISP_AS(&lambda, lisp_lambda_t),
                                      42,
                                      fmt,
                                      va));
  va_end(va);

  ASSERT_EQ_I(tst,
              lisp_exception_code(&exception), 32);

  ASSERT_EQ_CSTR(tst,
                 lisp_c_string(LISP_AS(lisp_exception_message(&exception),
                                       lisp_string_t)),
                 "error msg 1 ");

  ASSERT(tst,
         LISP_IS_LAMBDA(lisp_exception_lambda(&exception)));

  ASSERT_EQ_PTR(tst,
                LISP_AS(lisp_exception_lambda(&exception),
                        lisp_lambda_t),
                LISP_AS(&lambda,
                        lisp_lambda_t));

  ASSERT_EQ_I(tst,
              lisp_exception_pc(&exception),
              42);

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &exception));

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &lambda));

  lisp_free_unit_context(ctx);
}

static void test_make_va_exception(unit_test_t * tst)
{
  _helper_make_va_exception(tst, "error %s %d ", "msg", 1);
}

static void test_raise_exception(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t lambda;

  ASSERT_IS_OK(tst,
               lisp_lambda_compile(ctx->env, &lambda,
                                   INTEGER(ctx, 1)));
  lisp_raise_exception(ctx->env,
                       32,
                       LISP_AS(&lambda, lisp_lambda_t),
                       42,
                       "error %s %d ",
                       "msg",
                       1);

  ASSERT_EQ_I(tst,
              lisp_exception_code(&ctx->env->exception), 32);

  ASSERT_EQ_CSTR(tst,
                 lisp_c_string(LISP_AS(lisp_exception_message(&ctx->env->exception),
                                       lisp_string_t)),
                 "error msg 1 ");

  ASSERT(tst,
         LISP_IS_LAMBDA(lisp_exception_lambda(&ctx->env->exception)));

  ASSERT_EQ_PTR(tst,
                LISP_AS(lisp_exception_lambda(&ctx->env->exception),
                        lisp_lambda_t),
                LISP_AS(&lambda,
                        lisp_lambda_t));

  ASSERT_EQ_I(tst,
              lisp_exception_pc(&ctx->env->exception),
              42);

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &ctx->env->exception));

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &lambda));

  lisp_free_unit_context(ctx);
}

static void _helper_raise_va_exception(unit_test_t * tst, const char * fmt, ...)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t lambda;
  va_list     va;

  ASSERT_IS_OK(tst,
               lisp_lambda_compile(ctx->env, &lambda,
                                   INTEGER(ctx, 1)));


  va_start(va, fmt);
  lisp_raise_va_exception(ctx->env,
                          32,
                          LISP_AS(&lambda, lisp_lambda_t),
                          42,
                          fmt,
                          va);
  va_end(va);
  ASSERT_EQ_I(tst,
              lisp_exception_code(&ctx->env->exception), 32);

  ASSERT_EQ_CSTR(tst,
                 lisp_c_string(LISP_AS(lisp_exception_message(&ctx->env->exception),
                                       lisp_string_t)),
                 "error msg 1 ");

  ASSERT(tst,
         LISP_IS_LAMBDA(lisp_exception_lambda(&ctx->env->exception)));

  ASSERT_EQ_PTR(tst,
                LISP_AS(lisp_exception_lambda(&ctx->env->exception),
                        lisp_lambda_t),
                LISP_AS(&lambda,
                        lisp_lambda_t));

  ASSERT_EQ_I(tst,
              lisp_exception_pc(&ctx->env->exception),
              42);

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &ctx->env->exception));

  ASSERT_IS_OK(tst,
               lisp_unset_object(ctx->vm, &lambda));

  lisp_free_unit_context(ctx);
}

static void test_raise_va_exception(unit_test_t * tst)
{

  _helper_raise_va_exception(tst, "error %s %d ", "msg", 1);
}

void test_exception(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "exception");
  TEST(suite, test_make_exception_without_lambda);
  TEST(suite, test_make_exception);
  TEST(suite, test_make_va_exception);
  TEST(suite, test_raise_exception);
  TEST(suite, test_raise_va_exception);
  /* @todo test fatal exceptions */
}
