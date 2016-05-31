#include "util/unit_test.h"
#include "util/xmalloc.h"
#include "builtin/builtin_forms.h"
#include "core/lisp_vm.h"
#include "core/lisp_eval.h"
#include "core/lisp_symbol.h"

static void test_define_atom(unit_test_t * tst)
{
  /* (DEFINE A 1) */
  memcheck_begin();
  lisp_vm_t       * vm = lisp_create_vm(&lisp_vm_default_param);
  lisp_eval_env_t * env = lisp_create_eval_env(vm);
  lisp_cell_t       form_define;
  lisp_cell_t       args;
  lisp_cell_t       lst[2];
  lisp_make_symbol(vm, &lst[0], "a");
  lisp_make_integer(   &lst[1], 1);
  lisp_make_list_root(vm, &args, lst, 2);
  ASSERT_EQ_I(tst, lisp_make_form_define(vm, &form_define), LISP_OK);
  ASSERT_EQ_I(tst, lisp_eval_form(env,
				  LISP_AS(&form_define, lisp_lambda_t),
				  &args), LISP_OK);
  lisp_unset_object_root(vm, &args);
  ASSERT_EQ_U(tst, env->n_values, 1u);
  ASSERT_NEQ_PTR(tst, 
		 lisp_symbol_get(vm, LISP_AS(&lst[0], lisp_symbol_t)), 
		 NULL);
  ASSERT(tst,
	 lisp_eq_object(
			lisp_symbol_get(vm, 
					LISP_AS(&lst[0], lisp_symbol_t)),
			&lst[1]));
  lisp_unset_object(vm, &lst[0]);
  lisp_unset_object(vm, &lst[1]);

  lisp_unset_object(vm, &form_define);
  lisp_free_eval_env(env);
  lisp_free_vm(vm);
  ASSERT_MEMCHECK(tst);
  memcheck_end();
}


void test_builtin_forms(unit_context_t * ctx)
{
  unit_suite_t * suite = unit_create_suite(ctx, "builtin_forms");
  TEST(suite, test_define_atom);
}
