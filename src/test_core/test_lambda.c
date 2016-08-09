#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/mock.h"
#include "core/lisp_vm.h" 
#include "core/lisp_eval.h" 
#include "core/lisp_symbol.h"
#include "core/lisp_exception.h"
#include "core/lisp_lambda.h"
#include "test_core/context.h"
#include "lisp_vm_check.h"


static void test_list(unit_test_t * tst)
{
 lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
 ASSERT(tst, LISP_IS_NIL(LIST(ctx, NULL)));
 ASSERT(tst, LISP_IS_NIL(NIL(ctx)));
 lisp_free_unit_context(ctx);
}

static void test_compare_asm(unit_test_t * tst)
{
 lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
 unit_add_assertion(tst, lisp_compare_asm_list(__FILE__,
                                               __LINE__,
                                               ctx, 
                                               "",
                                               NIL(ctx),
                                               NIL(ctx)));
 unit_add_assertion(tst, lisp_compare_asm_list(__FILE__,
                                               __LINE__,
                                               ctx, 
                                               "",
                                               LIST(ctx, SYMBOL(ctx, "A"), NULL),
                                               LIST(ctx, SYMBOL(ctx, "A"), NULL)));
 unit_add_assertion(tst, assertion_invert(lisp_compare_asm_list(__FILE__,
                                                                __LINE__,
                                                                ctx, 
                                                                "",
                                                                LIST(ctx, SYMBOL(ctx, "A"), NULL),
                                                                LIST(ctx, SYMBOL(ctx, "B"), NULL))));
 unit_add_assertion(tst, assertion_invert(lisp_compare_asm_list(__FILE__,
                                                                __LINE__,
                                                                ctx, 
                                                                "",
                                                                NIL(ctx),
                                                                LIST(ctx, SYMBOL(ctx, "B"), NULL))));
 unit_add_assertion(tst, assertion_invert(lisp_compare_asm_list(__FILE__,
                                                                __LINE__,
                                                                ctx, 
                                                                "",
                                                                LIST(ctx, SYMBOL(ctx, "A"), NULL),
                                                                NIL(ctx))));
 lisp_free_unit_context(ctx);
}



static void test_lambda_mock_0_args(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_lambda_mock_t mock;
  lisp_init_lambda_mock(&mock, ctx->vm, 0);
  ASSERT_FALSE(tst,  lisp_lambda_mock_function(ctx->env, NULL, 0));
  ASSERT_EQ_U(tst,   ctx->env->n_values, 0u);
  lisp_free_lambda_mock(&mock);
  lisp_free_unit_context(ctx);
}

static void test_lambda_mock_10_args(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_lambda_mock_t mock;
  lisp_init_lambda_mock(&mock, ctx->vm, 10);
  ASSERT_FALSE(tst,  lisp_lambda_mock_function(ctx->env, NULL, 0));
  ASSERT_EQ_U(tst,   ctx->env->n_values, 0u);
  lisp_free_lambda_mock(&mock);
  lisp_free_unit_context(ctx);
}

static void test_lisp_make_builtin_lambda(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);

  lisp_cell_t          lambda;
  lisp_lambda_mock_t   mock;
  ASSERT_IS_OK(tst, lisp_make_builtin_lambda(ctx->vm,
                                             &lambda,
                                             0,
                                             NULL,
                                             lisp_lambda_mock_function));
  /* test code */
  ASSERT_DISASM(tst, ctx, &lambda, NULL,
                LIST(ctx, 
                     SYMBOL(ctx, "BUILTIN"),
                     SYMBOL(ctx, "RET"),
                     NULL));


  lisp_init_lambda_mock(&mock, ctx->vm, 1);
  lisp_make_integer(&mock.values[0], 23);
  mock_register(lisp_lambda_mock_function, NULL, &mock, NULL);
  lisp_push_integer(ctx->env, 1);
  lisp_push_integer(ctx->env, 2);
  ASSERT_IS_OK(tst, lisp_eval_lambda(ctx->env,
                                     LISP_AS(&lambda, lisp_lambda_t),
                                     2));


  /* test call */
  ASSERT_EQ_U(tst, mock.n_args, 2);
  ASSERT(tst, LISP_IS_INTEGER(&mock.args[0]));
  ASSERT_EQ_I(tst,  mock.args[0].data.integer, 1);
  ASSERT(tst, LISP_IS_INTEGER(&mock.args[1]));
  ASSERT_EQ_I(tst,  mock.args[1].data.integer, 2);

  /* test result */
  ASSERT_EQ_U(tst, ctx->env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(ctx->env->values));
  ASSERT_EQ_I(tst, ctx->env->values->data.integer, 23);
  

  /* retire mock */
  ASSERT_EQ_U(tst, mock_retire_all(), 0u);

  lisp_free_lambda_mock(&mock);
  lisp_free_unit_context(ctx);
}

