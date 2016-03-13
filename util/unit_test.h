#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__
#include <stdlib.h>
#include <stdio.h>

/**
Todo:
assertion      impl.   test
ASSERT_EQ_U    ok 
 CHECK_EQ_U     
ASSERT_NEQ_U    
 CHECK_NEQ_U     
ASSERT_GT_U    ok  
 CHECK_GT_U    ok 
ASSERT_LT_U    
 CHECK_LT_U    
ASSERT_GE_U    
 CHECK_GE_U    
ASSERT_LE_U    
 CHECK_LE_U    

*/

struct memchecker_t;
struct unit_test_t;
typedef void(*unit_test_function_t)(struct unit_test_t * tst);

typedef struct unit_assertion_t
{
  char * expect;
  char * expect_explain;
  char * file;
  int line;
  int success;
  struct unit_test_t * tst;
  struct unit_assertion_t * next;
} unit_assertion_t;

typedef struct unit_test_t
{
  char                    * name;
  unit_test_function_t      func;
  struct unit_test_t      * next;
  struct unit_assertion_t * first_assertion;
  struct unit_assertion_t * last_assertion;
  struct unit_suite_t     * suite;
  short int                 active;
  void                    * user_data;
  FILE                    * _fp;
  int                       _passed_assertions;
  int                       _failed_assertions;
} unit_test_t;

typedef struct unit_suite_t
{
  char                  * name;
  struct unit_test_t    * first_test;
  struct unit_test_t    * last_test;
  struct unit_context_t * ctx;
  struct unit_suite_t   * next;
  short int               active;
} unit_suite_t;

typedef struct unit_context_t
{
  int verbose_level;
  int color;
  struct unit_suite_t * first_suite;
  struct unit_suite_t * last_suite;
  size_t n_suites_passed;
  size_t n_suites_without_tests;
  size_t n_suites_failed;
  size_t n_suites_deactivated;
  size_t n_tests_passed;
  size_t n_tests_without_assertions;
  size_t n_tests_failed;
  size_t n_tests_deactivated;
  size_t n_assertions_passed;
  size_t n_assertions_failed;
} unit_context_t;

unit_context_t * unit_create_context();

void unit_free_context(unit_context_t * ctx);

void unit_parse_argv(unit_context_t * ctx, int argc, const char ** argv);

unit_suite_t * unit_create_suite(unit_context_t * ctx, 
                                 const char * name);

unit_test_t * unit_create_test(unit_suite_t * suite, 
                               const char * name, 
                               unit_test_function_t func);

unit_assertion_t * unit_create_assertion(unit_test_t * tst, 
                                         const char * expect,
                                         const char * file, int line, 
                                         int success);

unit_assertion_t * unit_create_assertion_eq_cstr(unit_test_t * tst,
                                                 const char * lhs_expr,
                                                 const char * rhs_expr,
                                                 const char * lhs,
                                                 const char * rhs,
                                                 const char * file,
                                                 int line);

unit_assertion_t * unit_create_assertion_eq_cstr_list(unit_test_t  * tst,
                                                      const char   * lhs_expect,
                                                      const char   * rhs_expect,
                                                      const char  ** lhs,
                                                      size_t         n,
                                                      const char  ** rhs,
                                                      size_t         m,
                                                      const char  *  file,
                                                      int            line,
                                                      int            negation);

unit_assertion_t * unit_create_assertion_cmp_u(unit_test_t * tst,
                                               const char * lhs_expr,
                                               const char * rhs_expr,
                                               unsigned int lhs,
                                               unsigned int rhs,
                                               const char * file,
                                               int          line,
                                               const char * op);

unit_assertion_t * unit_create_assertion_cmp_ptr(unit_test_t * tst,
                                                 const char * lhs_expr,
                                                 const char * rhs_expr,
                                                 const void * lhs,
                                                 const void * rhs,
                                                 const char * file,
                                                 int          line,
                                                 const char * op);

unit_assertion_t * unit_memchecker(unit_test_t              * tst,
                                   struct memchecker_t      * memcheck,
                                   const char               * file, 
                                   int                        line);
                                         
void unit_run(FILE * fp,
              unit_context_t * ctx);

void unit_final_report(FILE * fp,
                       unit_context_t * ctx);

