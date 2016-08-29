#include "context.h"
#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "util/xstring.h"
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
                            NULL,
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
    lisp_cons_unroot(ctx->vm, LISP_AS(&ctx->data, lisp_cons_t));
#if 0
    /* @todo check bug:  
       valgrind test lambda
     */
    //lisp_cell_t tmp;
    //tmp = ctx->data;
    //lisp_make_cons_root_car_cdr(ctx->vm, 
    //&ctx->data,
    //&lisp_nil,
    //&ctx->data);
    lisp_make_cons_car_cdr(ctx->vm, 
                           &ctx->data,
                           &lisp_nil,
                           &ctx->data);
    lisp_cons_root(ctx->vm, 
                   LISP_AS(&ctx->data, lisp_cons_t));
#else
    lisp_cell_t tmp;
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

lisp_cell_t * NIL(lisp_unit_context_t * ctx)
{
  return LIST(ctx, NULL);
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

lisp_cell_t * FORM(lisp_unit_context_t * ctx,
                   lisp_compile_phase1_t phase1,
                   lisp_compile_phase2_t phase2)
{
  lisp_cell_t * ret       = _create_new_cell(ctx);
  lisp_make_builtin_form(ctx->vm,
                         ret,
                         phase1,
                         phase2);
  return ret;
}



static assertion_t * _assertion_create_asm(lisp_vm_t         * vm,
                                           const char        * file, 
                                           int                 line,
                                           const char        * expr_lhs,
                                           const lisp_cell_t * lhs,
                                           const lisp_cell_t * rhs,
                                           int                 expr)
{
  assertion_t * assertion = assertion_create(file, line);
  if(!assertion) return NULL;    
  assertion->success = expr;
  int old = memcheck_enable(0);
  assertion->expect = alloc_sprintf("asm(%s) == CODE", expr_lhs);
  if(!expr || 1) 
  {
    const lisp_cell_t * curr_lhs = lhs;
    const lisp_cell_t * curr_rhs = rhs;
    size_t n_rows_lhs            = 0;
    size_t n_rows_rhs            = 0;
    size_t max_width_lhs         = 3; /* ... */
    size_t width_rhs             = 0;
    size_t tmp;
    while(LISP_IS_CONS(curr_lhs)) 
    {
      n_rows_lhs++;
      tmp = lisp_object_to_c_str(vm, NULL, 0, LISP_CAR(curr_lhs));
      if(tmp > max_width_lhs) 
      {
        max_width_lhs = tmp;
      }
      curr_lhs = LISP_CDR(curr_lhs);
    }
    while(LISP_IS_CONS(curr_rhs)) 
    {
      n_rows_rhs++;
      width_rhs+= lisp_object_to_c_str(vm, NULL, 0, LISP_CAR(curr_rhs));
      curr_rhs = LISP_CDR(curr_rhs);
    }
    size_t n_rows = n_rows_lhs > n_rows_rhs ? n_rows_lhs : n_rows_rhs;
    size_t n = (max_width_lhs + 10 ) * n_rows + width_rhs;
    size_t i;
    char * str = MALLOC(n + 1);
    char * cstr = str;
    curr_lhs = lhs;
    curr_rhs = rhs;
    while(LISP_IS_CONS(curr_lhs) || LISP_IS_CONS(curr_rhs))
    {
      if(LISP_IS_NIL(curr_lhs))
      {
        sprintf(cstr, "...");
        cstr+= 3;
        for(i = 0; i < max_width_lhs; i++) 
        {
          *cstr = ' ';
          cstr++;
        }
      }
      else if(LISP_IS_CONS(curr_lhs))
      {
        tmp = lisp_object_to_c_str(vm, cstr, n - (cstr - str), LISP_CAR(curr_lhs));
        cstr+= tmp;
        for(i = tmp; i < max_width_lhs; i++) 
        {
          *cstr = ' ';
          cstr++;
        }
        curr_lhs = LISP_CDR(curr_lhs);
      }
      *cstr = ' '; cstr++; 
      *cstr = '='; cstr++; 
      *cstr = '='; cstr++;
      *cstr = ' '; cstr++;
      if(LISP_IS_NIL(curr_rhs))
      {
        sprintf(cstr, "...");
        cstr+= 3;
      }
      else if(LISP_IS_CONS(curr_rhs))
      {
        tmp = lisp_object_to_c_str(vm, cstr, n - (cstr - str), LISP_CAR(curr_rhs));
        cstr+= tmp;
        curr_rhs = LISP_CDR(curr_rhs);
      }
      *cstr = '\n'; cstr++;
    }
    *cstr = '\0';
    assertion->expect_explain = str;
  }
  memcheck_enable(old);
  return assertion;
}

assertion_t * lisp_compare_asm_list(const char          * file,
                                    int                   line,
                                    lisp_unit_context_t * ctx,
                                    const char          * expr_lhs,
                                    const lisp_cell_t   * lhs,
                                    const lisp_cell_t   * rhs)
{
  const lisp_cell_t * curr_lhs = lhs;
  const lisp_cell_t * curr_rhs = rhs;
  while(1)
  {
    if(LISP_IS_NIL(curr_lhs) && 
       LISP_IS_NIL(curr_rhs)) 
    {
      /* OK */
      return _assertion_create_asm(ctx->vm,file, line, expr_lhs, lhs, rhs, 1);
    }
    else if(LISP_IS_NIL(curr_lhs)) 
    {
      return _assertion_create_asm(ctx->vm,file, line, expr_lhs, lhs, rhs, 0);
    }
    else if(LISP_IS_NIL(curr_rhs)) 
    {
      return _assertion_create_asm(ctx->vm,file, line, expr_lhs, lhs, rhs, 0);
    }
    else if(!LISP_IS_CONS(curr_lhs))
    {
      return _assertion_create_asm(ctx->vm,file, line, expr_lhs, lhs, rhs, 0);
    }
    else if(!LISP_IS_CONS(curr_rhs)) 
    {
      return _assertion_create_asm(ctx->vm, file, line, expr_lhs, lhs, rhs, 0);
    }
    else 
    {
      if(LISP_IS_SYMBOL(LISP_CAR(curr_rhs)))
      {
        if(!lisp_eq_object(LISP_CAR(curr_rhs), LISP_CAR(curr_lhs)))
        {
          return _assertion_create_asm(ctx->vm,
                                       file,
                                       line,
                                       expr_lhs,
                                       lhs,
                                       rhs,
                                       0);
        }
      }
      else 
      {
        /*@todo */
      }
      curr_rhs = LISP_CDR(curr_rhs);
      curr_lhs = LISP_CDR(curr_lhs);
    }
  }
  return NULL;
}

struct assertion_t * lisp_compare_return_code(const char          * file,
                                              int                   line,
                                              int                   code_lhs,
                                              int                   code_rhs,
                                              const char          * op)
{
  return NULL;
}


 
assertion_t * lisp_compare_asm(const char          * file,
                               int                   line,
                               lisp_unit_context_t * ctx,
                               const char          * expr_lhs,
                               const lisp_cell_t   * cell,
                               const lisp_cell_t   * structure)
{

  assertion_t * ret = assertion_create_true(file, 
                                            line,
                                            expr_lhs,
                                            LISP_IS_LAMBDA(cell),      
                                            1);
  if(ret) 
  {
    return ret;
  }
  lisp_cell_t disass;
  lisp_lambda_disassemble(ctx->vm, &disass, LISP_AS(cell, lisp_lambda_t));
  ret = lisp_compare_asm_list(file, line, ctx, expr_lhs, &disass, structure);
  lisp_unset_object(ctx->vm, &disass);
  return ret;
}

