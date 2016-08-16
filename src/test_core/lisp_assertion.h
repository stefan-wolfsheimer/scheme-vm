#ifndef __LISP_ASSERTION_H__
#define __LISP_ASSERTION_H__
#include "util/unit_test.h"
#include "core/lisp_type.h"

/* @todo replace pattern ASSERT_EQ_I(tst, expr, lisp_ok) with this macro
   @todo improve readibility (string instead of code in messages
 */
assertion_t * assertion_create_lisp_code(const char * file,
                                         int          line,
                                         const char * expr,
                                         int          actual,
                                         int          code,
                                         const char * cmp,
                                         int          only_on_failure);

#define CREATE_ASSERT_LISP_CODE_EQ(__TEST__, __EXPR__, __CODE__)        \
  if(!unit_add_assertion((__TEST__),                                    \
                           assertion_create_lisp_code(__FILE__,         \
                                                      __LINE__,         \
                                                      (#__EXPR__),      \
                                                      (__EXPR__),       \
                                                      (__CODE__),       \
                                                      "==",             \
                                                      0)))              \
  { return; }

#define CREATE_CHECK_LISP_CODE_EQ(__TEST__, __EXPR__, __CODE__)         \
  unit_add_assertion((__TEST__),                                        \
                     assertion_create_lisp_code(__FILE__,               \
                                                __LINE__,               \
                                                (#__EXPR__),            \
                                                (__EXPR__),             \
                                                (__CODE__),             \
                                                "==",                   \
                                                1))


#define ASSERT_IS_OK(__TST__, __EXPR__)                         \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_OK)

#define ASSERT_IS_ALLOC_ERROR(__TST__, __EXPR__)                        \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_ALLOC_ERROR)

#define ASSERT_IS_TYPE_ERROR(__TST__, __EXPR__)                         \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_TYPE_ERROR)

#define ASSERT_IS_EVAL_ERROR(__TST__, __EXPR__)                         \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_EVAL_ERROR)

#define ASSERT_IS_UNSUPPORTED(__TST__, __EXPR__)                        \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_UNSUPPORTED)

#define ASSERT_IS_UNDEFINED(__TST__, __EXPR__)                          \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_UNDEFINED)

#define ASSERT_IS_RANGE_ERROR(__TST__, __EXPR__)                        \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_RANGE_ERROR)

#define ASSERT_IS_COMPILATION_ERROR(__TST__, __EXPR__)                  \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_COMPILATION_ERROR)

#define ASSERT_IS_STACK_OVERFLOW(__TST__, __EXPR__)                     \
  CREATE_ASSERT_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_STACK_OVERFLOW)

#define CHECK_IS_OK(__TST__, __EXPR__)                          \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_OK) 

#define CHECK_IS_ALLOC_ERROR(__TST__, __EXPR__)                         \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_ALLOC_ERROR) 

#define CHECK_IS_TYPE_ERROR(__TST__, __EXPR__)                          \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_TYPE_ERROR) 

#define CHECK_IS_EVAL_ERROR(__TST__, __EXPR__)                          \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_EVAL_ERROR) 

#define CHECK_IS_UNSUPPORTED(__TST__, __EXPR__)                         \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_UNSUPPORTED) 

#define CHECK_IS_UNDEFINED(__TST__, __EXPR__)                           \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_UNDEFINED) 

#define CHECK_IS_RANGE_ERROR(__TST__, __EXPR__)                         \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_RANGE_ERROR) 

#define CHECK_IS_COMPILATION_ERROR(__TST__, __EXPR__)          \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_COMPILATION_ERROR) 

#define CHECK_IS_STACK_OVERFLOW(__TST__, __EXPR__)                      \
  CREATE_CHECK_LISP_CODE_EQ((__TST__), (__EXPR__), LISP_STACK_OVERFLOW) 

#endif