static void test_lambda_compile_atom(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t     lambda;
  ASSERT_IS_OK(tst, lisp_lambda_compile(ctx->env, &lambda,
                                        INTEGER(ctx, 1)));
  ASSERT_DISASM(tst,
                ctx,
                &lambda,
                NULL,
                LIST(ctx, 
                     SYMBOL(ctx, "LDVD"),
                     SYMBOL(ctx, "RET"),
                     NULL));
  ASSERT_IS_OK(tst,
               lisp_eval_lambda(ctx->env, 
                                LISP_AS(&lambda, lisp_lambda_t),
                                0));

  /* test result */
  ASSERT_EQ_U(tst, ctx->env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(ctx->env->values));

  lisp_free_unit_context(ctx);
}

static void test_lambda_compile_atom_object(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t           lambda;
  ASSERT_IS_OK(tst, lisp_lambda_compile(ctx->env, 
                                        &lambda, 
                                        TEST_OBJECT(ctx)));
  ASSERT_DISASM(tst,
                ctx,
                &lambda,
                NULL,
                LIST(ctx, 
                     SYMBOL(ctx, "LDVD"),
                     SYMBOL(ctx, "RET"),
                     NULL));
  ASSERT_IS_OK(tst, lisp_eval_lambda(ctx->env, 
                                     LISP_AS(&lambda, lisp_lambda_t),
                                     0));
  ASSERT_EQ_U(tst, ctx->env->n_values, 1u);
  ASSERT_EQ_I(tst, ctx->env->values->type_id, ctx->test_object_id);
  ASSERT_EQ_U(tst, LISP_REFCOUNT(ctx->env->values), 3u);

  lisp_unset_object(ctx->vm, &lambda);
  lisp_free_unit_context(ctx);
}

static void test_lambda_compile_nil(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t       lambda;
  ASSERT_IS_COMPILATION_ERROR(tst, 
                              lisp_lambda_compile(ctx->env,
                                                  &lambda,
                                                  &lisp_nil));
  /* @todo test exception */
  lisp_unset_object(ctx->vm, &lambda);
  lisp_free_unit_context(ctx);
}

static void test_lambda_compile_symbol(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t           lambda;
  lambda = lisp_nil;

  lisp_symbol_set(ctx->vm, 
                  LISP_AS(SYMBOL(ctx, "a"), lisp_symbol_t),
                  INTEGER(ctx, 23));

  ASSERT_IS_OK(tst, lisp_lambda_compile(ctx->env, 
                                        &lambda,
                                        SYMBOL(ctx,"a")));

  ASSERT_DISASM(tst,
                ctx,
                &lambda,
                NULL,
                LIST(ctx, 
                     SYMBOL(ctx, "LDVR"),
                     SYMBOL(ctx, "RET"),
                     NULL));
  ASSERT_IS_OK(tst, lisp_eval_lambda(ctx->env, 
                                     LISP_AS(&lambda, lisp_lambda_t),
                                     0));

  ASSERT_EQ_U(tst,  ctx->env->n_values, 1u);
  ASSERT(tst,       lisp_eq_object(ctx->env->values, INTEGER(ctx, 23)));

  lisp_unset_object(ctx->vm, &lambda);
  lisp_free_unit_context(ctx);
}

