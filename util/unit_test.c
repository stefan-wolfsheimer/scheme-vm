#include "unit_test.h"
#include "xmalloc.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define COLOR_NORMAL   "\x1B[0m"
#define COLOR_RED      "\x1B[31m"
#define COLOR_GREEN    "\x1B[32m"
#define COLOR_YELLOW   "\x1B[33m"
#define COLOR_BLUE     "\x1B[34m"
#define COLOR_MAGENTA  "\x1B[35m"
#define COLOR_CYAN     "\x1B[36m"
#define COLOR_WHITE    "\x1B[37m"

static unit_assertion_t * _unit_create_basic_assertion(unit_test_t      * tst,
						       const char       * file, 
						       int                line);
static void _free_assertion(unit_assertion_t * ass);
static void _print_result(FILE * fp, int success, int color);
static void _print_assertion(FILE * fp, unit_assertion_t * ass, int color);
static void _count_assertion(unit_test_t * tst, unit_assertion_t * ass);

static char * _alloc_sprintf(const char * fmt, ...);
static char * _alloc_strcpy(const char * from);

static int _unit_run_test(FILE * fp, unit_context_t * ctx, unit_test_t * tst);

static void _unit_deactivate_all_suites(unit_context_t * ctx);
static void _unit_final_report_print(FILE * fp, 
                                     unit_context_t * ctx,
                                     const char * name,
                                     const char * result_name,
                                     size_t n_passed,
                                     size_t n_failed,
                                     size_t no_results);

static unit_assertion_t * _unit_create_basic_assertion(unit_test_t      * tst,
						       const char       * file, 
						       int                line) 
{
  unit_assertion_t * assertion = malloc(sizeof(unit_assertion_t));
  if(assertion == NULL) 
  {
    return assertion;
  }
  assertion->expect         = NULL;
  assertion->expect_explain = NULL;
  assertion->file           = _alloc_strcpy(file);
  assertion->line           = line;
  assertion->success        = 0;
  assertion->tst            = NULL;
  assertion->next           = NULL;
  assertion->expect         = NULL;
  if(tst) 
  {
    if(tst->last_assertion != NULL) 
    {
      tst->last_assertion->next = assertion;
      tst->last_assertion = assertion;
    }
    else 
    {
      tst->last_assertion = assertion;
      tst->first_assertion = assertion;
    }
    assertion->tst = tst;
  }
  return assertion;
}

static void _free_assertion(unit_assertion_t * ass)
{
  if(ass->expect) 
  {
    free(ass->expect);
  }
  if(ass->expect_explain) 
  {
    free(ass->expect_explain);
  }
  if(ass->file)
  {
    free(ass->file);
  }
  free(ass);
}

static void _print_result(FILE * fp, int success, int color) 
{
  if(color)
  {
    if(success) fprintf(fp, COLOR_GREEN"[PASSED]"COLOR_NORMAL);
    else fprintf(fp, COLOR_RED"[FAILED]"COLOR_NORMAL);
  }
  else 
  {
    if(success) fprintf(fp, "[PASSED]");
    else fprintf(fp, "[FAILED]");
  }
}

static void _print_assertion(FILE * fp,
                             unit_assertion_t * ass,
			     int color)
{
  _print_result(fp, ass->success, color);
  fprintf(fp, "    %s:%d: assertion <%s>\n",
	  ass->file, ass->line, ass->expect);
  if(ass->expect_explain) 
  {
    const char * begin = ass->expect_explain;
    int size = snprintf(NULL, 0, "            %s:%d: ", ass->file, ass->line);
    while(*begin) 
    {
      const char * ptr = begin;
      while(*ptr && *ptr!= '\n')
      {
	ptr++;
      }
      char * buff = malloc(size + (ptr-begin) + 2);
      int i;
      for(i = 0; i < size; i++) 
      {
	buff[i] = ' ';
      }
      ptr = begin;
      while(*ptr && *ptr!= '\n')
      {
	buff[i++] = *ptr;
	ptr++;
      }
      buff[i] = '\0';
      fprintf(fp, "%s\n", buff);
      begin = ptr;
      if(*begin == '\n')
      { 
	begin++;
      }
      free(buff);
    }
  }
}

