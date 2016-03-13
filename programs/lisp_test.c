#include "util/unit_test.h"
#include <string.h>

void test_test(unit_context_t * ctx);
void test_hash_table(unit_context_t * ctx);
void test_xmalloc(unit_context_t * ctx);
void test_vm(unit_context_t * ctx);
void test_asm(unit_context_t * ctx);
void test_parser(unit_context_t * ctx);
void test_cons(unit_context_t * ctx);
void test_lambda(unit_context_t * ctx);

int main(int argc, const char ** argv)
{
  unit_context_t * ctx = unit_create_context();
  test_test(ctx);
  test_xmalloc(ctx);
  test_hash_table(ctx);
  test_cons(ctx);
  test_vm(ctx);
  test_asm(ctx);
  test_parser(ctx);
  test_lambda(ctx);
  unit_parse_argv(ctx, argc, argv);
  unit_run(stdout, ctx);
  unit_final_report(stdout, ctx);
  unit_free_context(ctx);
  return 0;
}