static void test_lambda_compile_symbol_undefined(unit_test_t * tst)
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t       lambda;
  ASSERT_IS_OK(tst, lisp_lambda_compile(ctx->env,
                                        &lambda,
                                        SYMBOL(ctx, "a")));
  ASSERT_DISASM(tst,
                ctx,
                &lambda,
                NULL,
                LIST(ctx, 
                     SYMBOL(ctx, "LDVR"),
                     SYMBOL(ctx, "RET"),
                     NULL));
  ASSERT_IS_UNDEFINED(tst, lisp_eval_lambda(ctx->env, 
                                            LISP_AS(&lambda, lisp_lambda_t),
                                            0));
  lisp_unset_object(ctx->vm, &lambda);
  ASSERT_EQ_U(tst, ctx->env->n_values, 1u);
  ASSERT(tst, LISP_IS_EXCEPTION(ctx->env->values));
  ASSERT_IS_UNDEFINED(tst, LISP_AS(ctx->env->values,
                                   lisp_exception_t)->error_code);
  /* @todo: set variable and run continuation again */
  lisp_free_unit_context(ctx);
}

static void test_compile_cons_builtin_arg_arg(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_lambda_mock_t  mock;
  lisp_cell_t         lambda;
  ASSERT_IS_OK(tst,
               lisp_lambda_compile(ctx->env,
                                   &lambda, 
                                   LIST(ctx, 
                                        BUILTIN(ctx,
                                                lisp_lambda_mock_function,
                                                NULL),
                                        INTEGER(ctx, 1),
                                        INTEGER(ctx, 2),
                                        NULL)));
  ASSERT_DISASM(tst,
                ctx,
                &lambda,
                NULL,
                LIST(ctx, 
                     SYMBOL(ctx, "PUSHD"),
                     SYMBOL(ctx, "PUSHD"),
                     SYMBOL(ctx, "JP"),
                     NULL));
  lisp_init_lambda_mock(&mock, ctx->vm, 1);
  lisp_make_integer(&mock.values[0], 23);
  mock_register(lisp_lambda_mock_function, NULL, &mock, NULL);
  ASSERT_IS_OK(tst, lisp_eval_lambda(ctx->env,
                                     LISP_AS(&lambda, lisp_lambda_t),
                                     0));

  /* test call */
  ASSERT_EQ_U(tst, mock.n_args, 2);
  ASSERT(tst, lisp_eq_object(&mock.args[0], INTEGER(ctx, 1)));
  ASSERT(tst, lisp_eq_object(&mock.args[1], INTEGER(ctx, 2)));

  /* test result */
  ASSERT_EQ_U(tst, ctx->env->n_values, 1u);
  ASSERT(tst,      LISP_IS_INTEGER(ctx->env->values));
  ASSERT_EQ_I(tst, ctx->env->values->data.integer, 23);

  ASSERT_EQ_U(tst, ctx->env->stack_top, 0u);
  ASSERT_EQ_U(tst, mock_retire_all(), 0u);
  lisp_free_lambda_mock(&mock);

  lisp_free_unit_context(ctx);
}