static void _count_assertion(unit_test_t * tst, 
                             unit_assertion_t * ass)
{
  if(tst != NULL) 
  {
    if(ass->success) tst->_passed_assertions++;
    else tst->_failed_assertions++;
    if((!ass->success && 
	tst->suite->ctx->verbose_level > 0) || 
       tst->suite->ctx->verbose_level > 1)
    {
      _print_assertion(tst->_fp, ass,tst->suite->ctx->color);
    }
  }
}

static char * _alloc_sprintf(const char * fmt, ...)
{
  int       size;
  va_list   ap1, ap2;
  char    * ret;
  va_start(ap1, fmt);
  va_copy(ap2, ap1);
  size = vsnprintf(NULL, 0, fmt, ap1);
  va_end(ap1);
  ret = malloc( sizeof(char) * (size+1) );
  vsprintf(ret, fmt, ap2);  
  va_end(ap2);
  return ret;
}

static char * _alloc_strcpy(const char * from)
{
  if(from == NULL) 
  {
    return NULL;
  }
  else 
  {
    size_t len = strlen(from);
    char * tmp = malloc(len + 1);
    if(tmp != NULL) 
    {
      strcpy(tmp, from);
    }
    return tmp;
  }
}

static int _unit_run_test(FILE * fp, unit_context_t * ctx, unit_test_t * tst) 
{
  int passed = 1;
  if(ctx->verbose_level) 
  {
    fprintf(fp, "          test '%s'\n", tst->name);
  }
  tst->_fp                = fp;
  tst->_passed_assertions = 0;
  tst->_failed_assertions = 0;
  tst->func(tst);
  if(tst->_passed_assertions + tst->_failed_assertions) 
  {
    ctx->n_assertions_passed+= tst->_passed_assertions;
    ctx->n_assertions_failed+= tst->_failed_assertions;
    if(tst->_failed_assertions) 
    {
      passed = 0;
      ctx->n_tests_failed++;
      if(ctx->verbose_level) 
      {
        _print_result(fp, 0, ctx->color);
        fprintf(fp, "  test '%s'\n", tst->name);
      }
    }
    else 
    {
      if(ctx->verbose_level) 
      {
        _print_result(fp, 1, ctx->color);
        fprintf(fp, "  test '%s'\n", tst->name);
      }
      ctx->n_tests_passed++;
    }
  }
  else 
  {
    if(ctx->verbose_level) 
    {
      _print_result(fp, 2, ctx->color);
      fprintf(fp, "  test '%s'\n", tst->name);
    }
    ctx->n_tests_without_assertions++;
  }
  return passed;
}

static void _unit_deactivate_all_suites(unit_context_t * ctx) 
{
  unit_suite_t * suite = ctx->first_suite;
  while(suite != NULL) 
  {
    suite->active = 0;
    suite = suite->next;
  }
}

static void _unit_final_report_print(FILE * fp, 
                                     unit_context_t * ctx,
                                     const char * name,
                                     const char * result_name,
                                     size_t n_passed,
                                     size_t n_failed,
                                     size_t no_results)
{
  size_t n = n_passed + n_failed + no_results;
  if(ctx->color) 
  {
    if(n == n_passed)
    {
      fprintf(fp, COLOR_GREEN);
    }
    else if(n_failed) 
    {
      fprintf(fp, COLOR_RED);
    }
    else 
    {
      fprintf(fp, COLOR_BLUE);
    }
    fprintf(fp, "%u / %u %s passed "COLOR_NORMAL, n_passed,n,name);
    if(no_results) 
    {
      fprintf(fp, COLOR_BLUE" (%u without %s)"COLOR_NORMAL, 
              no_results, result_name);
    }
    fprintf(fp, "\n");
  }
  else
  {
    fprintf(fp, "%u / %u %s passed ", n_passed,n,name);
    if(no_results) 
    {
      fprintf(fp, " (%u without %s)", no_results, result_name);
    }
    fprintf(fp, "\n");
  }
}

