#include "lisp_assertion.h"
#include "util/xmalloc.h"
#include "util/xstring.h"
#include <string.h>

assertion_t * assertion_create_lisp_code(const char * file,
                                         int          line,
                                         const char * expr,
                                         int          actual,
                                         int          code,
                                         const char * op,
                                         int          only_on_failure)
{
  assertion_t * ret = NULL;
  int success =
    ( !strcmp(op,"==") && actual == code) ||
    ( !strcmp(op,"!=") && actual != code) ||
    ( !strcmp(op,"<")  && actual < code)  ||
    ( !strcmp(op,">")  && actual > code)  ||
    ( !strcmp(op,"<=") && actual <= code) ||
    ( !strcmp(op,">=") && actual >= code);
  if(!success || !only_on_failure)
  {
    ret = assertion_create(file, line);
    ret->success = (actual == code);
    int old = memcheck_enable(0);
    ret->expect = alloc_sprintf("%s%s%s",
                                expr, op,
                                lisp_error_message(code));
    ret->expect_explain = alloc_sprintf("actual %s",
                                        lisp_error_message(actual));
    memcheck_enable(old);
  }
  return ret;
}
