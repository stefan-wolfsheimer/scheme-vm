#ifndef __CONTEXT_H__
#define __CONTEXT_H__
#include <stdarg.h>
#include "core/lisp_type.h"

struct lisp_vm_param_t;
struct lisp_vm_t;
struct lisp_eval_env_t;
struct unit_test_t;
struct assertion_t;

/** object that can be used 
 *  for allocation tests 
 */
#define TEST_OBJECT_STATE_UNINIT 0
#define TEST_OBJECT_STATE_INIT   1
#define TEST_OBJECT_STATE_FREE   2

typedef struct lisp_test_object_t
{
  int * flags;
} lisp_test_object_t;


typedef struct lisp_unit_context_t
{
  struct lisp_vm_t       * vm;
  struct lisp_eval_env_t * env;
  struct unit_test_t     * tst;
  struct lisp_cell_t       data;
  lisp_type_id_t           test_object_id;
} lisp_unit_context_t;

lisp_unit_context_t * lisp_create_unit_context(struct lisp_vm_param_t * param,
                                               struct unit_test_t     * tst);
void lisp_free_unit_context(lisp_unit_context_t * context);

lisp_cell_t * INTEGER(lisp_unit_context_t * ctx, 
                      lisp_integer_t        value);
lisp_cell_t * TEST_OBJECT(lisp_unit_context_t * ctx);
lisp_cell_t * SYMBOL(lisp_unit_context_t * ctx,
                     const char * str);
lisp_cell_t * LIST(lisp_unit_context_t * ctx, ...);
lisp_cell_t * NIL(lisp_unit_context_t * ctx);
lisp_cell_t * BUILTIN(lisp_unit_context_t * ctx,
                      lisp_builtin_function_t   func,
                      ...);
lisp_cell_t * FORM(lisp_unit_context_t * ctx,
                   lisp_compile_phase1_t phase1,
                   lisp_compile_phase2_t phase2);

struct assertion_t * lisp_compare_asm_list(const char          * file,
                                           int                   line,
                                           lisp_unit_context_t * ctx,
                                           const char          * expr_lhs,
                                           const lisp_cell_t   * lhs,
                                           const lisp_cell_t   * rhs);

struct assertion_t *  lisp_compare_asm(const char          * file,
                                       int                   line,
                                       lisp_unit_context_t * ctx,
                                       const char          * expr_lhs,
                                       const lisp_cell_t   * cell,
                                       const lisp_cell_t   * structure);

struct assertion_t * lisp_compare_return_code(const char          * file,
                                              int                   line,
                                              int                   code_lhs,
                                              int                   code_rhs,
                                              const char          * op);

#define ASSERT_DISASM(__TST__, __CTX__, __CELL__, __ARGS__,__INSTR__)   \
  if(!unit_add_assertion( (__TST__),                                    \
                          lisp_compare_asm(__FILE__,                    \
                                           __LINE__,                    \
                                           (__CTX__),                   \
                                           #__CELL__,                   \
                                           (__CELL__),                  \
                                           (__INSTR__))))               \
  { return ; }

#endif
