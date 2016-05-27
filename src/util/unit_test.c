#include "unit_test.h"
#include "xmalloc.h"
#include "xstring.h"
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

static void _print_result(FILE * fp, int success, int color);

static int _unit_run_test(FILE * fp, unit_context_t * ctx, unit_test_t * tst);

static void _unit_deactivate_all_suites(unit_context_t * ctx);
static void _unit_final_report_print(FILE * fp, 
                                     unit_context_t * ctx,
                                     const char * name,
                                     const char * result_name,
                                     size_t n_passed,
                                     size_t n_failed,
                                     size_t no_results);



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
  unit_test_t * tst;
  while(suite != NULL) 
  {
    suite->active = 0;
    tst           = suite->first_test;
    while(tst != NULL) 
    {
      tst->active = 0;
      tst = tst->next;
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
    fprintf(fp, "%zu / %zu %s passed "COLOR_NORMAL, n_passed,n,name);
    if(no_results) 
    {
      fprintf(fp, COLOR_BLUE" (%zu without %s)"COLOR_NORMAL, 
              no_results, result_name);
    }
    fprintf(fp, "\n");
  }
  else
  {
    fprintf(fp, "%zu / %zu %s passed ", n_passed,n,name);
    if(no_results) 
    {
      fprintf(fp, " (%zu without %s)", no_results, result_name);
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
        assertion_t * ass = tst->first_assertion;
        while(ass) 
        {
          assertion_t * next_ass = ass->next;
	  assertion_free(ass);
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

size_t _get_suite_name_length(const char * name)
{
  size_t ret = 0;
  while(*name) 
  {
    if(*name == ':') 
    {
      name++;
      ret++;
      if(*name == ':') 
      {
	return ret-1;
      }
    }
    else 
    {
      ret++;
      name++;
    }
  }
  return ret;
}

unit_suite_t * unit_find_suite_by_name(unit_context_t * ctx, const char * name)
{
  size_t suite_name_length = _get_suite_name_length(name);
  unit_suite_t * suite = ctx->first_suite;
  while(suite != NULL) 
  {
    if(!strncmp(suite->name, name, suite_name_length)) 
    {
      return suite;
    }
    suite = suite->next;
  }
  return NULL;
}

unit_test_t * unit_find_test_by_name(unit_suite_t * suite, const char * name)
{
  if(suite == NULL) 
  {
    return NULL;
  }
  else 
  {
    unit_test_t * tst = suite->first_test;
    while(tst) 
    {
      if(!strcmp(name, tst->name)) 
      {
	return tst;
      }
      tst = tst->next;
    }
    return NULL;
  }
}

const char * unit_extract_test_name(const char * suite_test_name)
{
  int state = 0;
  while(*suite_test_name) 
  {
    if(state == 0 && *suite_test_name == ':') 
    {
      state = 1;
    }
    else if(state == 1 && *suite_test_name == ':')
    {
      state = 2;
    }
    else if(state == 2)
    {
      break;
    }
    suite_test_name++;
  }
  if(state == 2) 
  {
    return suite_test_name;
  }
  else 
  {
    return NULL;
  }
}

static int _check_argument_name(unit_context_t * ctx, const char * name) 
{
  unit_suite_t * suite = unit_find_suite_by_name(ctx, name);
  if(suite) 
  {
    const char * test_name = unit_extract_test_name(name);
    if(test_name != NULL) 
    {
      if(unit_find_test_by_name(suite, test_name) == NULL)
      {
	fprintf(stderr, "undefined test %s\n", name);
	return 0;
      }
    }
  }
  else 
  {
    fprintf(stderr, "undefined suite %s\n", name);
    return 0;
  }
  return 1;
}

static void _activate_tests(unit_context_t * ctx, const char * name)
{
  unit_suite_t * suite = unit_find_suite_by_name(ctx, name);
  if(suite) 
  {
    suite->active = 1;
    const char * test_name = unit_extract_test_name(name);
    unit_test_t * tst;
    if(test_name == NULL) 
    {
      tst = suite->first_test;
      while(tst) 
      {
	tst->active = 1;
	tst = tst->next;
      }
    }
    else 
    {
      tst = unit_find_test_by_name(suite, test_name);
      if(tst) 
      {
	tst->active = 1;
      }
    }
  }
}

int unit_parse_argv(unit_context_t * ctx, int argc, const char ** argv)
{
  int is_filtered = 0;
  int i;
  int verbose_level = 0;
  int color = 1;
  int ret = UNIT_ARGV_RUN;
  /* scan flags */  
  for(i = 1; i < argc; i++) 
  {
    if(!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) 
    {
      if(ret == UNIT_ARGV_RUN) 
      {
	ret = UNIT_ARGV_HELP;
      }
    }
    else if(!strcmp(argv[i],"--list") || !strcmp(argv[i],"-l")) 
    {
      if(ret == UNIT_ARGV_RUN) 
      {
	ret = UNIT_ARGV_LIST;
      }
    }
    else if(!strcmp(argv[i], "--verbose") || !strcmp(argv[i],"-v")) 
    {
      verbose_level++;
    }
    else if(!strcmp(argv[i], "--nocolor")) 
    {
      color = 0;
    }
    else if(argv[i][0] == '-') 
    {
      fprintf(stderr, "invalid option %s\n", argv[i]);
      ret = UNIT_ARGV_ERROR;
    }
    else 
    {
      is_filtered = 1;
      if(!_check_argument_name(ctx, argv[i])) 
      {
	ret = UNIT_ARGV_ERROR;
      }
    }
  }
  if(ret == UNIT_ARGV_ERROR || ret == UNIT_ARGV_HELP) 
  {
    return ret;
  }
  ctx->verbose_level = verbose_level;
  ctx->color         = color;
  if(is_filtered) 
  {
    _unit_deactivate_all_suites(ctx);
    for(i = 1; i < argc; i++) 
    {
      if(argv[i][0] != '-') 
      {
	_activate_tests(ctx, argv[i]);
      }
    }
  }
  return ret;
}

void unit_print_help(FILE * fp, unit_context_t * ctx, const char * progr)
{
  fprintf(fp, "usage:\n");
  fprintf(fp, "%s [OPTIONS] [ARGUMENTS]\n", progr);
  fprintf(fp, "\n");
  fprintf(fp, "OPTIONS:\n");
  fprintf(fp, "--help, -h      show help and exit\n");
  fprintf(fp, "--verbose, -v   increase verbose level\n");
  fprintf(fp, "                (flag may appear multiple times)\n");
  fprintf(fp, "--list,-l       list all suites (or tests)\n");
  fprintf(fp, "                if --verbose is set list all tests\n");
  fprintf(fp, "                if suites are given as ARGUMENTS, only the\n");
  fprintf(fp, "                test tests of that suite are listed\n");
  fprintf(fp, "--nocolor       don't use colors to report results\n");
  fprintf(fp, "\n");
  fprintf(fp, "ARGUMENTS:\n");
  unit_suite_t * suite = ctx->first_suite;
  int n = 0;
  size_t len = 0;
  while(suite != NULL) 
  {
    if(n) 
    {
      fprintf(fp, "|");
    }
    if(len > 32) 
    {
      fprintf(fp, "\n");
      len = 0;
    }
    fprintf(fp, "%s", suite->name);
    len += strlen(suite->name);
    n++;
    suite = suite->next;
  }
  fprintf(fp, "\nOR\nSUITE::TEST_NAME\n");
  fprintf(fp, "\n");
  fprintf(fp, "\n");
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
    int old = memcheck_enable(0);
    suite->name       = alloc_strcpy(name);
    memcheck_enable(old);
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
    int old = memcheck_enable(0);
    tst->name            = alloc_strcpy(name);
    memcheck_enable(old);
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

int unit_add_assertion(unit_test_t * tst, assertion_t * assertion_lst)
{
  int ok = 1;
  assertion_t * current = assertion_lst;
  assertion_t * next    = NULL;
  while(current) 
  {
    next = current->next;
    if(!current->success) 
    {
      ok = 0;
    }
    if(tst != NULL) 
    {
      if(current->success) tst->_passed_assertions++;
      else tst->_failed_assertions++;
      if(tst->last_assertion != NULL) 
      {
	tst->last_assertion->next = current;
	tst->last_assertion       = current;
      }
      else 
      {
	tst->last_assertion  = current;
	tst->first_assertion = current;
      }
      if((!current->success && 
	  tst->suite->ctx->verbose_level > 0) || 
	 tst->suite->ctx->verbose_level > 1)
      {
	assertion_print(tst->_fp, current,tst->suite->ctx->color);
      }
    }
    else 
    {
      assertion_free(current);
    }
    current = next;
  }
  return ok;
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
	  if(tst->active) 
	  {
	    suite_passed&= _unit_run_test(fp, ctx, tst); 
	  }
	  else 
	  {
	    ctx->n_tests_deactivated++;
	  }
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

void unit_list_suites(FILE * fp, 
		      unit_context_t * ctx)
{
  unit_suite_t * suite = ctx->first_suite;
  while(suite != NULL) 
  {
    if(suite->active) 
    {
      if(!ctx->verbose_level) 
      {
	fprintf(fp, "%s\n", suite->name);
      }
      else 
      {
	unit_test_t * tst = suite->first_test;
	while(tst != NULL) 
	{
	  fprintf(fp, "%s::%s\n", suite->name, tst->name);
          tst = tst->next;
	}

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
      fprintf(fp, "%s%zu / %zu suites deactivated%s\n", 
              COLOR_BLUE,
              ctx->n_suites_deactivated,
              n,
              COLOR_NORMAL);
      fprintf(fp, COLOR_BLUE);
    }
    else 
    {
      fprintf(fp, "%zu / %zu suites deactivated\n", ctx->n_suites_deactivated, n);
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
      fprintf(fp, "%s%zu / %zu tests deactivated%s\n", 
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