unit_context_t * unit_create_context()
{
  unit_context_t * ctx = malloc(sizeof(unit_context_t));
  if(ctx != NULL) 
  {
    ctx->first_suite                = NULL;
    ctx->last_suite                 = NULL;
    ctx->n_suites_passed            = 0;
    ctx->n_suites_failed            = 0;
    ctx->n_suites_without_tests     = 0;
    ctx->n_suites_deactivated       = 0;
    ctx->n_tests_passed             = 0;
    ctx->n_tests_failed             = 0;
    ctx->n_tests_without_assertions = 0;
    ctx->n_tests_deactivated        = 0;
    ctx->n_assertions_passed        = 0;
    ctx->n_assertions_failed        = 0;
    ctx->verbose_level              = 0;
    ctx->color                      = 1;

  }
  return ctx;
}

void unit_free_context(unit_context_t * ctx)
{
  if(ctx) 
  {
    unit_suite_t * suite = ctx->first_suite;
    while(suite) 
    {
      unit_suite_t * next_suite = suite->next;
      unit_test_t * tst = suite->first_test;
      while(tst) 
      {
        unit_test_t * next_test = tst->next;
        unit_assertion_t * ass = tst->first_assertion;
        while(ass) 
        {
          unit_assertion_t * next_ass = ass->next;
	  _free_assertion(ass);
          ass = next_ass;
        }
        if(tst->name) 
        {
          free(tst->name);
        }
        free(tst);
        tst = next_test;
      }
      if(suite->name != NULL) 
      {
        free(suite->name);
      }
      free(suite);
      suite = next_suite;
    }
    free(ctx);
  }
}

void unit_parse_argv(unit_context_t * ctx, int argc, const char ** argv)
{
  int i;
  int suite_selected = 0;
  for(i = 1; i < argc; i++) 
  {
    if(!strcmp(argv[i], "--verbose")) 
    {
      ctx->verbose_level++;
    }
    else if(!strcmp(argv[i], "--nocolor")) 
    {
      ctx->color = 0;
    }
    else 
    {
      unit_suite_t * suite = ctx->first_suite;
      while(suite != NULL) 
      {
        if(!strcmp(suite->name, argv[i])) 
        {
          if(!suite_selected) 
          {
            suite_selected = 1;
            _unit_deactivate_all_suites(ctx);
          }
          suite->active = 1;
        }
        suite = suite->next;
      }
    } 
  }
}

unit_suite_t * unit_create_suite(unit_context_t * ctx, const char * name)
{
  unit_suite_t * suite = malloc(sizeof(unit_suite_t));
  if(suite != NULL)
  {
    if(ctx->last_suite != NULL) 
    {
      ctx->last_suite->next = suite;
      ctx->last_suite = suite;
    }
    else 
    {
      ctx->last_suite = ctx->first_suite = suite;
    }
    suite->next       = NULL;
    suite->first_test = NULL;
    suite->last_test  = NULL;
    suite->name       = _alloc_strcpy(name);
    suite->active     = 1;
    suite->ctx        = ctx;
  }
  return suite;
}

unit_test_t * unit_create_test(unit_suite_t         * suite, 
                               const char           * name, 
                               unit_test_function_t   func)
{
  unit_test_t * tst = malloc(sizeof(unit_test_t));
  if(tst != NULL)
  {
    if(suite->last_test != NULL) 
    {
      suite->last_test->next = tst;
      suite->last_test = tst;
    }
    else 
    {
      suite->last_test = tst;
      suite->first_test = tst;
    }
    tst->name            = _alloc_strcpy(name);
    tst->func            = func;
    tst->next            = NULL;
    tst->first_assertion = NULL;
    tst->last_assertion  = NULL;
    tst->suite           = suite;
    tst->user_data       = NULL;
    tst->active          = 1;
  }
  return tst;
}

