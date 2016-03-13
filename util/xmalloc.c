#include "xmalloc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define MEMCHECK_BLOCK_SIZE 4
static memchecker_t ** memcheck_stack = NULL;
static size_t          memcheck_stack_size = 0;

static memchecker_chunk_t * _memcheck_find_chunk(void * ptr);
static int _memcheck_next_mock(const char * file, int line);


memchecker_chunk_t * _memcheck_find_chunk(void * ptr)
{
  size_t i,n;
  if(memcheck_stack_size)
  {
    memchecker_t * memcheck = memcheck_stack[memcheck_stack_size-1];
    for(i = 0; i < memcheck->n_chunks; i++)
    {
      if(memcheck->chunks[i].ptr == ptr)
      {
        return &memcheck->chunks[i];
      }
    }
    if(memcheck->n_chunks % MEMCHECK_BLOCK_SIZE == 0)
    {
      n = memcheck->n_chunks / MEMCHECK_BLOCK_SIZE;
      n++;
      memcheck->chunks = realloc(   memcheck->chunks,
                                    MEMCHECK_BLOCK_SIZE * n *
                                    sizeof(memchecker_chunk_t));
    }
    memcheck->chunks[memcheck->n_chunks].ptr        = ptr;
    memcheck->chunks[memcheck->n_chunks].alloc_file = NULL;
    memcheck->chunks[memcheck->n_chunks].free_file  = NULL;
    memcheck->chunks[memcheck->n_chunks].alloc_line = 0;
    memcheck->chunks[memcheck->n_chunks].free_line  = 0;
    return &memcheck->chunks[memcheck->n_chunks++];
  }
  else
  {
    return NULL;
  }
}

int memcheck_next_mock(const char * file, int line) 
{
  size_t i,n;
  if(memcheck_stack_size)
  {
    memchecker_t * memcheck = memcheck_stack[memcheck_stack_size-1];
    int ret = 1;
    if(memcheck->next_mock != NULL) 
    {
      ret = memcheck->next_mock->success;
      memchecker_alloc_mock_t * next = memcheck->next_mock->next;
      free(memcheck->next_mock);
      memcheck->next_mock = next;
      if(next) 
      {
	next->prev = NULL;
      }
      else 
      {
	memcheck->last_mock = NULL;
      }
    }
    return ret;
  }
  else 
  {
    return 1;
  }
}

void memcheck_register_alloc(  const char * file,
                               int          line,
                               void       * ptr)
{
  memchecker_chunk_t * chunk;
  chunk = _memcheck_find_chunk(ptr);
  if(chunk == NULL)
  {
    return;
  }
  if(chunk->free_file != NULL)
  {
    free(chunk->free_file);
    chunk->free_file = NULL;
    chunk->free_line = 0;
  }
  if(chunk->alloc_file != NULL) 
  {
    free(chunk->alloc_file);
    chunk->alloc_file = NULL;
    chunk->alloc_line = 0;
  }
  chunk->alloc_file = malloc(strlen(file) + 1);
  strcpy(chunk->alloc_file, file);
  chunk->alloc_line = line;
}

#define __FMT_NOT_MANAGED \
  "%s:%d: attempt to free chunk %p that is not managed by memchecker"

void memcheck_register_freed( const char * file,
                              int          line,
                              void       * ptr)
{
  memchecker_chunk_t * chunk;
  chunk = _memcheck_find_chunk(ptr);
  if(chunk == NULL)
  {
    return;
  }
  if(memcheck_stack_size && chunk->alloc_file == NULL)
  {

    memchecker_t * memcheck = memcheck_stack[memcheck_stack_size-1];
    int l = snprintf(NULL,0, __FMT_NOT_MANAGED, file,line, ptr);
    char * msg = malloc(l+1);
    sprintf(msg, __FMT_NOT_MANAGED, file,line, ptr);
    memcheck->message_not_managed = 
      realloc(memcheck->message_not_managed, 
              sizeof(char*) * (memcheck->n_not_managed+1));
    memcheck->message_not_managed[memcheck->n_not_managed++] = msg;
  }
  else
  {
    if(chunk->free_file != NULL)
    {
      fprintf(stderr,
              "fatal:double free detected at "
              "%s:%d\n"
              "chunk %p was previously allocated at %s:%d\n"
              "chunk %p was previously freed at %s:%d\n",
              file,
              line,
              ptr,
              chunk->alloc_file,
              chunk->alloc_line,
              ptr,
              chunk->free_file,
              chunk->free_line);
      free(chunk->free_file);
    }
    chunk->free_file = malloc(strlen(file)+1);
    strcpy(chunk->free_file, file);
    chunk->free_line = line;
  }
}



