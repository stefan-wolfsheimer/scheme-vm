#ifndef __ASSERTION_H__
#define __ASSERTION_H__
#include <stdlib.h>
#include <stdio.h>

typedef struct assertion_t
{
  char               * expect;
  char               * expect_explain;
  char               * file;
  int                  line;
  int                  success;
  int                  is_exception;
  struct assertion_t * next;
} assertion_t;

assertion_t * assertion_create(const char * file, 
			       int          line);

assertion_t * assertion_create_message(const char * file,
				       int          line,
				       const char * expect,
				       const char * explain,
				       int          is_exception);

/** 
 * Free the assertion if it has only one reference.
 * @return next assertion in the list
 */
assertion_t * assertion_free(assertion_t * assertion);

/** 
 * Invert the assertion. 
 */
assertion_t * assertion_invert(assertion_t * assertion);

void assertion_print(FILE * fp, assertion_t * ass, int color);

/*********************************************************************
 * 
 * assertions for int 
 *
 *********************************************************************/
int           assertion_cmp_i(int lhs, 
			      int rhs);
assertion_t * assertion_create_cmp_i(const char * file, 
				     int          line,
				     const char * lhs_expr,
				     const char * rhs_expr,
				     int          lhs,
				     int          rhs,
				     const char * cmp_op,
				     int          only_on_failure);

/*********************************************************************
 * 
 * assertions for vector of int 
 *
 *********************************************************************/
int assertion_cmp_arr_i(const int * lhs, size_t nlhs,
			const int * rhs, size_t nrhs);
assertion_t * assertion_create_cmp_arr_i(const char * file, 
					 int          line,
					 const char * lhs_expr,
					 const char * rhs_expr,
					 const int *  lhs,
					 size_t       nlhs,
					 const int *  rhs,
					 size_t       nrhs,
					 const char * cmp_op,
					 int          only_on_failure);


/*********************************************************************
 * 
 * assertions for unsigned int 
 *
 *********************************************************************/
int assertion_cmp_u(unsigned lhs,
		    unsigned rhs);
assertion_t * assertion_create_cmp_u(const char * file, 
				     int          line,
				     const char * lhs_expr,
				     const char * rhs_expr,
				     unsigned int lhs,
				     unsigned int rhs,
				     const char * cmp_op,
				     int          only_on_failure);

/*********************************************************************
 * 
 * assertions for vector of unsigned int 
 *
 *********************************************************************/
int assertion_cmp_arr_u(const unsigned * lhs, size_t nlhs,
			const unsigned * rhs, size_t nrhs);

assertion_t * assertion_create_cmp_arr_u(const char *      file, 
					 int               line,
					 const char *      lhs_expr,
					 const char *      rhs_expr,
					 const unsigned *  lhs,
					 size_t            nlhs,
					 const unsigned *  rhs,
					 size_t            nrhs,
					 const char *      cmp_op,
					 int               only_on_failure);

/*********************************************************************
 * 
 * assertions for pointer
 *
 *********************************************************************/
int assertion_cmp_ptr(const void * lhs, 
		      const void * rhs);
assertion_t * assertion_create_cmp_ptr(const char * file, 
				       int          line,
				       const char * lhs_expr,
				       const char * rhs_expr,
				       const void * lhs,
				       const void * rhs,
				       const char * cmp_op,
				       int          only_on_failure);

/*********************************************************************
 * 
 * assertions for vector of pointer
 *
 *********************************************************************/
int assertion_cmp_arr_ptr(const void ** lhs, size_t nlhs,
			  const void ** rhs, size_t nrhs);

assertion_t * assertion_create_cmp_arr_ptr(const char *      file, 
					   int               line,
					   const char *      lhs_expr,
					   const char *      rhs_expr,
					   const void **  lhs,
					   size_t            nlhs,
					   const void **     rhs,
					   size_t            nrhs,
					   const char *      cmp_op,
					   int               only_on_failure);

/*********************************************************************
 * 
 * assertions for c-string
 *
 *********************************************************************/
int assertion_cmp_cstr(const char * lhs,
		       const char * rhs);
assertion_t * assertion_create_cmp_cstr(const char * file, 
					int          line,
					const char * lhs_expr,
					const char * rhs_expr,
					const char * lhs,
					const char * rhs,
					const char * cmp_op,
					int          only_on_failure);

/*********************************************************************
 * 
 * assertions for array of c-string
 *
 *********************************************************************/
int assertion_cmp_arr_cstr(const char ** lhs, size_t nlhs,
			   const char ** rhs, size_t nrhs);


assertion_t * assertion_create_cmp_arr_cstr(const char *      file, 
					    int               line,
					    const char *      lhs_expr,
					    const char *      rhs_expr,
					    const char **  lhs,
					    size_t            nlhs,
					    const char **     rhs,
					    size_t            nrhs,
					    const char *      cmp_op,
					    int               only_on_failure);

#endif
