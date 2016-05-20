#ifndef __XMALLOC_H__
#define __XMALLOC_H__
/** @file xmalloc.h 
 */
#include <stdlib.h>
#include "assertion.h"
/** 
 *  A chunk of memory managed by the memory checker
 */
typedef struct memchecker_chunk_t
{
  void  * ptr;
  char  * alloc_file; /** if alloc_file != NULL and free_file == NULL
                          the chunk is still reachable */
  int     alloc_line;

  char  * free_file; /** if alloc_file != NULL and free_file != NULL
                         the chunk is freed */
  int     free_line;
} memchecker_chunk_t;

typedef struct memchecker_t
{
  memchecker_chunk_t      * chunks;
  size_t                    n_chunks;
  assertion_t             * first_assertion;
  assertion_t             * last_assertion;
  int                       enabled;
} memchecker_t;

#ifdef DEBUG

#define MALLOC(SIZE) memcheck_debug_malloc(__FILE__,              \
					   __LINE__,              \
					   (SIZE))
#define REALLOC(PTR,SIZE) memcheck_debug_realloc(  __FILE__, __LINE__,  \
						   (PTR),		\
						   (SIZE))
#define FREE(PTR) memcheck_debug_free(__FILE__,__LINE__,(PTR))
#else
#define MALLOC(SIZE)                malloc((SIZE))
#define REALLOC(PTR,SIZE)           realloc((PTR),(SIZE))
#define FREE(PTR)                   free((PTR))
#endif

/**
 * Begin a context of managed memory allocations.
 */
memchecker_t * memcheck_begin();

/** 
 * End a context of managed memory allocations
 */
int memcheck_end();

/** 
 * Get the current memcheck context.
 * Returns NULL is context has not been created with memcheck_begin
 * @retiurn current context or NULL
 */
memchecker_t * memcheck_current();

/** 
 * Disable or enable allocation tracking for the current context.
 * @return non zero if tracking was enabled 
 */
int memcheck_enable(int enable);



/*********************************************************************
 * 
 * Check functions 
 *
 *********************************************************************/
/** 
 * Remove assertions from current context.
 * For all chunks that haven't been freed so far an assertion is returned.
 * @return a single-linked list of assertions
 */
assertion_t * memcheck_finalize();

/** 
 * Remove the first assertion from current context.
 * @return assertion
 */
assertion_t * memcheck_remove_first_assertion();


/*********************************************************************
 * 
 * Mock functions 
 *
 *********************************************************************/
/** 
 * Register a mock for expected allocations
 */
void memcheck_expected_alloc(int success);

/** 
 * Check if all registered mocks have been consumed 
 * @return 0 if there are pendinding mocks
int memcheck_check_mocks(memchecker_t * memchecker);
 */

/** 
 *  Remove all registered mocks from the memchecker 
 *  @return number of removed mocks
 */
size_t memcheck_retire_mocks();


/*********************************************************************
 * 
 * Allocation functions
 *
 *********************************************************************/
void memcheck_register_alloc( const char * file,
                              int          line,
                              void       * ptr);

int memcheck_register_freed(  const char * file,
			      int          line,
			      void       * ptr);

void * memcheck_debug_malloc(  const char     * file,
                               int              line,
                               size_t           size);

void * memcheck_debug_realloc( const char     * file,
                               int              line,
                               void           * ptr,
                               size_t           size);

void memcheck_debug_free( const char    * file,
                          int             line,
                          void          * ptr);

#endif