memchecker_t * memcheck_begin(int verbose)
{
  memcheck_stack_size++;
  memcheck_stack = realloc(memcheck_stack,
			   sizeof(memchecker_t*)*memcheck_stack_size);
  memchecker_t * memchecker = malloc(sizeof(memchecker_t));
  memchecker->chunks = NULL;
  memchecker->chunks   = NULL;
  memchecker->n_chunks = 0;
  memchecker->message_not_managed = NULL;
  memchecker->n_not_managed = 0;
  memchecker->message_double_free = NULL;
  memchecker->n_double_free = 0;
  memchecker->next_mock = NULL;
  memchecker->last_mock = NULL;
  memcheck_stack[memcheck_stack_size-1] = memchecker;
  return memchecker;
}

int memcheck_finalize(int verbose)
{
  size_t                    i;
  memchecker_chunk_t * chunk;
  int ret = 1;
  if(memcheck_stack_size)
  {
    memchecker_t * memcheck = memcheck_stack[memcheck_stack_size-1];
    for(i = 0; i < memcheck->n_chunks; i++)
    {
      chunk = &memcheck->chunks[i];
      if(chunk->alloc_file != NULL)
      {
        if(chunk->free_file == NULL)
        {
          fprintf(stderr, "segment %p allocated at %s:%d still reachable\n",
                  chunk->ptr,
                  chunk->alloc_file,
                  chunk->alloc_line);
          ret = 0;
        }
      }
      if(chunk->alloc_file != NULL)
      {
        free(chunk->alloc_file);
      }
      if(chunk->free_file != NULL)
      {
        free(chunk->free_file);
      }
    }
    for(i = 0; i < memcheck->n_not_managed; i++) 
    {
      free(memcheck->message_not_managed[i]);
    }
    for(i = 0; i < memcheck->n_double_free; i++) 
    {
      free(memcheck->message_double_free[i]);
    }
    free(memcheck->message_not_managed);
    free(memcheck->message_double_free);
    free(memcheck->chunks);
    /* check mocks */
    memchecker_alloc_mock_t * current_mock;
    memchecker_alloc_mock_t * next_mock;
    current_mock = memcheck->next_mock;
    while(current_mock != NULL) 
    {
      /* @todo create error message, if there 
               are pending mocks*/   
      next_mock = current_mock->next;
      free(current_mock);
      current_mock = next_mock;
    }
    free(memcheck);
    memcheck_stack_size--;
    if(!memcheck_stack_size) 
    {
      free(memcheck_stack);
      memcheck_stack = NULL;
    }
    return ret;
  }
  else 
  {
    fprintf(stderr,"memcheck_finalize: not initalized\n");
    return 0;
  }
}

void memcheck_expected_alloc(memchecker_t * memchecker,
			     int success)
{
  memchecker_alloc_mock_t * mock;
  mock = malloc(sizeof(memchecker_alloc_mock_t));
  mock->prev = memchecker->last_mock;
  mock->next = NULL;
  mock->success = success;
  if(mock->prev == NULL)
  {
    memchecker->next_mock = mock;
  }
  else 
  {
    mock->prev->next = mock;
  }
  memchecker->last_mock = mock;
}


void * memcheck_debug_malloc(const char * file, int line, size_t size)
{
  void * ptr = NULL;
  if(memcheck_next_mock(file, line)) 
  {
    ptr = malloc(size);
    if(memcheck_stack_size)
    {
      memcheck_register_alloc(file, line, ptr);
    }
  }
  return ptr;
}

void * memcheck_debug_realloc(const char * file,
                              int          line,
                              void       * ptr,
                              size_t       size)
{
  if(memcheck_next_mock(file, line)) 
  {
    if(ptr == NULL)
    {
      return memcheck_debug_malloc(file, line, size);
    }
    else
    {
      void * old = ptr;
      ptr = realloc(ptr,size);
      if(memcheck_stack_size)
      {
	memcheck_register_freed(file, line, old);
	memcheck_register_alloc(file, line, ptr);
      }
      return ptr;
    }
  }
  else 
  {
    return NULL;
  }
}

void memcheck_debug_free(   const char    * file,
                        int             line,
                        void          * ptr)
{
  if(memcheck_stack_size)
  {
    memcheck_register_freed(file, line, ptr);
  }
  free(ptr);
}



