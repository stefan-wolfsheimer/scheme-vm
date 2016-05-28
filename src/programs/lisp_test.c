#include "util/unit_test.h"
#include <string.h>

void test_test(unit_context_t * ctx);
void test_mock(unit_context_t * ctx);
void test_xmalloc(unit_context_t * ctx);
void test_xstring(unit_context_t * ctx);
void test_assertion(unit_context_t * ctx);
void test_hash_table(unit_context_t * ctx);
void test_vm(unit_context_t * ctx);

void test_cons(unit_context_t * ctx);
void test_symbol(unit_context_t * ctx);
void test_string(unit_context_t * ctx);
void test_eval(unit_context_t * ctx);

void test_asm(unit_context_t * ctx);
void test_parser(unit_context_t * ctx);
void test_lambda(unit_context_t * ctx);

void test_builtin(unit_context_t * ctx);
void test_builtin_values(unit_context_t * ctx);

int main(int argc, const char ** argv)
{
  unit_context_t * ctx = unit_create_context();
  test_test(ctx);
  test_mock(ctx);
  test_xmalloc(ctx);
  test_xstring(ctx);
  test_assertion(ctx);
  test_hash_table(ctx);

  test_vm(ctx);
  test_cons(ctx);
  test_symbol(ctx);
  test_string(ctx);
  test_eval(ctx);

  test_builtin(ctx);
  test_builtin_values(ctx);
  /* 
     @todo refactor 
  test_asm(ctx);
  test_parser(ctx);
  test_lambda(ctx);
  */

  int parse_result = unit_parse_argv(ctx, argc, argv);
  if(parse_result == UNIT_ARGV_RUN) 
  {
    unit_run(stdout, ctx);
    unit_final_report(stdout, ctx);
  }
  else if(parse_result == UNIT_ARGV_LIST) 
  {
    unit_list_suites(stdout, ctx);
  }
  else if(parse_result == UNIT_ARGV_ERROR) 
  {
    unit_print_help(stderr, ctx, argv[0]);
  }
  else if(parse_result == UNIT_ARGV_HELP) 
  {
    unit_print_help(stdout, ctx, argv[0]);
  }
  unit_free_context(ctx);
  if(parse_result == UNIT_ARGV_ERROR) 
  {
    return 8;
  }
  else 
  {
    return 0;
  }
}