#define TEST(__SUITE__, __FUNC__) \
  unit_create_test((__SUITE__), #__FUNC__, (__FUNC__))

/** todo: create check for all assertion types */
#define ASSERT(__TEST__,__EXPR__)                                       \
  if(! unit_create_assertion((__TEST__),                                \
                             #__EXPR__" is true ",                      \
                             __FILE__, __LINE__,                        \
                             (__EXPR__))->success)                      \
    { return; }

#define CHECK(__TEST__, __EXPR__)                                       \
  ((__EXPR__) ? 1 :                                                     \
   unit_create_assertion((__TEST__),                                    \
                         #__EXPR__" is true ",                          \
                         __FILE__, __LINE__,                            \
                         0)->success)

#define ASSERT_FALSE(__TEST__,__EXPR__)                                 \
  if(! unit_create_assertion((__TEST__),                                \
                             #__EXPR__" is false ",                     \
                             __FILE__, __LINE__,                        \
                             !(__EXPR__))->success)                     \
    { return; }

#define CHECK_FALSE(__TEST__,__EXPR__)                                  \
  (!(__EXPR__) ? 1: unit_create_assertion((__TEST__),                   \
                                          #__EXPR__" is false ",        \
                                          __FILE__, __LINE__,           \
                                          0)->success)   


#define ASSERT_EQ(__TEST__ , __LHS__ , __RHS__)                         \
  if(! unit_create_assertion((__TEST__),                                \
                             #__LHS__" == "#__RHS__,                    \
                             __FILE__, __LINE__,                        \
                             ((__LHS__)==(__RHS__)))->success)          \
    { return; }

#define ASSERT_EQ_CSTR(__TEST__, __LHS__, __RHS__)                      \
  if(! unit_create_assertion_eq_cstr((__TEST__),                        \
                                     #__LHS__,                          \
                                     #__RHS__,                          \
                                     (__LHS__),                         \
                                     (__RHS__),                         \
                                     __FILE__, __LINE__)->success)      \
    { return; }

/** Compare two arrays of strings */
#define ASSERT_EQ_CSTR_LIST(__TEST__, __LHS__,__SLHS__,                 \
                            __RHS__, __SRHS__)                          \
  if(! unit_create_assertion_eq_cstr_list((__TEST__),                   \
                                          #__LHS__,                     \
                                          #__RHS__,                     \
                                          (__LHS__),                    \
                                          (__SLHS__),                   \
                                          (__RHS__),                    \
                                          (__SRHS__),                   \
                                          __FILE__,                     \
                                          __LINE__,                     \
                                          0)->success)                  \
    { return; }

#define ASSERT_NEQ_CSTR_LIST(__TEST__, __LHS__,__SLHS__,                \
                            __RHS__, __SRHS__)                          \
  if(! unit_create_assertion_eq_cstr_list((__TEST__),                   \
                                          #__LHS__,                     \
                                          #__RHS__,                     \
                                          (__LHS__),                    \
                                          (__SLHS__),                   \
                                          (__RHS__),                    \
                                          (__SRHS__),                   \
                                          __FILE__,                     \
                                          __LINE__,                     \
                                          1)->success)                  \
    { return; }

/*****************************************************************
 * unsigned int assertions 
 *****************************************************************/
#define ASSERT_EQ_U(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_u((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                   "==")->success)                    \
    { return; }

#define CHECK_EQ_U(__TEST__, __LHS__, __RHS__)                          \
  ( (__LHS__) == (__RHS__) ? 1 :                                        \
    unit_create_assertion_cmp_u((__TEST__),                             \
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                "==")->success)

