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

static void _print_assertion(unit_test_t * tst, 
                             unit_assertion_t * ass, 
                             const char * file, 
                             int line) 
{
  if(ass->success) tst->_passed_assertions++;
  else tst->_failed_assertions++;
  if((!ass->success && 
      tst->suite->ctx->verbose_level > 0) || 
     tst->suite->ctx->verbose_level > 1)
  {
    _print_result(tst->_fp, ass->success, tst->suite->ctx->color);

    fprintf(tst->_fp, "    %s:%d: assertion <%s>\n",
            file, line, ass->expect);
    if(ass->expect_explain) 
    {
      const char * begin = ass->expect_explain;
      int size = snprintf(NULL, 0, "            %s:%d: ", file,line);
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
        fprintf(tst->_fp, "%s\n", buff);
        begin = ptr;
        if(*begin == '\n')
        { 
          begin++;
        }
        free(buff);
      }
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

static void _unit_deactivate_all_suites(unit_context_t * ctx) 
{
  unit_suite_t * suite = ctx->first_suite;
  while(suite != NULL) 
  {
    suite->active = 0;
    suite = suite->next;
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

unit_assertion_t * _unit_create_basic_assertion(unit_test_t * tst, 
                                                const char  * file, 
                                                int           line) 
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
  assertion->tst            = tst;
  assertion->next           = NULL;
  assertion->expect         = NULL;
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
  return assertion;
}


unit_assertion_t * unit_create_assertion(unit_test_t * tst, 
                                         const char * expect, 
                                         const char * file, int line, 
                                         int success)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst,
                                                        file,
                                                        line);
  if(ass) 
  {
    ass->expect = malloc(strlen(expect)+1);
    if(ass->expect != NULL) 
    {
      strcpy(ass->expect, expect);
    }
    ass->success = success;
  }
  _print_assertion(tst, ass, file, line);
  return ass;
}

unit_assertion_t * unit_create_assertion_cmp_u(unit_test_t * tst,
                                               const char * lhs_expr,
                                               const char * rhs_expr,
                                               unsigned int lhs,
                                               unsigned int rhs,
                                               const char * file,
                                               int line,
                                               const char * op)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst,
                                                        file,
                                                        line);
  ass->success = 0;
  ass->expect = _alloc_sprintf("%s%s%s", lhs_expr, op, rhs_expr);
  ass->expect_explain = _alloc_sprintf("with %s==%u\n"
                                       "     %s==%u", 
                                       lhs_expr, lhs, rhs_expr, rhs);
  if(!strcmp(op,"==")) 
  {
    ass->success = (lhs == rhs);
  }
  else if(!strcmp(op,"!=")) 
  {
    ass->success = (lhs != rhs);
  }
  else if(!strcmp(op,"<")) 
  {
    ass->success = (lhs < rhs);
  }
  else if(!strcmp(op, ">")) 
  {
    ass->success = (lhs > rhs);
  }
  else if(!strcmp(op,"<=")) 
  {
    ass->success = (lhs <= rhs);
  }
  else if(!strcmp(op, ">=")) 
  {
    ass->success = (lhs >= rhs);
  }

  _print_assertion(tst, ass, file, line);
  return ass;
}

unit_assertion_t * unit_create_assertion_cmp_ptr(unit_test_t * tst,
                                                 const char * lhs_expr,
                                                 const char * rhs_expr,
                                                 const void * lhs,
                                                 const void * rhs,
                                                 const char * file,
                                                 int          line,
                                                 const char * op)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst,
                                                        file,
                                                        line);
  ass->success = 0;
  ass->expect = _alloc_sprintf("%s%s%s", lhs_expr, op, rhs_expr);
  ass->expect_explain = _alloc_sprintf("with %s==%p\n"
                                       "     %s==%p", 
                                       lhs_expr, lhs, rhs_expr, rhs);
  if(!strcmp(op,"==")) 
  {
    ass->success = (lhs == rhs);
  }
  else if(!strcmp(op,"!=")) 
  {
    ass->success = (lhs != rhs);
  }
  else if(!strcmp(op,"<")) 
  {
    ass->success = (lhs < rhs);
  }
  else if(!strcmp(op, ">")) 
  {
    ass->success = (lhs > rhs);
  }
  else if(!strcmp(op,"<=")) 
  {
    ass->success = (lhs <= rhs);
  }
  else if(!strcmp(op, ">=")) 
  {
    ass->success = (lhs >= rhs);
  }

  _print_assertion(tst, ass, file, line);
  return ass;
}

unit_assertion_t * unit_create_assertion_eq_ptr(unit_test_t * tst,
                                                const char  * lhs_expr,
                                                const char  * rhs_expr,
                                                void        * lhs,
                                                void        * rhs,
                                                const char  * file,
                                                int           line,
                                                int           negation)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst,
                                                        file,
                                                        line);
  ass->success = 0;
  if(negation)
  {
    ass->expect = _alloc_sprintf("%s!=%s", lhs_expr, rhs_expr);
  }
  else 
  {
    ass->expect = _alloc_sprintf("%s==%s", lhs_expr, rhs_expr);
  }
  ass->expect_explain = _alloc_sprintf("with %s==%p\n"
                                       "     %s==%p", lhs_expr, lhs, rhs_expr, rhs);
  ass->success = (negation && lhs != rhs) || (!negation && lhs == rhs);
  _print_assertion(tst, ass, file, line);
  return ass;
}

