#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_exception.h" 
#include "core/lisp_vm.h" 

static void test_make_exception(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_cell_t       exception;
  ASSERT_EQ_I(tst, lisp_make_exception(vm, &exception,
				       32,
				       NULL,
				       0,
				       "error msg %d",
				       1), LISP_OK);
  lisp_exception_t * exception_obj = LISP_AS(&exception,
					     lisp_exception_t);
  ASSERT_EQ_I(tst,  exception.type_id, LISP_TID_EXCEPTION);
  ASSERT(tst,       LISP_IS_OBJECT(&exception));
  ASSERT(tst,       LISP_IS_EXCEPTION(&exception));

  ASSERT(tst,       LISP_IS_STRING(&exception_obj->error_message));
  
  ASSERT_FALSE(tst, lisp_string_cmp_c_string(LISP_AS(&exception_obj
						     ->error_message,
						     lisp_string_t),
					     "error msg 1"));
  ASSERT_EQ_I(tst, lisp_unset_object(vm, &exception), LISP_OK);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_exception(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "exception");
  TEST(suite, test_make_exception);
}
