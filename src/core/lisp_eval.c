#include "lisp_eval.h"
#include "util/xmalloc.h"
#include "util/assertion.h"
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"
#include "config.h"

/* @TODO call stack 
 * @TODO dummy halt function at the bottom of the call stack
 */
lisp_eval_env_t * lisp_create_eval_env(lisp_vm_t * vm)
{
  REQUIRE_NEQ_PTR(vm, NULL);
  lisp_eval_env_t * env;
  lisp_cell_t     * values;
  lisp_size_t       i;
  values = MALLOC(sizeof(lisp_cell_t) * LISP_EVAL_DEFAULT_MAX_VALUES);
  if(values == NULL) 
  {
    return NULL;
  }
  env = MALLOC(sizeof(lisp_eval_env_t));
  if(env != NULL) 
  {
    for(i = 0; i < LISP_EVAL_DEFAULT_MAX_VALUES; i++) 
    {
      values[i] = lisp_nil;
    }
    env->vm             = vm;
    env->max_values     = LISP_EVAL_DEFAULT_MAX_VALUES;
    env->n_values       = 0;
    env->values         = values;
    env->stack_top      = 0;
    env->max_stack_size = LISP_MAX_STACK_SIZE;
    env->stack_size     = 0;
    env->stack          = NULL;
    for(i = 0; i < env->stack_size; i++) 
    {
      env->stack[i] = lisp_nil;
    }
    /* @todo make dynamic extension of stack and use relative offsets in 
             order to not invalidate pointers when reallocating the 
             stack */
  }
  else 
  {
    FREE(values);
  }
  return env;
}

void lisp_free_eval_env(lisp_eval_env_t * env)
{
  lisp_size_t i;
  REQUIRE_NEQ_PTR(env, NULL);
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  if(env->stack != NULL) 
  {
    for(i = 0; i < env->stack_top; i++) 
    {
      lisp_unset_object_root(env->vm, &env->stack[i]);
    }
    FREE(env->stack);
  }
  FREE(env->values);
  FREE(env);
}

/* @todo refactor */
int lisp_register_builtin_function(lisp_eval_env_t        * env,
				   const char             * name,
				   lisp_builtin_function_t  func)
{
  lisp_cell_t symbol;
  lisp_cell_t cell;
  /* @todo error check */
  /* @todo create lisp_builtin_function without regististration */
  lisp_byte_code_t * lambda = MALLOC_OBJECT(sizeof(lisp_byte_code_t), 0);
  cell.type_id = LISP_TID_LAMBDA;
  cell.data.ptr = lambda;
  //lambda->func = func;
  //lambda->data_size = 0;
  lambda->instr_size = 0;
  lisp_make_symbol(env->vm, &symbol, name);
  lisp_symbol_set(env->vm, symbol.data.ptr, &cell);
  return LISP_OK;
}

int lisp_push_integer(lisp_eval_env_t * env,
                      lisp_integer_t    value)
{
  lisp_cell_t tmp;
  lisp_make_integer(&tmp, value);
  return lisp_push(env, &tmp);
}

int lisp_push(lisp_eval_env_t * env,
              const lisp_cell_t * cell)
{
  if(env->stack_top <= env->stack_size) 
  {
    if(env->stack_size == 0) 
    {
      env->stack_size = LISP_STACK_INIT_BLOCK_SIZE;
    }
    else 
    {
      env->stack_size = env->stack_size << 1;
    }
    env->stack = REALLOC(env->stack, sizeof(lisp_cell_t) * env->max_stack_size);
    if(!env->stack) 
    {
      return LISP_ALLOC_ERROR;
    }
  }
  return lisp_copy_object_as_root(env->vm, &env->stack[env->stack_top++], cell);  
}
