#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include "core/lisp_asm.h" 
#include <stdio.h>

static void test_make_empty_lambda(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t lambda;
  ASSERT_FALSE(tst, 
               lisp_make_lambda_instr(vm, 
                                      &lambda,
                                      0,
                                      NULL,
                                      0,
                                      (const lisp_instr_t[]) { 
                                        LISP_END
                                       }));
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&lambda), 1);
  ASSERT(tst, LISP_IS_LAMBDA(&lambda));
  ASSERT_EQ_U(tst, LISP_AS(&lambda, lisp_lambda_t)->args_size, 0);
  ASSERT_EQ_U(tst, LISP_AS(&lambda, lisp_lambda_t)->instr_size, 0);
  ASSERT_EQ_U(tst, LISP_AS(&lambda, lisp_lambda_t)->data_size, 0);
  LISP_UNSET(vm, &lambda);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_make_lambda(unit_test_t * tst) 
{
  /* @todo implement copy_object_as_root  */
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t data[4];
  data[0] = lisp_nil;
  LISP_MAKE_INTEGER(&data[1], 123);
  lisp_make_string(vm, &data[2], "abc");

  lisp_make_cons_car_cdr(vm, &data[3], &data[2], &data[0]);
  lisp_cell_t lambda;
  ASSERT_FALSE(tst, 
               lisp_make_lambda_instr(vm, 
                                      &lambda,
                                      0,
                                      data,
                                      4,
                                      (const lisp_instr_t[]) { 
                                        LISP_PUSHD(1),
                                        LISP_END
					  }));

  ASSERT(tst,      LISP_IS_LAMBDA(&lambda));
  ASSERT(tst,      LISP_IS_OBJECT(&lambda));
  ASSERT_EQ_U(tst, LISP_REFCOUNT(&lambda), 1);
  ASSERT_EQ_U(tst, LISP_AS(&lambda, lisp_lambda_t)->args_size, 0);
  ASSERT_EQ_U(tst, LISP_AS(&lambda, lisp_lambda_t)->instr_size, 2);
  ASSERT_EQ_U(tst, LISP_AS(&lambda, lisp_lambda_t)->data_size, 4);
  /* @todo test if data has been copied
   *       depends on requires:
   * @0001 lisp_eq(lisp_cell_t *, lisp_cell_t *)
   */
  LISP_UNSET(vm, &data[0]);
  LISP_UNSET(vm, &data[1]);
  LISP_UNSET(vm, &data[2]);
  LISP_UNSET(vm, &data[3]);
  LISP_UNSET(vm, &lambda);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_make_lambda_alloc_error_1(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t data[4];
  data[0] = lisp_nil;
  LISP_MAKE_INTEGER(&data[1], 123);
  lisp_make_string(vm, &data[2], "abc");
  lisp_make_cons_car_cdr(vm, &data[3], &data[2], &data[0]);
  lisp_cell_t lambda = lisp_nil;
  /* allocation will fail */
  memcheck_expected_alloc(memcheck, 0);
  ASSERT(tst, 
	 lisp_make_lambda_instr(vm, 
				&lambda,
				0,
				data,
				4,
				(const lisp_instr_t[]) { 
				  LISP_PUSHD(1),
				    LISP_END
				    }));

  ASSERT(tst,      LISP_IS_NIL(&lambda));
  LISP_UNSET(vm, &data[0]);
  LISP_UNSET(vm, &data[1]);
  LISP_UNSET(vm, &data[2]);
  LISP_UNSET(vm, &data[3]);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_make_lambda_alloc_error_2(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t data[4];
  data[0] = lisp_nil;
  LISP_MAKE_INTEGER(&data[1], 123);
  lisp_make_string(vm, &data[2], "abc");
  lisp_make_cons_car_cdr(vm, &data[3], &data[2], &data[0]);
  lisp_cell_t lambda = lisp_nil;
  /* copy of objects will fail 
   * @todo: copy conses to root instead of white set 
   *        then this test should fail
   */
  memcheck_expected_alloc(memcheck, 1);
  memcheck_expected_alloc(memcheck, 1); /* <- @todo set it to 0 and 
					   handle mock */
  ASSERT_FALSE(tst, 
	       lisp_make_lambda_instr(vm, 
				      &lambda,
				      0,
				      data,
				      4,
				      (const lisp_instr_t[]) { 
					LISP_PUSHD(1),
					LISP_END
				       }));
  LISP_UNSET(vm, &data[0]);
  LISP_UNSET(vm, &data[1]);
  LISP_UNSET(vm, &data[2]);
  LISP_UNSET(vm, &data[3]);
  LISP_UNSET(vm, &lambda);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

void test_lambda(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "lambda");
  TEST(suite, test_make_empty_lambda);
  TEST(suite, test_make_lambda);
  TEST(suite, test_make_lambda_alloc_error_1);
  TEST(suite, test_make_lambda_alloc_error_2);
}