unit_assertion_t * unit_create_assertion(unit_test_t * tst,
					 const char  * expect, 
                                         const char  * file, 
					 int           line, 
                                         int           success)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst, file, line);
  if(ass) 
  {
    ass->expect = malloc(strlen(expect)+1);
    if(ass->expect != NULL) 
    {
      strcpy(ass->expect, expect);
    }
    ass->success = success;
  }
  _count_assertion(tst, ass);
  return ass;
}

int unit_create_check(unit_test_t * tst,
		      const char  * expect,
		      const char  * file, 
		      int           line,
		      int           success,
		      int           verbose)
{
  if(success) 
  {
    return 1;
  }
  else 
  {
    unit_assertion_t * ass = unit_create_assertion(tst, 
						   expect,
						   file,
						   line,
						   success);
    if(tst == NULL) 
    {
      /* not managed by tst */
      if(verbose)
      {
	_print_assertion(stderr, ass, 1);
      }
      _free_assertion(ass);
    }
    return 0;
  }
}

/*********************************************************************
 * 
 * macros to create check and assertion functions
 *
 *********************************************************************/
#define __CREATE_CMP__(__CMP_TYPE__, __TYPE__)				\
  int unit_##__CMP_TYPE__(__TYPE__ lhs, __TYPE__ rhs,			\
			  const char * op)				\
  { return ( !strcmp(op,"==") && (lhs == rhs) ) ||			\
      ( !strcmp(op,"!=") && (lhs != rhs) ) ||				\
      ( !strcmp(op,"<")  && (lhs < rhs) )  ||				\
      ( !strcmp(op, ">") && (lhs > rhs) ) ||				\
      ( !strcmp(op,"<=") && (lhs <= rhs) ) ||				\
      ( !strcmp(op, ">=") && (lhs >= rhs) );				\
  }

#define __CREATE_ARR_CMP__(__CMP_TYPE__, __TYPE__)			\
  int unit_arr_##__CMP_TYPE__(const __TYPE__  * lhs,			\
			      size_t            nlhs,			\
			      const __TYPE__  * rhs,			\
			      size_t            nrhs,			\
			      const char      * op)			\
  {									\
    size_t i;								\
    for(i = 0; i < nlhs; i++)						\
    {									\
      if(i == nrhs)							\
      {									\
	if( !strcmp(op, "==") || !strcmp(op, "<") || !strcmp(op, "<="))	\
	{								\
	  return 0;							\
	}								\
	else								\
	{								\
	  return 1;							\
	}								\
      }									\
      if(!unit_##__CMP_TYPE__(lhs[i], rhs[i],"=="))			\
      {									\
	return unit_##__CMP_TYPE__(lhs[i], rhs[i],op);			\
      }									\
    }									\
    return								\
      ( !strcmp(op, "==") && (nlhs ==  nrhs) ) ||			\
      ( !strcmp(op, "!=") && (nlhs !=  nrhs) ) ||			\
      ( !strcmp(op, "<")  && (nlhs <   nrhs) ) ||			\
      ( !strcmp(op, ">")  && (nlhs >   nrhs) ) ||			\
      ( !strcmp(op, "<=") && (nlhs <=  nrhs) ) ||			\
      ( !strcmp(op, ">=") && (nlhs >= nrhs) );				\
  }

#define __CREATE_ASSERTION__(__CMP_TYPE__, __TYPE__, __FMT__)		\
  unit_assertion_t *							\
  unit_create_assertion_##__CMP_TYPE__(unit_test_t * tst,		\
				       const char * lhs_expr,		\
				       const char * rhs_expr,		\
				       __TYPE__ lhs,			\
				       __TYPE__ rhs,			\
				       const char * file,		\
				       int line,			\
				       const char * op)			\
  {									\
    unit_assertion_t * ass = _unit_create_basic_assertion(tst,		\
							  file,		\
							  line);	\
    ass->success = 0;							\
    ass->expect = _alloc_sprintf("%s%s%s", lhs_expr, op, rhs_expr);	\
    ass->expect_explain = _alloc_sprintf("with %s=="#__FMT__"\n"	\
					 "     %s=="#__FMT__"",		\
					 lhs_expr, lhs, rhs_expr, rhs);	\
    ass->success = unit_##__CMP_TYPE__(lhs, rhs, op);			\
    _count_assertion(tst, ass);						\
    return ass;								\
  }									\

