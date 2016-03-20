#ifndef __LISP_ASSERT_H__
#define __LISP_ASSERT_H__

#ifdef DEBUG
#include "util/unit_test.h"
#elsif
#ifdef DEBUG

#define LISP_ASSERT(__EXPR__)						\
  CHECK(unit_get_current_test(), __EXPR__)				\
#else 
#define LISP_ASSERT(__EXPR__)
#endif

#endif
#if 0
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
