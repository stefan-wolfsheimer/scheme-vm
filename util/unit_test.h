#ifndef __UNIT_TEST_H__
#define __UNIT_TEST_H__
#include <stdlib.h>
#include <stdio.h>

#define UNIT_CHECKS_VERBOSE 0
/* @todo printer for array assertions */
/* @todo extra library for c string */

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
					 const char  * expect,
                                         const char  * file, int line, 
                                         int           success);

int unit_create_check(unit_test_t * tst,
		      const char  * expect,
		      const char  * file, 
		      int           line, 
		      int           success,
		      int           verbose);

/*********************************************************************
 * 
 * macros to create check and assertion function declarations and
 * assertion macros
 *
 *********************************************************************/
#define __CREATE_CMP__(__CMP_TYPE__, __TYPE__)				\
  int unit_##__CMP_TYPE__(__TYPE__ lhs, __TYPE__ rhs, const char * op);

#define __CREATE_ARR_CMP__(__CMP_TYPE__, __TYPE__)			\
  int unit_arr_##__CMP_TYPE__(const __TYPE__  * lhs,			\
			      size_t            nlhs,			\
			      const __TYPE__  * rhs,			\
			      size_t            nrhs,			\
			      const char      * op);

#define __CREATE_ASSERTION__(__CMP_TYPE__, __TYPE__)			\
  unit_assertion_t *							\
  unit_create_assertion_##__CMP_TYPE__(unit_test_t * tst,		\
				       const char * lhs_expr,		\
				       const char * rhs_expr,		\
				       __TYPE__ lhs,			\
				       __TYPE__ rhs,			\
				       const char * file,		\
				       int line,			\
				       const char * op);

#define __CREATE_ASSERTION_ARR__(__CMP_TYPE__, __TYPE__)		\
  unit_assertion_t *							\
  unit_create_assertion_arr_##__CMP_TYPE__(unit_test_t *    tst,	\
					   const char *     lhs_expr,	\
					   const char *     rhs_expr,	\
					   const __TYPE__ * lhs,	\
					   size_t            n_lhs,	\
					   const __TYPE__  * rhs,	\
					   size_t            n_rhs,	\
					   const char *      file,	\
					   int               line,	\
					   const char      * op);

#define __CREATE_CHECK__(__CMP_TYPE__, __TYPE__)		\
  int unit_create_check_##__CMP_TYPE__(unit_test_t  * tst,	\
				       const char   * lhs_expr,	\
				       const char   * rhs_expr,	\
				       __TYPE__       lhs,	\
				       __TYPE__       rhs,	\
				       const char   * file,	\
				       int            line,	\
				       const char   * op,	\
				       int            verbose);

#define __CREATE_CHECK_ARR__(__CMP_TYPE__, __TYPE__)			\
  int unit_create_check_arr_##__CMP_TYPE__(unit_test_t      * tst,	\
					     const char     * lhs_expr,	\
					     const char     * rhs_expr,	\
					     const __TYPE__ * lhs,	\
					     size_t           n_lhs,	\
					     const __TYPE__ * rhs,	\
					     size_t           n_rhs,	\
					     const char     * file,	\
					     int              line,	\
					     const char     * op,	\
					     int              verbose);