#define __CREATE_ASSERTION_ARR__(__CMP_TYPE__, __TYPE__, __FMT__)	\
  unit_assertion_t *							\
  unit_create_assertion_arr_##__CMP_TYPE__(unit_test_t *     tst,	\
					   const char *      lhs_expr,	\
					   const char *      rhs_expr,	\
					   const __TYPE__  * lhs,	\
					   size_t            nlhs,	\
					   const __TYPE__  * rhs,	\
					   size_t            nrhs,	\
					   const char *      file,	\
					   int               line,	\
					   const char      * op)	\
  {									\
    unit_assertion_t * ass = _unit_create_basic_assertion(tst,		\
							  file,		\
							  line);	\
    ass->success = 0;							\
    ass->expect = _alloc_sprintf("%s%s%s", lhs_expr, op, rhs_expr);	\
    ass->expect_explain = _alloc_sprintf("with %s==%p\n"		\
					 "     %s==%p",			\
					 lhs_expr, lhs, rhs_expr, rhs);	\
    ass->success = unit_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs, op);	\
    _count_assertion(tst, ass);						\
    return ass;								\
  }


#define __CREATE_CHECK__(__CMP_TYPE__, __TYPE__)		\
  int unit_create_check_##__CMP_TYPE__(unit_test_t  * tst,	\
				     const char   * lhs_expr,	\
				     const char   * rhs_expr,	\
				     __TYPE__   lhs,		\
				     __TYPE__   rhs,		\
				     const char   * file,	\
				     int            line,	\
				     const char   * op,		\
				     int            verbose)	\
  {								\
    if(unit_##__CMP_TYPE__(lhs, rhs, op))			\
    {								\
      return 1;							\
    }								\
    else							\
    {								\
      unit_assertion_t * ass;					\
      ass = unit_create_assertion_##__CMP_TYPE__(tst,		\
						lhs_expr,	\
						rhs_expr,	\
						lhs,		\
						rhs,		\
						file,		\
						line,		\
						op);		\
	if(tst == NULL)						\
	{							\
	  /* not managed by tst */				\
	  if(verbose)						\
	  {							\
	    _print_assertion(stderr, ass, 1);			\
	  }							\
	  _free_assertion(ass);					\
	}							\
	return 0;						\
    }								\
  }

#define __CREATE_CHECK_ARR__(__CMP_TYPE__, __TYPE__)			\
  int unit_create_check_arr_##__CMP_TYPE__(unit_test_t    * tst,	\
					   const char     * lhs_expr,	\
					   const char     * rhs_expr,	\
					   const __TYPE__ * lhs,	\
					   size_t           nlhs,	\
					   const __TYPE__ * rhs,	\
					   size_t           nrhs,	\
					   const char     * file,	\
					   int              line,	\
					   const char     * op,		\
					   int              verbose)	\
  {									\
    if(unit_arr_##__CMP_TYPE__(lhs, nlhs, rhs, nrhs, op))		\
    {									\
      return 1;								\
    }									\
    else								\
    {									\
      unit_assertion_t * ass;						\
      ass = unit_create_assertion_arr_##__CMP_TYPE__(tst,		\
						     lhs_expr,		\
						     rhs_expr,		\
						     lhs,		\
						     nlhs,		\
						     rhs,		\
						     nrhs,		\
						     file,		\
						     line,		\
						     op);		\
	if(tst == NULL)							\
	{								\
	  /* not managed by tst */					\
	  if(verbose)							\
	  {								\
	    _print_assertion(stderr, ass, 1);				\
	  }								\
	  _free_assertion(ass);						\
	}								\
	return 0;							\
    }									\
  }


