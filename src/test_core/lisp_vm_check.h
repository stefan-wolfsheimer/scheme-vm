#ifndef __TEST_CORE_CHECK_H__
#define __TEST_CORE_CHECK_H__
#include "core/lisp_vm.h"
#include "util/unit_test.h"
#include "context.h"

void lisp_test_object_destructor(lisp_vm_t * vm, void * ptr);

int lisp_make_test_object(lisp_cell_t   * target, 
			  int           * flags,
			  lisp_type_id_t  id);



void set_up_conses(unit_test_t * tst, 
		   lisp_vm_t   * vm,
		   lisp_size_t   n_root_conses,
		   lisp_size_t   n_black_conses,
		   lisp_size_t   n_gap_conses,
		   lisp_size_t   n_grey_conses,
		   lisp_size_t   n_white_conses);


int lisp_vm_check(unit_test_t * tst, lisp_vm_t * vm);

int lisp_is_root_cons(lisp_vm_t * vm, const lisp_cell_t * cell);
int lisp_is_black_cons(lisp_vm_t * vm, const lisp_cell_t * cell);
int lisp_is_white_cons(lisp_vm_t * vm, const lisp_cell_t * cell);
int lisp_is_grey_cons(lisp_vm_t * vm,  const lisp_cell_t * cell);

lisp_size_t lisp_n_root_cons(lisp_vm_t * vm);
lisp_size_t lisp_n_black_cons(lisp_vm_t * vm);
lisp_size_t lisp_n_gap_cons(lisp_vm_t * vm);
lisp_size_t lisp_n_grey_cons(lisp_vm_t * vm);
lisp_size_t lisp_n_white_cons(lisp_vm_t * vm);

lisp_cell_t lisp_get_root_cons( lisp_vm_t * vm, lisp_size_t i);
lisp_cell_t lisp_get_black_cons( lisp_vm_t * vm, lisp_size_t i);
lisp_cell_t lisp_get_grey_cons( lisp_vm_t * vm, lisp_size_t i);
lisp_cell_t lisp_get_white_cons( lisp_vm_t * vm, lisp_size_t i);

int lisp_mock_return_alloc_error(void * user_data);

#endif
