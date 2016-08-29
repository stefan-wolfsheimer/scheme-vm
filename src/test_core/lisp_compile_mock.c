#include "lisp_compile_mock.h"
#include "util/xmalloc.h"
#include "util/mock.h"
#include "core/lisp_vm.h"
#include "core/lisp_asm.h"

/* mock for lambda calls */
void lisp_init_lambda_mock(lisp_lambda_mock_t * mock, 
			   lisp_vm_t          * vm,
			   size_t               n_values)
{
  size_t i;
  mock->vm       = vm;
  mock->n_values = n_values;
  mock->values   = MALLOC(sizeof(lisp_cell_t)*n_values);
  mock->n_args   = 0;
  mock->args     = NULL;
  for(i = 0; i < mock->n_values; i++) 
  {
    mock->values[i] = lisp_nil;
  }
}

void lisp_free_lambda_mock(lisp_lambda_mock_t * mock)
{
  size_t i;
  for(i = 0; i < mock->n_values; i++) 
  {
    lisp_unset_object_root(mock->vm, &mock->values[i]);
  }
  if(mock->values != NULL) 
  {
    FREE(mock->values);
  }
  for(i = 0; i < mock->n_args; i++) 
  {
    lisp_unset_object_root(mock->vm, &mock->args[i]);
  }
  if(mock->args != NULL) 
  {
    FREE(mock->args);
  }
}

int lisp_lambda_mock_function(struct lisp_eval_env_t * env,
                              const lisp_lambda_t    * lambda,
                              lisp_size_t               nargs)
{
  mock_expected_t * expected;
  lisp_cell_t     * args = env->stack + env->stack_top - nargs;
  expected = mock_get_expected(lisp_lambda_mock_function);
  if(expected != NULL)
  {
    lisp_lambda_mock_t * ret = expected->user_data;
    size_t i;
    ret->n_args = nargs;
    ret->args   = MALLOC(sizeof(lisp_cell_t) * ret->n_args);
    for(i = 0; i < ret->n_args; i++) 
    {
      lisp_copy_object_as_root(env->vm, &ret->args[i], &args[i]);
    }
    env->n_values = ret->n_values;
    /* @todo call resize value register if n_values > max_values */
    for(i = 0; i < ret->n_values; i++) 
    {
      lisp_copy_object_as_root(env->vm, &env->values[i], &ret->values[i]);
    }
    mock_remove(expected);				       
  }
  return LISP_OK;
}

int lisp_compile_phase1_mock_failure(struct lisp_vm_t * vm, 
                                     lisp_size_t      * instr_size,
                                     const lisp_cell_t * expr)
{
  *instr_size = 0;
  return LISP_UNSUPPORTED;
}

int lisp_compile_phase1_mock(struct lisp_vm_t * vm, 
                             lisp_size_t      * instr_size,
                             const lisp_cell_t * expr)
{
  *instr_size = LISP_SIZ_PUSHD + LISP_SIZ_PUSHD + LISP_SIZ_RET;
  return LISP_OK;
}

int lisp_compile_phase2_mock(struct lisp_vm_t  * vm,
                             lisp_cell_t       * cell,
                             lisp_instr_t      * instr,
                             const lisp_cell_t * expr)
{
  lisp_cell_t one ,two;
  lisp_make_integer(&one, 1);
  lisp_make_integer(&two, 2);
  LISP_SET_INSTR(LISP_ASM_PUSHD, instr, lisp_cell_t, one);
  instr+= LISP_SIZ_PUSHD;    
  LISP_SET_INSTR(LISP_ASM_PUSHD, instr, lisp_cell_t, two);
  instr+= LISP_SIZ_PUSHD;    
  *instr = LISP_ASM_RET;
  return LISP_OK;
}

int lisp_compile_phase2_mock_failure(struct lisp_vm_t  * vm,
                                     lisp_cell_t       * cell,
                                     lisp_instr_t      * instr,
                                     const lisp_cell_t * expr)
{
  return LISP_UNSUPPORTED;
}
