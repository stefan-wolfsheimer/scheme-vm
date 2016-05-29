#ifndef __LISP_ARITHMETIC_H__
#define __LISP_ARITHMETIC_H__

struct lisp_vm_t;
struct lisp_cell_t;


int lisp_make_func_plus(struct lisp_vm_t * vm, 
			struct lisp_cell_t * cell);


#endif