/*********************************************************************
 * 
 * int 
 *
 *********************************************************************/
__CREATE_CMP__(           cmp_i, int);
__CREATE_ARR_CMP__(       cmp_i, int);
__CREATE_ASSERTION__(     cmp_i, int, "%d");
__CREATE_ASSERTION_ARR__( cmp_i, int, "%d");
__CREATE_CHECK__(         cmp_i, int);
__CREATE_CHECK_ARR__(     cmp_i, int);

/*********************************************************************
 * 
 * unsigned int 
 *
 *********************************************************************/
__CREATE_CMP__(           cmp_u, unsigned int);
__CREATE_ASSERTION__(     cmp_u, unsigned int, "%u");
__CREATE_CHECK__(         cmp_u, unsigned int);
__CREATE_ARR_CMP__(       cmp_u, unsigned int);
__CREATE_ASSERTION_ARR__( cmp_u, unsigned int, "%u");
__CREATE_CHECK_ARR__(     cmp_u, unsigned int);

/*********************************************************************
 * 
 * pointer
 *
 *********************************************************************/
__CREATE_CMP__(           cmp_ptr, const void *);
__CREATE_ASSERTION__(     cmp_ptr, const void *, "%p");
__CREATE_CHECK__(         cmp_ptr, const void *);
__CREATE_ARR_CMP__(       cmp_ptr, const void *);
__CREATE_ASSERTION_ARR__( cmp_ptr, const void *, "%p");
__CREATE_CHECK_ARR__(     cmp_ptr, const void *);

/*********************************************************************
 * 
 * c string
 *
 *********************************************************************/
int unit_cmp_cstr(const char * lhs, const char * rhs, const char * op)
{
  return ( !strcmp(op,"==") && !strcmp(lhs, rhs)) ||
    ( !strcmp(op,"!=") && strcmp(lhs, rhs))       ||
    ( !strcmp(op,"<")  && strcmp(lhs, rhs)<0)     ||
    ( !strcmp(op, ">") && strcmp(lhs, rhs)>0)     ||
    ( !strcmp(op,"<=") && strcmp(lhs, rhs)<=0)    ||
    ( !strcmp(op, ">=")&& strcmp(lhs, rhs)>=0);
}
__CREATE_ARR_CMP__(   cmp_cstr, const char *);
__CREATE_ASSERTION__( cmp_cstr, const char *, "%s");
__CREATE_CHECK__(     cmp_cstr, const char *);
__CREATE_ASSERTION_ARR__( cmp_cstr, const char *, "%s");
__CREATE_CHECK_ARR__(     cmp_cstr, const char *);


/*********************************************************************
 * 
 * memchecker
 *
 *********************************************************************/
unit_assertion_t * unit_memchecker(unit_test_t              * tst,
                                   struct memchecker_t      * memcheck,
                                   const char               * file, 
                                   int                        line)
{
  size_t i;
  memchecker_chunk_t * chunk;
  int ret = 1;
  char buff[512];
  for(i = 0; i < memcheck->n_chunks; i++)
  {
    chunk = &memcheck->chunks[i];
    if(chunk->alloc_file != NULL)
    {
      if(chunk->free_file == NULL)
      {
        sprintf(buff, "segment %p (allocated %s:%d) freed", 
                chunk->ptr,
                chunk->alloc_file,
                chunk->alloc_line);
	unit_create_assertion(tst, buff, file, line, 0);
        ret = 0;
      }
    }
  }
  for(i = 0; i < memcheck->n_not_managed; i++) 
  {
    ret = 0;
    unit_create_assertion(tst,memcheck->message_not_managed[i], file, line, 0);
  }
  for(i = 0; i < memcheck->n_double_free; i++) 
  {
    ret = 0;
    unit_create_assertion(tst, memcheck->message_double_free[i], file, line, 0);
  }
  return unit_create_assertion(tst, "memory segmentation", file, line, ret); 
}

