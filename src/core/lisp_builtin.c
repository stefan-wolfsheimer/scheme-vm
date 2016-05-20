#include "lisp_builtin.h"
#include "util/assertion.h"

int lisp_builtin_plus(lisp_vm_t    * vm,
		      lisp_cell_t  * target,
		      lisp_cell_t  * stack)
{
  REQUIRE(LISP_IS_INTEGER(stack));
  lisp_integer_t nargs = stack->data.integer;
  lisp_integer_t i;
  target->type_id = LISP_TID_INTEGER;
  target->data.integer = 0;

  for(i = 0; i < nargs; i++) 
  {
    if(!LISP_IS_INTEGER(&stack[i+1])) 
    {
      *target = lisp_nil;
      return LISP_TYPE_ERROR;
    }
    target->data.integer+= stack[i+1].data.integer;
  }
  return LISP_OK;
}
