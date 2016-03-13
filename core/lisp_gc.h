#ifndef __LISP_GC__
#define __LISP_GC__

#include "lisp_types.h"

typedef struct lisp_gc_object_t
{
  lisp_cons_t ** cons_pages;
  lisp_cons_t ** root_conses;
  lisp_size_t    n_root_conses;
  lisp_cons_t ** conses;
  lisp_size_t    cons_black_top;
  lisp_size_t    cons_white_top;
  lisp_size_t    cons_grey_top;
  lisp_size_t    cons_void_top;
  /** 
      0              ... cons_white_top-1  white
      cons_white_top ... cons_grey_top-1   grey
      cons_grey_top  ... cons_black_top-1  black
  */
} lisp_gc_object_t;

typedef struct lisp_gc_t
{
} lisp_gc_t;


void lisp_gc_set_root(lisp_gc_t * gc, lisp_cons_t * obj);
void lisp_gc_unset_root(lisp_gc_t * gc, lisp_cons_t * obj);
//short lisp_gc_grey_next_child();



#endif
