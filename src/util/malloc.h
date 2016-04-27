#ifndef __XMALLOC_H__
#define __XMALLOC_H__

#include <stdlib.h>
#include "lisp_types.h"

typedef struct memchecker_chunk_t
{
  void        * ptr;

  const char  * alloc_file;
                /** if alloc_file != NULL and free_file == NULL
                    the chunk is still reachable */
  int           alloc_line;

  const char  * free_file;
                /** if alloc_file != NULL and free_file != NULL
                    the chunk is freed */
  int free_line;
} memchecker_chunk_t;

typedef struct memchecker_t
{
  memchecker_chunk_t *  chunks;
  size_t                     n_chunks;
  char                    ** message_not_managed;
  size_t                     n_not_managed;
  char                    ** message_double_free;
  size_t                     n_double_free;
} memchecker_t;

#ifdef DEBUG

#define MALLOC(SIZE) lisp_debug_malloc(__FILE__,              \
                                       __LINE__,              \
                                       (SIZE))
#define MALLOC_OBJECT(SIZE, RCOUNT) lisp_debug_malloc_object(__FILE__,  \
                                                             __LINE__,  \
                                                             (SIZE),    \
                                                             (RCOUNT))
#define REALLOC(PTR,SIZE) lisp_debug_realloc(  __FILE__, __LINE__,  \
                                                 (PTR),             \
                                                 (SIZE))
#define FREE(PTR) lisp_debug_free(__FILE__,__LINE__,(PTR))
#define FREE_OBJECT(PTR) lisp_debug_free_object(__FILE__,__LINE__,(PTR))
#else

#define MALLOC(SIZE)                malloc((SIZE))
#define MALLOC_OBJECT(SIZE, RCOUNT) lisp_malloc_object((SIZE),(RCOUNT))
#define REALLOC(PTR,SIZE)           realloc((PTR),(SIZE))
#define FREE(PTR)                   free((PTR))
#define FREE_OBJECT(PTR)            lisp_free_object((PTR))
#endif

/**
 * begin a block of managed memory allocations.
 */
memchecker_t * memcheck_begin(int verbose);

/**
 * finalize a block of managed memory allocations.
 */
int lisp_memcheck_finalize(int verbose);

void * lisp_debug_malloc(  const char     * file,
                           int              line,
                           size_t           size);

void * lisp_debug_realloc( const char     * file,
                           int              line,
                           void           * ptr,
                           size_t           size);

void lisp_debug_free( const char    * file,
                      int             line,
                      void          * ptr);

/**
 * create object with reference count
 */
void * lisp_debug_malloc_object( const char      * file,
                                 int               line, 
                                 size_t            size,
                                 lisp_ref_count_t  rcount);

void * lisp_malloc_object( size_t           size,
                           lisp_ref_count_t rcount );

void lisp_debug_free_object( const char    * file,
                             int             line,
                             void          * ptr);

void lisp_free_object( void * ptr);


void lisp_memcheck_register_alloc( const char * file,
                                   int          line,
                                   void       * ptr);

void lisp_memcheck_register_freed(  const char * file,
                                    int          line,
                                    void       * ptr);


#endif
