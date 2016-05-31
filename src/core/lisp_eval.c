#include "lisp_eval.h"
#include "util/xmalloc.h"
#include "util/assertion.h"
#include "core/lisp_symbol.h"
#include "config.h"

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
    env->vm         = vm;
    env->max_values = LISP_EVAL_DEFAULT_MAX_VALUES;
    env->n_values   = 0;
    env->values     = values;

    /// @todo registration function in separate module 
    //lisp_register_builtin_function(env, "+", 
    //				   &lisp_builtin_plus);
    //register_form(vm, "define", LISP_TID_FDEFINE);
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
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  REQUIRE_NEQ_PTR(env, NULL);
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
  lisp_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_lambda_t), 0);
  cell.type_id = LISP_TID_LAMBDA;
  cell.data.ptr = lambda;
  lambda->func = func;
  lisp_make_symbol(env->vm, &symbol, name);
  lisp_symbol_set(env->vm, symbol.data.ptr, &cell);
  return LISP_OK;
}

static int _lisp_eval_atom(lisp_eval_env_t   * env,
			   const lisp_cell_t * expr)
{
  if(LISP_IS_NIL(expr)) 
  {
    /* @todo add message to exception */
    int ret = lisp_make_cons_root_typed_car_cdr(env->vm,
						env->values,
						LISP_TID_EVAL_ERROR,
						expr,
						NULL);
    if(ret) 
    {
      return ret;
    }
    else 
    {
      env->n_values = 1;
      return LISP_EVAL_ERROR;
    }
  }
  else 
  {
    env->values->type_id = expr->type_id;
    env->values->data    = expr->data;
    env->n_values        = 1;
    return LISP_OK;
  }
}

static int _lisp_eval_symbol(lisp_eval_env_t   * env,
			     const lisp_cell_t * expr)
{
  const lisp_cell_t * obj = lisp_symbol_get(env->vm, 
					    LISP_AS(expr, lisp_symbol_t));
  if(obj == NULL) 
  {
    /* @todo undefine symbol exception */
    return LISP_UNDEFINED;
  }
  else 
  {
    env->n_values        = 1;
    return lisp_copy_object_as_root( env->vm,
				     env->values,
				     obj);
  }
}


static int _lisp_eval_object(lisp_eval_env_t   * env,
			     const lisp_cell_t * expr)
{
  ++LISP_REFCOUNT(expr);
  env->values->type_id = expr->type_id;
  env->values->data    = expr->data;
  env->n_values        = 1;
  return LISP_OK;
}

static int _lisp_eval_cons(lisp_eval_env_t   * env,
			   const lisp_cell_t * expr)
{
  lisp_cell_t car;
  int ret = LISP_UNSUPPORTED;
  lisp_eval(env, &LISP_AS(expr, lisp_cons_t)->car);
  lisp_copy_object_as_root(env->vm, &car, env->values);
  switch(env->values->type_id) 
  {
  case LISP_TID_LAMBDA:
    /* @todo optimize copy */
    /* @todo distinguish between bytecode and external function */
    ret = lisp_eval_lambda(env,
			   LISP_AS(&car, lisp_lambda_t),
			   &LISP_AS(expr, lisp_cons_t)->cdr);
    break;
  case LISP_TID_FORM:
    /* @todo test case */
    ret = lisp_eval_form(env,
			   LISP_AS(&car, lisp_lambda_t),
			   &LISP_AS(expr, lisp_cons_t)->cdr);
    break;
  default:
    break;
  }
  /* @todo check if car is root object */
  lisp_unset_object_root(env->vm, &car);
  return ret;
}

int lisp_eval(lisp_eval_env_t   * env,
	      const lisp_cell_t * expr)
{
  /* reset old values */
  lisp_size_t i;
  for(i = 0; i < env->n_values; i++) 
  {
    lisp_unset_object_root(env->vm, &env->values[i]);
  }
  env->n_values = 0;
  if(LISP_IS_ATOM(expr)) 
  {
    return _lisp_eval_atom(env,expr);
  }
  else if(LISP_IS_SYMBOL(expr)) 
  {
    /* needs to be checked before object because 
       symbol is object as well */
    return _lisp_eval_symbol(env,expr);
  }
  else if(LISP_IS_OBJECT(expr)) 
  {
    return _lisp_eval_object(env,expr);
  }
  else if(LISP_IS_CONS(expr)) 
  {
    return _lisp_eval_cons(env,expr);
  }
  else 
  {
    /* @todo exception */
    return LISP_UNSUPPORTED;
  }
}
