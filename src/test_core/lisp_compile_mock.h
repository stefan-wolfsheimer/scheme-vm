#ifndef __LISP_COMPILE_MOCK_H__
#define __LISP_COMPILE_MOCK_H__

#include "core/lisp_type.h"

struct lisp_vm_t;
struct lisp_eval_env_t;
struct lisp_lambda_t;

/* mock for lambda calls */
typedef struct lisp_lambda_mock_t
{
  struct lisp_vm_t   * vm;
  size_t               n_values;
  lisp_cell_t        * values;
  size_t               n_args;
  lisp_cell_t        * args;
} lisp_lambda_mock_t;

void lisp_init_lambda_mock(lisp_lambda_mock_t * mock, 
			   struct lisp_vm_t   * vm,
			   size_t               n_args);

void lisp_free_lambda_mock(lisp_lambda_mock_t * mock);

int lisp_lambda_mock_function(struct lisp_eval_env_t     * env,
                              const lisp_lambda_t        * lambda,
                              lisp_size_t                  nargs);

int lisp_compile_phase1_mock(struct lisp_vm_t * vm, 
                             lisp_size_t      * instr_size,
                             const lisp_cell_t * expr);

int lisp_compile_phase1_mock_failure(struct lisp_vm_t * vm, 
                                     lisp_size_t      * instr_size,
                                     const lisp_cell_t * expr);

int lisp_compile_phase2_mock(struct lisp_vm_t  * vm,
                             lisp_cell_t       * cell,
                             lisp_instr_t      * instr,
                             const lisp_cell_t * expr);

int lisp_compile_phase2_mock_failure(struct lisp_vm_t  * vm,
                                     lisp_cell_t       * cell,
                                     lisp_instr_t      * instr,
                                     const lisp_cell_t * expr);

#endif