#define ASSERT_NEQ_U(__TEST__, __LHS__, __RHS__)                      \
  if(! unit_create_assertion_cmp_u((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                   "!=")->success)                    \
    { return; }

#define CHECK_NEQ_U(__TEST__, __LHS__, __RHS__)                         \
  ( (__LHS__) != (__RHS__) ? 1 :                                        \
    unit_create_assertion_cmp_u((__TEST__),                             \
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                "!=")->success)

#define CHECK_GT_U(__TEST__, __LHS__, __RHS__)                          \
  ( (__LHS__) > (__RHS__) ? 1 :                                         \
    unit_create_assertion_cmp_u((__TEST__),                             \
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                ">")->success)

#define ASSERT_GT_U(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_u((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                   ">")->success)                     \
    { return; }

#define CHECK_GE_U(__TEST__, __LHS__, __RHS__)                          \
  ( (__LHS__) >= (__RHS__) ? 1 :                                        \
    unit_create_assertion_cmp_u((__TEST__),                             \
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                ">=")->success)

#define ASSERT_GE_U(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_u((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                ">=")->success)                       \
    { return; }

#define CHECK_LT_U(__TEST__, __LHS__, __RHS__)                          \
  ( (__LHS__) < (__RHS__) ? 1 :                                         \
    unit_create_assertion_cmp_u((__TEST__),                             \
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                "<")->success)

#define ASSERT_LT_U(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_u((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                   "<")->success)                     \
    { return; }

#define CHECK_LE_U(__TEST__, __LHS__, __RHS__)                          \
  ( (__LHS__) <= (__RHS__) ? 1 :                                        \
    unit_create_assertion_cmp_u((__TEST__),                             \
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                "<=")->success)

#define ASSERT_LE_U(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_u((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                "<=")->success)                       \
    { return; }


/*****************************************************************
 * pointer  assertions 
 *****************************************************************/
#define ASSERT_EQ_PTR(__TEST__, __LHS__, __RHS__)                     \
  if(! unit_create_assertion_cmp_ptr((__TEST__),                        \
                                   #__LHS__,                          \
                                   #__RHS__,                          \
                                   (__LHS__),                         \
                                   (__RHS__),                         \
                                   __FILE__,                          \
                                   __LINE__,                          \
                                   "==")->success)                    \
    { return; }

#define CHECK_EQ_PTR(__TEST__, __LHS__, __RHS__)			\
  ( (__LHS__) == (__RHS__) ? 1 :					\
    unit_create_assertion_cmp_ptr((__TEST__),				\
                                #__LHS__,                               \
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                "==")->success)

#define ASSERT_NEQ_PTR(__TEST__, __LHS__, __RHS__)                      \
  if(! unit_create_assertion_cmp_ptr((__TEST__),                        \
				     #__LHS__,                          \
				     #__RHS__,                          \
				     (__LHS__),                         \
				     (__RHS__),                         \
				     __FILE__,                          \
				     __LINE__,                          \
				     "!=")->success)                    \
    { return; }

#define CHECK_NEQ_PTR(__TEST__, __LHS__, __RHS__)			\
  ( (__LHS__) != (__RHS__) ? 1 :                                        \
    unit_create_assertion_cmp_ptr((__TEST__),				\
				  #__LHS__,				\
				  #__RHS__,				\
				  (__LHS__),				\
				  (__RHS__),				\
				  __FILE__,				\
				  __LINE__,				\
				  "!=")->success)

#define CHECK_GT_PTR(__TEST__, __LHS__, __RHS__)			\
  ( (__LHS__) > (__RHS__) ? 1 :                                         \
    unit_create_assertion_cmp_ptr((__TEST__),				\
				  #__LHS__,				\
                                #__RHS__,                               \
				  (__LHS__),				\
				  (__RHS__),				\
				  __FILE__,				\
				  __LINE__,				\
				  ">")->success)

#define ASSERT_GT_PTR(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_ptr((__TEST__),                        \
				     #__LHS__,                          \
				     #__RHS__,                          \
				     (__LHS__),                         \
				     (__RHS__),                         \
				     __FILE__,                          \
				     __LINE__,                          \
				     ">")->success)                     \
    { return; }

#define CHECK_GE_PTR(__TEST__, __LHS__, __RHS__)			\
  ( (__LHS__) >= (__RHS__) ? 1 :                                        \
    unit_create_assertion_cmp_ptr((__TEST__),				\
				  #__LHS__,				\
				  #__RHS__,				\
				  (__LHS__),				\
				  (__RHS__),				\
				  __FILE__,				\
				  __LINE__,				\
				  ">=")->success)

#define ASSERT_GE_PTR(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_ptr((__TEST__),                        \
				     #__LHS__,                          \
				     #__RHS__,                          \
				     (__LHS__),                         \
				     (__RHS__),                         \
				     __FILE__,                          \
				     __LINE__,                          \
				     ">=")->success)			\
  { return; }

#define CHECK_LT_PTR(__TEST__, __LHS__, __RHS__)			\
  ( (__LHS__) < (__RHS__) ? 1 :                                         \
    unit_create_assertion_cmp_ptr((__TEST__),				\
				  #__LHS__,				\
                                #__RHS__,                               \
                                (__LHS__),                              \
                                (__RHS__),                              \
                                __FILE__,                               \
                                __LINE__,                               \
                                "<")->success)

#define ASSERT_LT_PTR(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_ptr((__TEST__),                        \
				     #__LHS__,                          \
				     #__RHS__,				\
				     (__LHS__),                         \
				     (__RHS__),                         \
				     __FILE__,                          \
				     __LINE__,                          \
				     "<")->success)                     \
  { return; }

#define CHECK_LE_PTR(__TEST__, __LHS__, __RHS__)                        \
   ( (__LHS__) <= (__RHS__) ? 1 :                                       \
    unit_create_assertion_cmp_ptr((__TEST__),                           \
                                  #__LHS__,                             \
                                  #__RHS__,                             \
                                  (__LHS__),                            \
                                  (__RHS__),                            \
                                  __FILE__,                             \
                                  __LINE__,                             \
                                  "<=")->success)

#define ASSERT_LE_PTR(__TEST__, __LHS__, __RHS__)                       \
  if(! unit_create_assertion_cmp_ptr((__TEST__),                        \
				     #__LHS__,                          \
				     #__RHS__,                          \
				     (__LHS__),                         \
				     (__RHS__),                         \
				     __FILE__,                          \
				     __LINE__,                          \
				     "<=")->success)			\
    { return; }
 

#define CHECK_MEMCHECK(__TEST__, __MEMCHECKER__)                   \
  ( unit_memchecker( (__TEST__), (__MEMCHECKER__),                 \
                     __FILE__, __LINE__)->success )



#define ASSERT_MEMCHECK(__TEST__, __MEMCHECKER__)                 \
  if(! unit_memchecker( (__TEST__), (__MEMCHECKER__),             \
                        __FILE__, __LINE__)->success)             \
    { return; }
  
#endif
