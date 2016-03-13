#include "util/xmalloc.h"
#include "util/unit_test.h"
#include "core/lisp_parser.h" 
#include <stdio.h>
#include <ctype.h>

static int final_parser_state(lisp_vm_t * vm, 
                              const lisp_char_t * str,
                              short end_state,
                              size_t level)
{
  lisp_parser_state_t state;
  lisp_parser_init(vm, &state);
  lisp_parse_next_expression_cstr(vm, &state, str);
  int ret = state.state == end_state;
  LISP_SET(vm, &vm->value, &state.expression);
  lisp_parser_free(vm, &state);
  return ret;
}

static void test_no_expression(unit_test_t * tst)
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT(tst, final_parser_state(vm, "",      LISP_PARSER_START,0));
  ASSERT(tst, LISP_IS_NIL(&vm->value));
  ASSERT(tst, final_parser_state(vm, "  ",    LISP_PARSER_START,0));
  ASSERT(tst, LISP_IS_NIL(&vm->value));
  ASSERT(tst, final_parser_state(vm, " \n  ", LISP_PARSER_START,0));
  ASSERT(tst, LISP_IS_NIL(&vm->value));
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

static void test_parens(unit_test_t * tst) 
{
  memchecker_t * memcheck = memcheck_begin(0);
  lisp_vm_t * vm = lisp_create_vm(&lisp_vm_default_param);
  ASSERT(tst, final_parser_state(vm, "(",     LISP_PARSER_CAR, 1));
  ASSERT(tst, LISP_IS_NIL(&vm->value));

  ASSERT(tst, final_parser_state(vm, "()",    LISP_PARSER_END, 0));
  ASSERT(tst, LISP_IS_NIL(&vm->value));

  ASSERT(tst, final_parser_state(vm, " ( ) ", LISP_PARSER_END, 0));
  ASSERT(tst, LISP_IS_NIL(&vm->value));

  ASSERT(tst, final_parser_state(vm, " ) " ,  LISP_PARSER_UNEXPECTED, 0));
  ASSERT(tst, LISP_IS_NIL(&vm->value));

  ASSERT(tst, final_parser_state(vm, " ( () ) ", LISP_PARSER_END, 0));
  ASSERT(tst, LISP_IS_CONS(&vm->value));
  ASSERT(tst, LISP_IS_NIL(LISP_CAR(&vm->value)));
  ASSERT(tst, LISP_IS_NIL(LISP_CDR(&vm->value)));

  ASSERT(tst, final_parser_state(vm, " ( () () ) ", LISP_PARSER_END, 0));
  ASSERT(tst, LISP_IS_CONS(&vm->value));
  ASSERT(tst, LISP_IS_NIL(LISP_CAR(&vm->value)));
  ASSERT(tst, LISP_IS_CONS(LISP_CDR(&vm->value)));
  ASSERT(tst, LISP_IS_NIL(LISP_CAR(LISP_CDR(&vm->value))));
  ASSERT(tst, LISP_IS_NIL(LISP_CDR(LISP_CDR(&vm->value))));

  ASSERT(tst, final_parser_state(vm, " ( ( () ) ( () ) () ) ", 
                                 LISP_PARSER_END, 0));
  ASSERT(tst, LISP_IS_CONS(&vm->value));
  ASSERT(tst, LISP_IS_CONS(LISP_CAR(&vm->value)));
  ASSERT(tst, LISP_IS_NIL(LISP_CAR(LISP_CAR(&vm->value))));
  ASSERT(tst, LISP_IS_CONS(LISP_CDR(&vm->value)));
  ASSERT(tst, LISP_IS_CONS(LISP_CAR(LISP_CDR(&vm->value))));
  ASSERT(tst, LISP_IS_NIL(LISP_CAR(LISP_CAR(LISP_CDR(&vm->value)))));
  ASSERT(tst, LISP_IS_CONS(LISP_CDR(LISP_CDR(&vm->value))));
  ASSERT(tst, LISP_IS_NIL(LISP_CAR(LISP_CDR(LISP_CDR(&vm->value)))));
  ASSERT(tst, LISP_IS_NIL(LISP_CDR(LISP_CDR(LISP_CDR(&vm->value)))));

  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst, memcheck);
  memcheck_finalize(1);
}

void test_parser(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "parser");
  TEST(suite, test_no_expression);
  TEST(suite, test_parens); 
}
