#ifndef __LISP_LAMBDA_H__
#define __LISP_LAMBDA_H__

#include "lisp_type.h"
struct lisp_vm_t;


int lisp_make_builtin_lambda(struct lisp_vm_t       * vm,
                             lisp_cell_t            * cell,
                             lisp_size_t              args_size,
                             const lisp_cell_t      * args,
                             lisp_builtin_function_t  func);

int lisp_make_builtin_form(struct lisp_vm_t       * vm,
                           lisp_cell_t            * cell,
                           lisp_compile_phase1_t    phase1,
                           lisp_compile_phase2_t    phase2);

int lisp_make_builtin_c_str(struct lisp_vm_t         * vm,
                            lisp_cell_t              * cell,
                            lisp_size_t                args_size,
                            const char              ** args,
                            lisp_builtin_function_t    func);

int lisp_make_builtin_lambda_opt_args(struct lisp_vm_t       * vm,
                                      lisp_cell_t            * cell,
                                      lisp_size_t              args_size,
                                      lisp_cell_t            * args,
                                      lisp_size_t              opt_args_size,
                                      lisp_cell_t            * opt_args,
                                      lisp_size_t              named_args_size,
                                      lisp_cell_t            * named_args,
                                      lisp_cell_t            * named_args_values,
                                      int                      has_rest_args,
                                      lisp_builtin_function_t  func);

int lisp_eval_lambda(lisp_eval_env_t        * env,
                     lisp_lambda_t          * lambda,
                     lisp_size_t              nargs);

int lisp_lambda_compile(lisp_eval_env_t   * env,
                        lisp_cell_t       * cell,
                        const lisp_cell_t * expr);

int lisp_lambda_disassemble(struct lisp_vm_t * vm,
                            lisp_cell_t * cell,
                            lisp_lambda_t * lambda);


#endif
