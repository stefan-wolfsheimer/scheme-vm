#include "context.h"
#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "core/lisp_vm.h"
#include "core/lisp_eval.h"
#include "core/lisp_symbol.h"
#include "core/lisp_lambda.h"


static void _test_object_destructor(lisp_vm_t * vm, void * ptr)
{
  int * flags = ((lisp_test_object_t*) ptr)->flags;
  if(flags) 
  {
    *flags = TEST_OBJECT_STATE_FREE;
  }
  FREE_OBJECT(ptr);
}

lisp_unit_context_t * lisp_create_unit_context(lisp_vm_param_t     * param,
                                               struct unit_test_t  * tst)
{
  lisp_unit_context_t * ret;
  ret = MALLOC(sizeof(lisp_unit_context_t));
  memcheck_begin();
  ret->vm = lisp_create_vm(param);
  ret->env = lisp_create_eval_env(ret->vm);
  ret->tst = tst;
  ret->data = lisp_nil;

  lisp_register_object_type(ret->vm,
                            "TEST",
                            _test_object_destructor,
                            &ret->test_object_id);


  return ret;
}

void lisp_free_unit_context(lisp_unit_context_t * context)
{
  lisp_free_eval_env(context->env);
  lisp_free_vm(context->vm);
  ASSERT_MEMCHECK(context->tst);
  memcheck_end();
  FREE(context);
}

static lisp_cell_t * _create_new_cell(lisp_unit_context_t * ctx)
{
  if(LISP_IS_NIL(&ctx->data)) 
  {
    lisp_make_cons_root(ctx->vm, &ctx->data);
  }
  else 
  {
    lisp_cell_t tmp;
    lisp_cons_unroot(ctx->vm, LISP_AS(&ctx->data, lisp_cons_t));
#if 0
    /* @todo check bug:  
       valgrind test lambda
     */
    tmp = ctx->data;
    lisp_make_cons_root_car_cdr(ctx->vm, 
                                &ctx->data,
                                &lisp_nil,
                                &tmp);
#else
    lisp_make_cons_root_car_cdr(ctx->vm, 
                                &tmp,
                                &lisp_nil,
                                &ctx->data);
    lisp_copy_object_as_root(ctx->vm, &ctx->data, &tmp);
#endif

  }
  return LISP_CAR(&ctx->data);
}

lisp_cell_t * INTEGER(lisp_unit_context_t * ctx, 
                      lisp_integer_t        value)
{
  lisp_cell_t * ret = _create_new_cell(ctx);
  lisp_make_integer(ret, value);
  return ret;
}

lisp_cell_t * TEST_OBJECT(lisp_unit_context_t * ctx)
{
  lisp_cell_t * ret = _create_new_cell(ctx);
  lisp_test_object_t * obj = MALLOC_OBJECT(sizeof(lisp_test_object_t), 1);
  obj->flags = NULL;
  ret->type_id = ctx->test_object_id;
  ret->data.ptr = obj;
  return ret;
}

lisp_cell_t * SYMBOL(lisp_unit_context_t * ctx,
                     const char          * str)
{
  lisp_cell_t * ret = _create_new_cell(ctx);
  lisp_make_symbol(ctx->vm, ret, str);
  return ret;
}

lisp_cell_t * LIST(lisp_unit_context_t * ctx, ...)
{
  lisp_cell_t * ret = _create_new_cell(ctx);
  lisp_cell_t * arg;
  va_list       vl;
  lisp_cons_t * last = NULL;
  va_start(vl, ctx);
  arg = va_arg(vl, lisp_cell_t*);
  while(arg != NULL)
  {
    if(last == NULL) 
    {
      lisp_make_cons_car_cdr(ctx->vm, ret, arg, &lisp_nil);
      last = LISP_AS(ret, lisp_cons_t);
    }
    else 
    {
      lisp_cell_t tmp;
      lisp_make_cons_car_cdr(ctx->vm, &tmp, arg, &lisp_nil);
      lisp_cons_set_car_cdr(ctx->vm, last, NULL, &tmp);
      last = LISP_AS(&tmp, lisp_cons_t);
    }
    arg = va_arg(vl, lisp_cell_t*);
  }
  va_end(vl);
  return ret;
}

lisp_cell_t * BUILTIN(lisp_unit_context_t * ctx,
                      lisp_builtin_function_t   func,
                      ...)
{
  lisp_size_t   args_size = 0;
  lisp_cell_t * args      = NULL;
  lisp_cell_t * ret       = _create_new_cell(ctx);
  lisp_cell_t * arg;
  va_list       vl;
  va_list       vl2;
  lisp_size_t   i;
  va_start(vl, func);
  va_copy(vl2, vl);
  arg = va_arg(vl, lisp_cell_t*);
  while(arg != NULL)
  {
    args_size++;
  }
  va_end(vl);
  if(args_size) 
  {
    args = MALLOC(sizeof(lisp_cell_t) * args_size);
    arg = va_arg(vl2, lisp_cell_t*);
    i = 0;
    while(arg != NULL)
    {
      lisp_copy_object(ctx->vm, &args[i++], arg);
      args_size++;
      arg = va_arg(vl2, lisp_cell_t*);
    }
  }
  lisp_make_builtin_lambda(ctx->vm,
                           ret,
                           args_size,
                           args,
                           func);
  return ret;
}