void unit_run(FILE * fp, unit_context_t * ctx)
{
  unit_suite_t * suite = ctx->first_suite;
  while(suite != NULL) 
  {
    if(suite->active) 
    {
      unit_test_t * tst = suite->first_test;
      int suite_passed = 1;
      if(ctx->verbose_level) 
      {
        fprintf(fp,"         suite '%s':\n", suite->name);
      }
      if(tst == NULL) 
      {
        ctx->n_suites_without_tests++;
        if(ctx->verbose_level) 
        {
          _print_result(fp, 2, ctx->color);
          fprintf(fp," suite '%s':\n", suite->name);
        }
      }
      else 
      {
        while(tst != NULL) 
        {
          suite_passed&= _unit_run_test(fp, ctx, tst); 
          tst = tst->next;
        }
        if(suite_passed) 
        {
          ctx->n_suites_passed++;      
          if(ctx->verbose_level) 
          {
            _print_result(fp, 1, ctx->color);
            fprintf(fp," suite '%s':\n", suite->name);
          }
        }
        else
        {
          ctx->n_suites_failed++;
          if(ctx->verbose_level) 
          {
            _print_result(fp, 0, ctx->color);
            fprintf(fp," suite '%s':\n", suite->name);
          }
        }
      }
    }
    else 
    {
      ctx->n_suites_deactivated++;
      unit_test_t * tst = suite->first_test;
      while(tst) 
      {
        ctx->n_tests_deactivated++;
        tst = tst->next;
      }
    }
    suite = suite->next;
  }
}

                                     
void unit_final_report(FILE * fp,
                       unit_context_t * ctx)
{
  _unit_final_report_print(fp, ctx,
                           "suites", "tests",
                           ctx->n_suites_passed,
                           ctx->n_suites_failed,
                           ctx->n_suites_without_tests);

  if(ctx->n_suites_deactivated) 
  {
    size_t n = 
      ctx->n_suites_passed +
      ctx->n_suites_failed +
      ctx->n_suites_without_tests +
      ctx->n_suites_deactivated;
    if(ctx->color) 
    {
      fprintf(fp, "%s%u / %u suites deactivated%s\n", 
              COLOR_BLUE,
              ctx->n_suites_deactivated,
              n,
              COLOR_NORMAL);
      fprintf(fp, COLOR_BLUE);
    }
    else 
    {
      fprintf(fp, "%u / %u suites deactivated\n", ctx->n_suites_deactivated, n);
    }
  }

  _unit_final_report_print(fp, ctx, 
                           "tests", "assertions",
                           ctx->n_tests_passed,
                           ctx->n_tests_failed,
                           ctx->n_tests_without_assertions);
  if(ctx->n_tests_deactivated) 
  {
    size_t n = 
      ctx->n_tests_deactivated +
      ctx->n_tests_passed + 
      ctx->n_tests_failed +
      ctx->n_tests_without_assertions;
    if(ctx->color) 
    {
      fprintf(fp, "%s%u / %u tests deactivated%s\n", 
              COLOR_BLUE,
              ctx->n_tests_deactivated, n,
              COLOR_NORMAL);
      fprintf(fp, COLOR_BLUE);
    }
    else 
    {
      fprintf(fp, "%u / %u tests deactivated\n", (unsigned int)(ctx->n_tests_deactivated), (unsigned)n);
    }
  }

  _unit_final_report_print(fp, ctx, 
                           "assertions", "",
                           ctx->n_assertions_passed,
                           ctx->n_assertions_failed,
                           0);

}



