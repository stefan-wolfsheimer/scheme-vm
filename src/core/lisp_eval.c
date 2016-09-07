#include "lisp_eval.h"
#include "util/xmalloc.h"
#include "util/assertion.h"
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"
#include "core/lisp_asm.h"
#include "config.h"



static void _init_halt(lisp_eval_env_t * env)
{
  lisp_byte_code_t * byte_code = MALLOC_OBJECT(sizeof(lisp_byte_code_t) + 
                                               LISP_SIZ_HALT,
                                               1);
  byte_code->instr_size = LISP_SIZ_HALT;
  ((lisp_instr_t*) &byte_code[1])[0] = LISP_ASM_HALT;
  lisp_make_cons_typed(env->vm, &env->halt_lambda, LISP_TID_LAMBDA);
  LISP_CAR(&env->halt_lambda)->type_id  = LISP_TID_OBJECT;
  LISP_CAR(&env->halt_lambda)->data.ptr = byte_code;
  lisp_push_call(env, 
                 LISP_AS(&env->halt_lambda,
                         lisp_lambda_t),
                 (lisp_instr_t*)&LISP_AS(LISP_CAR(&env->halt_lambda),
                                         lisp_byte_code_t)[1]);
}

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
    env->vm              = vm;

    env->max_values      = LISP_EVAL_DEFAULT_MAX_VALUES;
    env->n_values        = 0;
    env->values          = values;


    env->max_stack_size  = LISP_MAX_STACK_SIZE;
    env->stack_size      = 0;
    env->stack           = NULL;
    env->stack_top       = 0;

    env->max_call_stack_size  = LISP_MAX_CALL_STACK_SIZE;
    env->call_stack      = NULL;
    env->call_stack_top  = 0;
    env->call_stack_size = 0;
    _init_halt(env);
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
  lisp_unset_object(env->vm, &env->exception);
  lisp_unset_object(env->vm, &env->halt_lambda);
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
  if(env->call_stack)
  {
    FREE(env->call_stack);
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
  if(env->stack_top >= env->stack_size) 
  {
    if(env->stack_size == 0) 
    {
      env->stack_size = LISP_STACK_INIT_BLOCK_SIZE;
    }
    else 
    {
      if((env->stack_size << 1) > env->max_stack_size) 
      {
        return LISP_STACK_OVERFLOW;
      }
      env->stack_size = env->stack_size << 1;
    }
    env->stack = REALLOC(env->stack, sizeof(lisp_cell_t) * env->stack_size);
    if(!env->stack) 
    {
      return LISP_ALLOC_ERROR;
    }
  }
  return lisp_copy_object_as_root(env->vm,
                                  &env->stack[env->stack_top++],
                                  cell);  
}

int lisp_push_call(lisp_eval_env_t * env,
                   lisp_lambda_t   * lambda,
                   lisp_instr_t    * next_instr)
{
  if(env->call_stack_top >= env->call_stack_size) 
  {
    if(env->call_stack_size == 0) 
    {
      env->call_stack_size = LISP_CALL_STACK_INIT_BLOCK_SIZE;
    }
    else 
    {
      if((env->call_stack_size << 1) > env->max_call_stack_size) 
      {
        return LISP_STACK_OVERFLOW;
      }
      env->call_stack_size = env->call_stack_size << 1;
    }

    env->call_stack = REALLOC(env->call_stack,
                              sizeof(lisp_call_stack_entry_t) *
                              env->call_stack_size);
    if(!env->call_stack) 
    {
      return LISP_ALLOC_ERROR;
    }
  }
  env->call_stack[env->call_stack_top].lambda = lambda;
  env->call_stack[env->call_stack_top++].next_instr = next_instr;
  env->exception = lisp_nil;
  return LISP_OK;
}

int lisp_push_halt(lisp_eval_env_t * env)
{
  return lisp_push_call(env,
                        LISP_AS(&env->halt_lambda,
                                lisp_lambda_t),
                        (lisp_instr_t*)&LISP_AS(LISP_CAR(&env->halt_lambda),
                                                lisp_byte_code_t)[1]);
}
