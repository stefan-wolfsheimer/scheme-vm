#include "util/unit_test.h"
#include "core/lisp_type.h" 
#include "util/xmalloc.h"
#include "test_core/lisp_assertion.h"
#include "test_core/lisp_vm_check.h"
#include <stdio.h>

static void test_lisp_error_message(unit_test_t * tst)
{
  memcheck_begin();
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_OK),
                 "OK");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_ALLOC_ERROR),
                 "ALLOC_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_TYPE_ERROR),
                 "TYPE_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_EVAL_ERROR),
                 "EVAL_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_UNSUPPORTED),
                 "UNSUPPORTED");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_RANGE_ERROR),
                 "RANGE_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_COMPILATION_ERROR),
                 "COMPILATION_ERROR");
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(LISP_STACK_OVERFLOW), 
                 "STACK_OVERFLOW");
  int i;
  for(i = LISP_STACK_OVERFLOW+1; i < LISP_STACK_OVERFLOW+2; i++) 
  {
    ASSERT_EQ_CSTR(tst,
                   lisp_error_message(i), 
                   "???");
  }
  ASSERT_EQ_CSTR(tst,
                 lisp_error_message(255), 
                 "???");
  memcheck_end();
}

static void test_register_type(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t     * vm;
  lisp_type_id_t  id = 0;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_IS_OK(tst, lisp_register_object_type(vm,
                                              "TEST",
                                              lisp_test_object_destructor,
                                              NULL,
                                              &id));
  ASSERT(tst,       (LISP_TID_OBJECT_MASK & id));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_register_too_many_object_types(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t     * vm;
  lisp_type_id_t  id = 0;
  lisp_type_id_t  i;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_IS_OK(tst, lisp_register_object_type(vm,
                                              "TEST",
                                              lisp_test_object_destructor,
                                              NULL,
                                              &id));
  for(i = id+1; i < vm->types_size; i++)
  {
    if(!CHECK_IS_OK(tst,
                    lisp_register_object_type(vm,
                                              "TEST",
                                              lisp_test_object_destructor,
                                              NULL,
                                              &id)))
      {
        return;
      }
  }
  ASSERT_IS_TYPE_ERROR(tst, lisp_register_object_type(vm,
                                                      "TEST",
                                                      lisp_test_object_destructor,
                                                      NULL,
                                                      &id));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_register_cons_type(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t    * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_type_id_t id = 0; 
  lisp_cell_t cons;
  ASSERT_IS_OK(tst, lisp_register_cons_type(vm,
                                            "MYCONS",
                                            &id));
  ASSERT_IS_OK(tst, lisp_make_cons_typed(vm,
                                         &cons,
                                         id));
  ASSERT(tst, LISP_IS_CONS_OBJECT(&cons));
  ASSERT(tst, id & LISP_TID_CONS_MASK);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

static void test_register_too_many_cons_types(unit_test_t * tst)
{
  memcheck_begin();
  lisp_vm_t     * vm;
  lisp_type_id_t  id = 0;
  lisp_type_id_t  i;
  vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_IS_OK(tst, lisp_register_cons_type(vm,
                                            "TEST",
                                            &id));
  for(i = id+1; i < 0x080; i++)
  {
    if(!CHECK_IS_OK(tst,
                    lisp_register_cons_type(vm,
                                            "TEST",
                                            &id)))
      {
        return;
      }
  }
  ASSERT_IS_TYPE_ERROR(tst, lisp_register_cons_type(vm,
                                                    "TEST",
                                                    &id));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_type(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "type");
  TEST(suite, test_lisp_error_message);
  TEST(suite, test_register_type);
  TEST(suite, test_register_too_many_object_types);
  TEST(suite, test_register_cons_type);
  TEST(suite, test_register_too_many_cons_types);
}
