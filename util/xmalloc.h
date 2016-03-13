#ifndef __XMALLOC_H__
#define __XMALLOC_H__
/** @file xmalloc.h 
 */
#include <stdlib.h>

typedef struct memchecker_chunk_t
{
  void  * ptr;

  char  * alloc_file; /** if alloc_file != NULL and free_file == NULL
                          the chunk is still reachable */
  int     alloc_line;

  char  * free_file; /** if alloc_file != NULL and free_file != NULL
                         the chunk is freed */
  int free_line;
} memchecker_chunk_t;

typedef struct memchecker_alloc_mock_t 
{
  int success;
  struct memchecker_alloc_mock_t * next;
  struct memchecker_alloc_mock_t * prev;
} memchecker_alloc_mock_t;

typedef struct memchecker_t
{
  memchecker_chunk_t      * chunks;
  size_t                    n_chunks;
  char                   ** message_not_managed;
  size_t                    n_not_managed;
  char                   ** message_double_free;
  size_t                    n_double_free;
  memchecker_alloc_mock_t * next_mock;
  memchecker_alloc_mock_t * last_mock;
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
 * begin a block of managed memory allocations.
 */
memchecker_t * memcheck_begin(int verbose);

/** 
 * Register a mock for expected allocations
 */
void memcheck_expected_alloc(memchecker_t * memcheck,
			     int success);

int memcheck_next_mock(const char * file, int line);

/**
 * finalize a block of managed memory allocations.
 */
int memcheck_finalize(int verbose);

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

void memcheck_free_object( void * ptr);


void memcheck_register_alloc( const char * file,
                              int          line,
                              void       * ptr);

void memcheck_register_freed(  const char * file,
                               int          line,
                               void       * ptr);


#endif