static void test_compile_form_arg_arg(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  //lisp_lambda_mock_t  mock;
  //lisp_cell_t         lambda;

  ASSERT_IS_OK(tst, lisp_symbol_set(ctx->vm,
                                    LISP_AS(SYMBOL(ctx, "myform"), lisp_symbol_t),
                                    FORM(ctx,
                                         lisp_compile_phase1_mock,
                                         lisp_compile_phase2_mock)));
#if 0
  ASSERT_IS_OK(tst,
               lisp_lambda_compile(ctx->env,
                                   &lambda, 
                                   LIST(ctx, 
                                        SYMBOL(ctx, "myform"),
                                        INTEGER(ctx, 1),
                                        INTEGER(ctx, 2),
                                        NULL)));

  ASSERT_DISASM(tst,
                ctx,
                &lambda,
                NULL,
                LIST(ctx, NULL));
#endif
  //SYMBOL(ctx, "PUSHD"),
  //SYMBOL(ctx, "PUSHD"),
  //SYMBOL(ctx, "JP"),
  //NULL));
  //lisp_init_lambda_mock(&mock, ctx->vm, 1);
  //lisp_make_integer(&mock.values[0], 23);
  //mock_register(lisp_lambda_mock_function, NULL, &mock, NULL);
  //ASSERT_IS_OK(tst, lisp_eval_lambda(ctx->env,
  //LISP_AS(&lambda, lisp_lambda_t),
  //0));
  
  /* test call */
  //ASSERT_EQ_U(tst, mock.n_args, 2);
  //ASSERT(tst, lisp_eq_object(&mock.args[0], INTEGER(ctx, 1)));
  //ASSERT(tst, lisp_eq_object(&mock.args[1], INTEGER(ctx, 2)));

  /* test result */
  //ASSERT_EQ_U(tst, ctx->env->n_values, 1u);
  //ASSERT(tst,      LISP_IS_INTEGER(ctx->env->values));
  //ASSERT_EQ_I(tst, ctx->env->values->data.integer, 23);

  //ASSERT_EQ_U(tst, ctx->env->stack_top, 0u);
  //ASSERT_EQ_U(tst, mock_retire_all(), 0u);
  //lisp_free_lambda_mock(&mock);
  lisp_free_unit_context(ctx);
}

static void test_compile_form_phase1_failure(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t         lambda;
  ASSERT_IS_OK(tst, lisp_symbol_set(ctx->vm,
                                    LISP_AS(SYMBOL(ctx, "myform"), lisp_symbol_t),
                                    FORM(ctx,
                                         lisp_compile_phase1_mock_failure,
                                         lisp_compile_phase2_mock)));
  ASSERT_IS_UNSUPPORTED(tst,
                        lisp_lambda_compile(ctx->env,
                                            &lambda, 
                                            LIST(ctx, 
                                                 SYMBOL(ctx, "myform"),
                                                 INTEGER(ctx, 1),
                                                 INTEGER(ctx, 2),
                                                 NULL)));
  /* @todo test that lambda is exception */
  lisp_free_unit_context(ctx);
}

static void test_compile_form_phase2_failure(unit_test_t * tst) 
{
  lisp_unit_context_t * ctx = lisp_create_unit_context(&lisp_vm_default_param,
                                                       tst);
  lisp_cell_t         lambda;
  ASSERT_IS_OK(tst, lisp_symbol_set(ctx->vm,
                                    LISP_AS(SYMBOL(ctx, "myform"), lisp_symbol_t),
                                    FORM(ctx,
                                         lisp_compile_phase1_mock,
                                         lisp_compile_phase2_mock_failure)));
  ASSERT_IS_UNSUPPORTED(tst,
                        lisp_lambda_compile(ctx->env,
                                            &lambda, 
                                            LIST(ctx, 
                                                 SYMBOL(ctx, "myform"),
                                                 INTEGER(ctx, 1),
                                                 INTEGER(ctx, 2),
                                                 NULL)));
  /* @todo test that lambda is exception */
  lisp_free_unit_context(ctx);
}


void test_lambda(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "lambda");
  /* @todo move to module context */
  TEST(suite, test_list);
  TEST(suite, test_compare_asm);

  TEST(suite, test_lambda_mock_0_args);
  TEST(suite, test_lambda_mock_10_args);

  TEST(suite, test_lisp_make_builtin_lambda);
  TEST(suite, test_lambda_compile_atom);
  TEST(suite, test_lambda_compile_atom_object);
  TEST(suite, test_lambda_compile_nil);
  TEST(suite, test_lambda_compile_symbol);
  TEST(suite, test_lambda_compile_symbol_undefined);
  TEST(suite, test_compile_cons_builtin_arg_arg);

  TEST(suite, test_compile_form_arg_arg);
  TEST(suite, test_compile_form_phase1_failure);
  TEST(suite, test_compile_form_phase2_failure);
}