/* @todo use this macro throughout and undefine it afterwards */
#define __CREATE_ASSERT_CMP__(__TYPE__,__TEST__, __LHS__, __RHS__,_OP_)	\
  if(!unit_create_assertion_##__TYPE__((__TEST__),			\
				       (#__LHS__),			\
				       (#__RHS__),			\
				       (__LHS__),			\
				       (__RHS__),			\
				       __FILE__,			\
				       __LINE__,			\
				       _OP_)->success)			\
  { return ; }

#define __CREATE_ASSERT_ARR_CMP__(__TYPE__,__TEST__,			\
				  __LHS__,__NLHS__,			\
				  __RHS__,__NRHS__,_OP_)		\
  if(!unit_create_assertion_arr_##__TYPE__((__TEST__),			\
					   (#__LHS__),			\
					   (#__RHS__),			\
					   (__LHS__),			\
					   (__NLHS__),			\
					   (__RHS__),			\
					   (__NRHS__),			\
					   __FILE__,			\
					   __LINE__,			\
					   _OP_)->success)		\
  { return ; }

#define __CREATE_CHECK_CMP__(__TYPE__,__TEST__, __LHS__, __RHS__,_OP_)	\
  unit_create_check_##__TYPE__((__TEST__),				\
			       (#__LHS__),				\
			       (#__RHS__),				\
			       (__LHS__),				\
			       (__RHS__),				\
			       __FILE__,				\
			       __LINE__,				\
			       (_OP_),					\
			       UNIT_CHECKS_VERBOSE)

#define __CREATE_CHECK_ARR_CMP__(__TYPE__,__TEST__,			\
				 __LHS__,__NLHS__,__RHS__,__NRHS__,	\
				 __OP__)				\
  unit_create_check_arr_##__TYPE__((__TEST__),				\
				     (#__LHS__),			\
				   (#__RHS__),				\
				   (__LHS__),				\
				   (__NLHS__),				\
				   (__RHS__),				\
				   (__NRHS__),				\
				   __FILE__,				\
				   __LINE__,				\
				   (__OP__),				\
				   UNIT_CHECKS_VERBOSE)
  

/*********************************************************************
 * 
 * int
 *
 **********************************************************************/
__CREATE_CMP__(           cmp_i, int);
__CREATE_ASSERTION__(     cmp_i, int);
__CREATE_CHECK__(         cmp_i, int);
__CREATE_ARR_CMP__(       cmp_i, int);
__CREATE_ASSERTION_ARR__( cmp_i, int);
__CREATE_CHECK_ARR__(     cmp_i, int);


#define ASSERT_EQ_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_I(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_I(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_i, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of int
 *
 **********************************************************************/
#define ASSERT_EQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")


#define CHECK_EQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_i, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_I(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_i, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">=")

  

/*********************************************************************
 * 
 * unsigned int
 *
 **********************************************************************/
__CREATE_CMP__(           cmp_u, unsigned int);
__CREATE_ASSERTION__(     cmp_u, unsigned int);
__CREATE_CHECK__(         cmp_u, unsigned int);
__CREATE_ARR_CMP__(       cmp_u, unsigned int);
__CREATE_ASSERTION_ARR__( cmp_u, unsigned int);
__CREATE_CHECK_ARR__(     cmp_u, unsigned int);

#define ASSERT_EQ_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_U(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_ASSERT_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_U(__TEST__, __LHS__, __RHS__)				\
  __CREATE_CHECK_CMP__(cmp_u, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector unsigned int
 *
 **********************************************************************/
#define ASSERT_EQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")


#define CHECK_EQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_u, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_U(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_u, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">=")


/*********************************************************************
 * 
 * pointer
 *
 *********************************************************************/
__CREATE_CMP__(           cmp_ptr, const void *);
__CREATE_ASSERTION__(     cmp_ptr, const void *);
__CREATE_CHECK__(         cmp_ptr, const void *);
__CREATE_ARR_CMP__(       cmp_ptr, const void *);
__CREATE_ASSERTION_ARR__( cmp_ptr, const void *);
__CREATE_CHECK_ARR__(     cmp_ptr, const void *);

#define ASSERT_EQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_PTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_ptr, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of pointer
 *
 *********************************************************************/
#define ASSERT_EQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_ptr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")


#define CHECK_EQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_ptr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_ptr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_ptr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_ptr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_ptr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_ptr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_ptr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_ptr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_ptr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_ptr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_PTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_ptr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">=")


/*********************************************************************
 * 
 * c string
 *
 *********************************************************************/
__CREATE_CMP__(           cmp_cstr, const char *);
__CREATE_ASSERTION__(     cmp_cstr, const char *);
__CREATE_CHECK__(         cmp_cstr, const char *);
__CREATE_ARR_CMP__(       cmp_cstr, const char *);
__CREATE_ASSERTION_ARR__( cmp_cstr, const char *);
__CREATE_CHECK_ARR__(     cmp_cstr, const char *);

#define ASSERT_EQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "==")

#define CHECK_EQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "==")

#define ASSERT_NEQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "!=")

#define CHECK_NEQ_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "!=")

#define ASSERT_GT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, ">")

#define CHECK_GT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, ">")

#define ASSERT_GE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, ">=")

#define CHECK_GE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, ">=")

#define ASSERT_LT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "<")

#define CHECK_LT_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "<")

#define ASSERT_LE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_ASSERT_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "<=")

#define CHECK_LE_CSTR(__TEST__, __LHS__, __RHS__)			\
  __CREATE_CHECK_CMP__(cmp_cstr, (__TEST__), __LHS__, __RHS__, "<=")

/*********************************************************************
 * 
 * vector of c-string
 *
 **********************************************************************/
#define ASSERT_EQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "==")

#define CHECK_EQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_cstr, (__TEST__),			\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "==")

#define ASSERT_NEQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cmp_cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "!=")


#define CHECK_NEQ_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_CHECK_ARR_CMP__(cmp_cstr, (__TEST__),			\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "!=")

#define ASSERT_LT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cmp_cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<")


#define CHECK_LT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_cstr, (__TEST__),			\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<")


#define ASSERT_LE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cmp_cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    "<=")


#define CHECK_LE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_cstr, (__TEST__),			\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), "<=")


#define ASSERT_GT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__) \
  __CREATE_ASSERT_ARR_CMP__(cmp_cstr, (__TEST__),			\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">")

#define CHECK_GT_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_cstr, (__TEST__),			\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">")

#define ASSERT_GE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_ASSERT_ARR_CMP__(cmp_cstr, (__TEST__),				\
			    (__LHS__),(__NLHS__),			\
			    (__RHS__), (__NRHS__),			\
			    ">=")


#define CHECK_GE_ARR_CSTR(__TEST__, __LHS__,__NLHS__, __RHS__,__NRHS__)	\
  __CREATE_CHECK_ARR_CMP__(cmp_cstr, (__TEST__),				\
			   (__LHS__),(__NLHS__),			\
			   (__RHS__), (__NRHS__), ">=")


#undef __CREATE_CMP__
#undef __CREATE_ARR_CMP__
#undef __CREATE_ASSERTION__
#undef __CREATE_CHECK__
#undef __CREATE_ASSERTION_ARR__
#undef __CREATE_CHECK_ARR__


unit_assertion_t * unit_memchecker(unit_test_t              * tst,
                                   struct memchecker_t      * memcheck,
                                   const char               * file, 
                                   int                        line);
                                         
void unit_run(FILE * fp,
              unit_context_t * ctx);

void unit_final_report(FILE * fp,
                       unit_context_t * ctx);

#define TEST(__SUITE__, __FUNC__)			\
  unit_create_test((__SUITE__), #__FUNC__, (__FUNC__))

#define ASSERT(__TEST__,__EXPR__)                                       \
  if(!unit_create_assertion((__TEST__),					\
			    #__EXPR__" is true ",			\
			    __FILE__, __LINE__,				\
			    (__EXPR__))->success)			\
  { return; }

#define CHECK(__TEST__, __EXPR__)                                       \
  unit_create_check((__TEST__),						\
		    #__EXPR__" is true ",				\
		    __FILE__, __LINE__,					\
		    (__EXPR__),						\
		    UNIT_CHECKS_VERBOSE)

#define ASSERT_FALSE(__TEST__,__EXPR__)                                 \
  if(!unit_create_assertion((__TEST__),					\
			    #__EXPR__" is false ",			\
			    __FILE__, __LINE__,				\
			    !(__EXPR__))->success)			\
  { return; }

#define CHECK_FALSE(__TEST__,__EXPR__)                                  \
  unit_create_check((__TEST__),						\
		    #__EXPR__" is false ",				\
		    __FILE__, __LINE__,					\
		    !(__EXPR__),					\
		    UNIT_CHECKS_VERBOSE)

#if 0
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

/** Compare two arrays of strings */
#define CHECK_EQ_CSTR_LIST(__TEST__, __LHS__,__SLHS__,			\
                            __RHS__, __SRHS__)                          \
  unit_create_chcek_eq_cstr_list((__TEST__),				\
				 #__LHS__,				\
				 #__RHS__,				\
				 (__LHS__),				\
				 (__SLHS__),				\
				 (__RHS__),				\
				 (__SRHS__),				\
				 __FILE__,				\
				 __LINE__,				\
				 0,					\
				 UNIT_CHECKS_VERBOSE)		    

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

#define CHECK_NEQ_CSTR_LIST(__TEST__, __LHS__,__SLHS__,			\
                            __RHS__, __SRHS__)                          \
  unit_check_assertion_eq_cstr_list((__TEST__),				\
				    #__LHS__,				\
				    #__RHS__,				\
				    (__LHS__),				\
				    (__SLHS__),				\
				    (__RHS__),				\
				    (__SRHS__),				\
				    __FILE__,				\
				    __LINE__,				\
				    1,					\
				    UNIT_CHECKS_VERBOSE)

#endif




/*****************************************************************
 * xmalloc assertions 
 *****************************************************************/
#define ASSERT_MEMCHECK(__TEST__, __MEMCHECKER__)                 \
  if(! unit_memchecker( (__TEST__), (__MEMCHECKER__),             \
                        __FILE__, __LINE__)->success)             \
    { return; }

#define CHECK_MEMCHECK(__TEST__, __MEMCHECKER__)                   \
  ( unit_memchecker( (__TEST__), (__MEMCHECKER__),                 \
                     __FILE__, __LINE__)->success )
  
#endif
