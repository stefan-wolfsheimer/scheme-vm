#ifndef __ASSERTION_H__
#define __ASSERTION_H__

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
/** 
 * Free the assertion if it has only one reference.
 * @return next assertion in the list
 */
assertion_t * assertion_free(assertion_t * assertion);


/** 
 * Invert the assertion. 
 */
assertion_t * assertion_invert(assertion_t * assertion);


#endif
