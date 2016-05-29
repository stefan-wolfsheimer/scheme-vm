#ifndef __BUILTIN_FORMS_H__
#define __BUILTIN_FORMS_H__

struct lisp_vm_t;
struct lisp_cell_t;

int lisp_make_form_define(struct lisp_vm_t * vm, 
			  struct lisp_cell_t * cell);

#endif
