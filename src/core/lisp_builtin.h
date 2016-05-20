#ifndef __LISP_BUILTIN_H__
#define __LISP_BUILTIN_H__
#include "lisp_vm.h"
/* 
 * @todo typecheck for values
 * @todo overflow check and high prec. arithmetric
 */
int lisp_builtin_plus(lisp_vm_t     * vm,
		      lisp_cell_t   * target,
		      lisp_cell_t   * stack);

#endif
