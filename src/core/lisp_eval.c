#include "lisp_eval.h"
#include "lisp_builtin.h"
#include "util/xmalloc.h"
#include "util/assertion.h"

static void register_form(lisp_vm_t * vm, 
			  const char * name,
			  lisp_integer_t id)
{
  lisp_cell_t symbol;
  lisp_cell_t cell;
  cell.type_id = id;
  lisp_make_symbol(vm, &symbol, name);
  lisp_symbol_set(vm, symbol.data.ptr, &cell);
}

lisp_eval_env_t * lisp_create_eval_env(lisp_vm_t * vm)
{
  REQUIRE_NEQ_PTR(vm, NULL);
  lisp_eval_env_t * env;
  env = MALLOC(sizeof(lisp_eval_env_t));
  if(env != NULL) 
  {
    env->vm = vm;
    lisp_register_builtin_function(env, "+", 
				   &lisp_builtin_plus);
    register_form(vm, "define", LISP_TID_FDEFINE);
  }
  return env;
}

void lisp_free_eval_env(lisp_eval_env_t * env)
{
  REQUIRE_NEQ_PTR(env, NULL);
  FREE(env);
}

int lisp_register_builtin_function(lisp_eval_env_t        * env,
				   const char             * name,
				   lisp_builtin_function_t  func)
{
  lisp_cell_t symbol;
  lisp_cell_t cell;
  /* @todo error check */
  lisp_builtin_lambda_t * lambda = MALLOC_OBJECT(sizeof(lisp_builtin_lambda_t), 0);
  cell.type_id = LISP_TID_BUILTIN_LAMBDA;
  cell.data.ptr = lambda;
  lambda->func = func;
  lisp_make_symbol(env->vm, &symbol, name);
  lisp_symbol_set(env->vm, symbol.data.ptr, &cell);
  return LISP_OK;
}

static int lisp_eval_builtin(lisp_vm_t                   * vm,
			     lisp_cell_t                 * cell,
			     const lisp_builtin_lambda_t * func,
			     const lisp_cell_t           * args)
{
  /* @todo: define argument signature class
     @todo: eval instead of copy */
  const lisp_cell_t * current = args;
  lisp_integer_t n = 0;
  while(!LISP_IS_NIL(current)) 
  {
    if(LISP_IS_CONS_OBJECT(current)) 
    {
      n++;
      current = &LISP_AS(current, lisp_cons_t)->cdr;
    }
    else 
    {
      /* @todo error */
    }
  }
  lisp_cell_t * stack_frame;
  stack_frame = MALLOC(sizeof(lisp_cell_t) * (n+1));
  lisp_make_integer(stack_frame, n);
  n = 1;
  current = args;
  while(!LISP_IS_NIL(current)) 
  {
    if(LISP_IS_CONS_OBJECT(current)) 
    {
      /* @todo eval arguements instead of copy */
      lisp_copy_object_as_root(vm, &stack_frame[n], 
			       &LISP_AS(current, lisp_cons_t)->car);
      current = &LISP_AS(current, lisp_cons_t)->cdr;
      n++;
    }
    else 
    {
      /* @todo error */
    }
  }
  func->func(vm, 
	     cell,
	     stack_frame);
  /* @todo unset root objects */
  FREE(stack_frame);
  return LISP_OK;
}

static int lisp_eval_define(lisp_eval_env_t   * env,
			    const lisp_cell_t * args)
{
  //LISP_IS_CONS(&LISP_AS(expr, lisp_cons_t)->cdr)
  /* define */
  if(LISP_IS_CONS(args))
  {
    lisp_cell_t car_cdr;
    lisp_cell_t car_cdr_cdr;
    lisp_eval(env, &car_cdr, &LISP_AS(args, lisp_cons_t)->car);
    if(LISP_IS_SYMBOL(&LISP_AS(args, lisp_cons_t)->car)) 
    {
      /* (define a X) */
      //lisp_cell_t obj;
      lisp_eval(env, &car_cdr_cdr, 
		&LISP_AS(
			 &LISP_AS(args, lisp_cons_t)->cdr,
			 lisp_cons_t)->car);
      lisp_symbol_set(env->vm,
		      LISP_AS(
			      &LISP_AS(args, lisp_cons_t)->car,
			      lisp_symbol_t),
		      &car_cdr_cdr);
      return LISP_OK;
    }
  }
  return LISP_OK;
}

static int _lisp_eval_atom(lisp_eval_env_t   * env,
			   lisp_cell_t       * cell,
			   const lisp_cell_t * expr)
{
  if(LISP_IS_NIL(expr)) 
  {
    /* @todo add message to exception */
    int ret = lisp_make_cons_root_typed_car_cdr(env->vm,
						cell,
						LISP_TID_EVAL_ERROR,
						expr,
						NULL);
    if(ret) 
    {
      return ret;
    }
    else 
    {
      return LISP_EVAL_ERROR;
    }
  }
  else 
  {
    cell->type_id = expr->type_id;
    cell->data = expr->data;
    return LISP_OK;
  }
}

static int _lisp_eval_symbol(lisp_eval_env_t   * env,
			     lisp_cell_t       * cell,
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
    /* @todo check if cell needs to be root object ???*/
    return lisp_copy_object( env->vm,
			     cell,
			     obj);
  }
}


static int _lisp_eval_object(lisp_eval_env_t   * env,
			     lisp_cell_t       * cell,
			     const lisp_cell_t * expr)
{
  ++LISP_REFCOUNT(expr);
  cell->type_id = expr->type_id;
  cell->data    = expr->data;
  return LISP_OK;
}

static int _lisp_eval_cons(lisp_eval_env_t   * env,
			   lisp_cell_t       * cell,
			   const lisp_cell_t * expr)
{
  lisp_cell_t car;
  int ret = LISP_UNSUPPORTED;
  lisp_eval(env, &car, &LISP_AS(expr, lisp_cons_t)->car);
  switch(car.type_id) 
  {
  case LISP_TID_BUILTIN_LAMBDA:
    ret = lisp_eval_builtin(env->vm,
			    cell,
			    LISP_AS(&car, lisp_builtin_lambda_t),
			    &LISP_AS(expr, lisp_cons_t)->cdr);
    break;
  case LISP_TID_LAMBDA:
    /* @todo */
    break;
  case LISP_TID_FDEFINE:
    ret = lisp_eval_define(env, &LISP_AS(expr, lisp_cons_t)->cdr);
    break;
  default:
    break;
  }
  /* @todo check if car is root object */
  lisp_unset_object(env->vm, &car);
  return ret;
}

int lisp_eval(lisp_eval_env_t   * env,
	      lisp_cell_t       * cell,
	      const lisp_cell_t * expr)
{
  *cell = lisp_nil;
  if(LISP_IS_ATOM(expr)) 
  {
    return _lisp_eval_atom(env,cell,expr);
  }
  else if(LISP_IS_SYMBOL(expr)) 
  {
    /* needs to be checked before object because 
       symbol is object as well */
    return _lisp_eval_symbol(env,cell,expr);
  }
  else if(LISP_IS_OBJECT(expr)) 
  {
    return _lisp_eval_object(env,cell,expr);
  }
  else if(LISP_IS_CONS(expr)) 
  {
    return _lisp_eval_cons(env,cell,expr);
  }
  else 
  {
    /* @todo exception */
    return LISP_UNSUPPORTED;
  }
}
