#include "lisp_vm.h"
#include "lisp_asm.h"
#include <string.h>

/*******************************************************************
 * 
 * lambda
 * 
 *******************************************************************/

int lisp_make_builtin_lambda(lisp_vm_t   * vm,
			     lisp_cell_t * cell,
			     lisp_size_t   args_size,
			     lisp_cell_t * args,
			     lisp_builtin_function_t  func)
{
  lisp_builtin_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_builtin_lambda_t),
						 1);
  cell->type_id  = LISP_TID_BUILTIN_LAMBDA;
  cell->data.ptr = lambda;
  lambda->func   = func;
  /* @todo alloc check */
  /* @todo arguments */
  return LISP_OK;
}

int lisp_make_builtin_lambda_opt_args(lisp_vm_t   * vm,
				      lisp_cell_t * cell,
				      lisp_size_t   args_size,
				      lisp_cell_t * args,
				      lisp_size_t   opt_args_size,
				      lisp_cell_t * opt_args,
				      lisp_size_t   named_args_size,
				      lisp_cell_t * named_args,
				      lisp_cell_t * named_args_values,
				      int           has_rest_args,
				      lisp_builtin_function_t  func)
{
  /* @todo implement */
  return LISP_OK;
}



/*******************************************************
@todo refactor rest
********************************************************/
int lisp_make_lambda_instr(lisp_vm_t          * vm, 
                           lisp_cell_t        * cell, 
                           lisp_size_t          args_size,
                           lisp_cell_t        * data,
                           lisp_size_t          data_size,
                           const lisp_instr_t * instr)
{
  lisp_size_t instr_size = 0;
  const lisp_instr_t * itr = instr;
  while(*itr != LISP_END) 
  {
    instr_size++;
    itr++;
  }
  return lisp_make_lambda_n_instr(vm, 
                                  cell,
                                  args_size,
                                  data,
                                  data_size, 
                                  instr,
                                  instr_size);
}

int lisp_make_lambda_n_instr(lisp_vm_t          * vm, 
                             lisp_cell_t        * cell, 
                             lisp_size_t          args_size,
                             lisp_cell_t        * data,
                             lisp_size_t          data_size,
                             const lisp_instr_t * instr,
                             lisp_size_t          n_instr)
{
  lisp_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_lambda_t) + 
                                         sizeof(lisp_instr_t) * n_instr + 
                                         sizeof(lisp_cell_t)  * data_size,
                                         1);
  if(!lambda) 
  {
    return 1; /*@todo correct return code for allocation error */
  }
  lambda->args_size   = args_size;
  lambda->instr_size  = n_instr;
  lambda->data_size   = data_size;
  cell->data.ptr      = lambda;
  cell->type_id       = LISP_TID_LAMBDA;
  memcpy((void*) &lambda[1], instr, sizeof(lisp_instr_t) * n_instr);
  return lisp_copy_n_objects_as_root( vm, LISP_LAMBDA_DATA(lambda), data, data_size);
}


/* stacks: 
   [] operand stack of lisp_cell_t
   () call stack of    lisp_call_t
   I,J,K: addresses 
          within the current frame (instruction addr or data)
   data{I}: Ith data constant  of current frame
   N,M:     numbers

   P,Q:     opt code
   instruction set: 
   PUSHD(I):       pushes the constant data{I} on stack
                   [] -> [data{I} ], stack_top -> stack_top + 1
                 
   PUSHC(P, N):    pushes an operation onto the call stack
                   N: number of arguments
                   () -> ((P, N, &next))
   
*/

int lisp_lambda_eval(lisp_vm_t           * thr,
                     const lisp_lambda_t * lambda,
                     lisp_size_t           nargs)
{

  return 0;
}
