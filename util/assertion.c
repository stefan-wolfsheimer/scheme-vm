#include "assertion.h"
#include "xstring.h"
#include "xmalloc.h"
#include <stdlib.h>

assertion_t * assertion_create(const char * file, 
			       int          line)
{
  memcheck_disable();
  assertion_t * assertion = malloc(sizeof(assertion_t));
  if(assertion == NULL) 
  {
    return assertion;
  }
  assertion->expect         = NULL;
  assertion->expect_explain = NULL;
  assertion->file           = alloc_strcpy(file);
  assertion->line           = line;
  assertion->success        = 0;
  assertion->is_exception   = 0;
  assertion->next           = NULL;
  assertion->expect         = NULL;
  memcheck_enable();
  return assertion;
}

assertion_t * assertion_free(assertion_t * assertion)
{
  assertion_t * ret = assertion->next;
  if(assertion->expect) 
  {
    free(assertion->expect);
  }
  if(assertion->expect_explain) 
  {
    free(assertion->expect_explain);
  }
  if(assertion->file)
  {
    free(assertion->file);
  }
  free(assertion);
  return ret;
}

assertion_t * assertion_invert(assertion_t * assertion)
{
  if(assertion) 
  {
    assertion->success = assertion->success ? 0 : 1;
    if(assertion->expect) 
    {
      char * tmp = assertion->expect;
      memcheck_disable();
      if(assertion->is_exception) 
      {
	assertion->expect = alloc_sprintf("expected %s", tmp);
      }
      else 
      {
	assertion->expect = alloc_sprintf("not (%s)", tmp);
      }
      free(tmp);
      memcheck_enable();
    }
    else 
    {
      memcheck_disable();
      assertion->expect = alloc_strcpy("not (...)");
      memcheck_enable();
    }
  }
  return assertion;
}