/*Todo: const char  * lhs_expect, const char  * rhs_expect, int negation */
unit_assertion_t * unit_create_assertion_eq_cstr(unit_test_t * tst,
                                                 const char * expect,
                                                 const char * rhs_expr,
                                                 const char * lhs,
                                                 const char * rhs,
                                                 const char * file,
                                                 int line)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst,
                                                        file,
                                                        line);
  if(ass != NULL) 
  {
    if(lhs == NULL) 
    {
      if(rhs == NULL) 
      {
        ass->expect = _alloc_sprintf("%s (NULL==NULL)", expect);
        ass->success = 1;
      }
      else 
      {
        ass->expect = _alloc_sprintf("%s (NULL==\"%s\")", expect, rhs);
        ass->success = 0;
      }
    }
    else 
    {
      if(rhs == NULL) 
      {
        ass->expect = _alloc_sprintf("%s (%s==NULL)", expect, lhs);
        ass->success = 0;
      }
      else 
      {
        ass->expect = _alloc_sprintf("%s (\"%s\"==\"%s\")", expect, lhs, rhs);
        if(!strcmp(lhs, rhs)) 
        {
          ass->success = 1;
        }
        else 
        {
          ass->success = 0;
        }
      }
    }
    _print_assertion(tst, ass, file, line);
  }
  return ass;
}

unit_assertion_t * unit_create_assertion_eq_cstr_list(unit_test_t  * tst,
                                                      const char   * lhs_expr,
                                                      const char   * rhs_expr,
                                                      const char  ** lhs,
                                                      size_t         n,
                                                      const char  ** rhs,
                                                      size_t         m,
                                                      const char  *  file,
                                                      int            line,
                                                      int            negation)
{
  unit_assertion_t * ass = _unit_create_basic_assertion(tst,
                                                        file,
                                                        line);
  if(negation)
  {
    ass->expect = _alloc_sprintf("%s!=%s", lhs_expr, rhs_expr);
  }
  else 
  {
    ass->expect = _alloc_sprintf("%s==%s", lhs_expr, rhs_expr);
  }
  size_t max_nm = n > m ? n : m;
  size_t size = 0;
  size_t j = 0,i = 0;
  ass->success = negation ? 0 : 1;
  for(i = 0; i < max_nm; i++) 
  {
    size+= 13+10;
    if(i < n) size += strlen(lhs[i]);
    else      size += 4;
    if(i < m) size += strlen(rhs[i]);
    else      size += 4;
  }
  ass->expect_explain = malloc(size + 2);
  for(i = 0; i < max_nm; i++) 
  {
    ass->expect_explain[j++] = '[';
    j+= snprintf(&ass->expect_explain[j], 8, "%u", i);
    ass->expect_explain[j++] = ']';
    ass->expect_explain[j++] = ':';
    ass->expect_explain[j++] = ' ';
    if(i < n) 
    {
      ass->expect_explain[j++] = '"';
      // Todo: escape string
      strcpy(&ass->expect_explain[j], lhs[i]);
      j+= strlen(lhs[i]);
      ass->expect_explain[j++] = '"';
    }
    else 
    {
      ass->expect_explain[j++] = 'N';
      ass->expect_explain[j++] = 'U';
      ass->expect_explain[j++] = 'L';
      ass->expect_explain[j++] = 'L';
    }

    ass->expect_explain[j++] = ' ';
    if(i < n && i < m) 
    {
      if(strcmp(lhs[i], rhs[i])) 
      {
        ass->expect_explain[j++] = '!';
        ass->success = negation ? 1 : 0;
      }
      else 
      {
        ass->expect_explain[j++] = '=';
      }
    }
    else 
    {
      ass->expect_explain[j++] = '!';
      ass->success = negation ? 1 : 0;
    }
    ass->expect_explain[j++] = '=';
    ass->expect_explain[j++] = ' ';
    if(i < m) 
    {
      ass->expect_explain[j++] = '"';
      //Todo: escape string
      strcpy(&ass->expect_explain[j], rhs[i]);
      j+= strlen(rhs[i]);
      ass->expect_explain[j++] = '"';
    }
    else 
    {
      ass->expect_explain[j++] = 'N';
      ass->expect_explain[j++] = 'U';
      ass->expect_explain[j++] = 'L';
      ass->expect_explain[j++] = 'L';
    }
    ass->expect_explain[j++] = '\n';
  }
  ass->expect_explain[j++] = '\0';
  _print_assertion(tst, ass, file, line);
  return ass;
}

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
    unit_create_assertion(tst, memcheck->message_not_managed[i], file, line, 0);
  }
  for(i = 0; i < memcheck->n_double_free; i++) 
  {
    ret = 0;
    unit_create_assertion(tst,memcheck->message_double_free[i], file, line, 0);
  }
  return unit_create_assertion(tst, "memory segmentation", file, line, ret);
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
      fprintf(fp, "%u / %u tests deactivated\n", ctx->n_tests_deactivated, n);
    }
  }

  _unit_final_report_print(fp, ctx, 
                           "assertions", "",
                           ctx->n_assertions_passed,
                           ctx->n_assertions_failed,
                           0);

}



