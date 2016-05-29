#ifndef __LISP_VALUES_H__
#define __LISP_VALUES_H__

struct lisp_vm_t;
struct lisp_cell_t;

int lisp_make_func_values(struct lisp_vm_t * vm, 
			  struct lisp_cell_t * cell);

#endif

