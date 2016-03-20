#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h" 
#include "core/lisp_asm.h" 
#include <stdio.h>

#define ASSERT_DISASSEMBLE(__TEST__, __VM__,                            \
                           __INSTR__, __SIZE__, __STRING__)             \
  {                                                                     \
   lisp_cell_t str;                                                     \
   ASSERT_EQ_U(tst,                                                     \
               lisp_disassemble_instr(                                  \
                                      vm,                               \
                                      &str,                             \
                                      (const lisp_instr_t[])            \
                                      __INSTR__ ),                      \
               (__SIZE__));                                             \
   ASSERT_EQ_CSTR(tst, lisp_c_string(&str), (__STRING__));              \
   lisp_unset_object((__VM__), &str);                                          \
  }

  


static void test_disassemble_instr(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin();
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT_DISASSEMBLE(tst, vm, { LISP_PUSHD(1) },   2, "PUSHD 1");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_PUSHS(2) },   2, "PUSHS 2");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_END },        1, "END");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_PUSHV },      1, "PUSHV");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_MOVS(1,2) },  3, "MOVS 1 2");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_CALLS(3) },   2, "CALLS 3");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_JP(16) },     2, "JP 0x10");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_CALLE_DISPLAY(1) },  3, "CALL DISPLAY 1");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_CALLE_READKEY(1) },  3, "CALL READKEY 1");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_CALLE_NE(1)      },  3, "CALL NE 1");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_CALLE_PLUS(1)    },  3, "CALL PLUS 1");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_RETURN },        1, "RETURN");
  ASSERT_DISASSEMBLE(tst, vm, { LISP_RETURNIF },      1, "RETURNIF");

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}

void test_asm(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "asm");
  TEST(suite, test_disassemble_instr);
}
