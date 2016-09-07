#ifndef __LISP_VM_H__
#define __LISP_VM_H__

/**
   @file  lisp_vm.h
   @brief core virtual machine functions 

   @todo  Replace create function with "make" functions.
   
           Convention
           int make_XXX(lisp_vm_t * vm, lisp_cell_t * target, ...)
           int make_XXX_version2(lisp_vm_t * vm, lisp_cell_t * target, ...)
           int make_XXX_variant3(lisp_vm_t * vm, lisp_cell_t * target, ...)
           target points to an non-initialized object or 
           an atom.
           returns: 0 on success, error code otherwise
 */
#include <stdlib.h>
#include <stdarg.h>
#include "lisp_type.h"
#include "util/hash_table.h"

typedef struct lisp_vm_t 
{
  /* @todo move to continiation */
  lisp_cell_t   * data_stack;
  lisp_size_t     data_stack_top;
  lisp_size_t     data_stack_size;
  //lisp_call_t   * call_stack;
  lisp_size_t     call_stack_top;
  lisp_size_t     call_stack_size;
  /* @todo */
  lisp_size_t     program_counter;

  lisp_type_t   * types;
  lisp_size_t     types_size;

  hash_table_t    symbols;

  /* cons data and garbage collector */
  lisp_cons_t               ** cons_table;
  lisp_size_t                  cons_table_size;

  lisp_size_t                  black_cons_top;
  lisp_size_t                  grey_cons_begin;
  lisp_size_t                  grey_cons_top;
  lisp_size_t                  white_cons_top;


  lisp_root_cons_t           * root_cons_table;
  lisp_size_t                  root_cons_table_size;
  lisp_size_t                  root_cons_top;


  lisp_cons_t               ** cons_pages;
  lisp_size_t                  n_cons_pages;
  lisp_size_t                  cons_page_size;

} lisp_vm_t;

typedef struct lisp_vm_param_t
{
  size_t data_stack_size;
  size_t call_stack_size;
} lisp_vm_param_t;

extern lisp_vm_param_t lisp_vm_default_param;

lisp_vm_t * lisp_create_vm( lisp_vm_param_t * param);
void lisp_free_vm(   lisp_vm_t * vm);


/*****************************************************************************
 * 
 * Types
 * 
 *****************************************************************************/

/** Register a user defined type 
 *  @param vm the machine
 *  @param name name of the type
 *  @param destructor (optional) 
 *         custom destructor that is called when the reference count is 0.
 *         if NULL, the memory is released.
 *  @param new_type the type id of the registered type
 *  @return LISP_OK or LISP_TYPE_ERROR
 */
int lisp_register_object_type(lisp_vm_t        * vm,
			      lisp_char_t      * name,
			      lisp_destructor_t  destructor,
                              lisp_printer_t      printer,
			      lisp_type_id_t   * new_type);

/** Register a user define cons type.
 *  @param vm the machine
 *  @param name name of the type
 *  @param new_type the type id of the registered type
 *  @return LISP_OK or LISP_TYPE_ERROR
 */
int lisp_register_cons_type(lisp_vm_t          * vm,
			    lisp_char_t        * name,
			    lisp_type_id_t     * new_type);

/*****************************************************************************
 * 
 * copy objects
 * 
 *****************************************************************************/
/* @todo implement copy as root object and test if conses 
         have been rooted
   target must be atom or uninitalized
*/
int lisp_copy_object( lisp_vm_t   * vm,
		      lisp_cell_t * target,
		      const lisp_cell_t * source);

int lisp_copy_object_as_root( lisp_vm_t   * vm,
			      lisp_cell_t * target,
			      const lisp_cell_t * source);

int lisp_copy_n_objects( lisp_vm_t   * vm,
			 lisp_cell_t * target,
			 const lisp_cell_t * source,
			 lisp_size_t   n);

int lisp_copy_n_objects_as_root( lisp_vm_t   * vm,
				 lisp_cell_t * target,
				 const lisp_cell_t * source,
				 lisp_size_t   n);

int lisp_unset_object(lisp_vm_t * vm, 
		      lisp_cell_t * target);

int lisp_unset_object_root(lisp_vm_t * vm,
			   lisp_cell_t * target);

/*****************************************************************************
 * 
 * compare objects
 * 
 *****************************************************************************/
/**
 * @return non zero if a and b have the same type and value 
 */
int lisp_eq_object(const lisp_cell_t * a,
		   const lisp_cell_t * b);

/*****************************************************************************
 * 
 * string
 * 
 *****************************************************************************/
/* @todo make module */
/* @todo unicode */
int lisp_make_string(lisp_vm_t         * vm,
                     lisp_cell_t       * cell,
                     const lisp_char_t * cstr);

int lisp_make_substring(lisp_vm_t           * vm,
			lisp_cell_t         * cell,
			const lisp_string_t * str,
			lisp_size_t           a,
			lisp_size_t           b);

int lisp_sprintf(lisp_vm_t         * vm,
                 lisp_cell_t       * cell,
                 const lisp_char_t * cstr,
                 ...);

int lisp_va_sprintf(lisp_vm_t   * vm,
                    lisp_cell_t * cell,
                    const char  * fmt,
                    va_list       val);

const char * lisp_c_string(const lisp_string_t * cell);

lisp_size_t lisp_string_length(const lisp_string_t * str);

int lisp_string_cmp(const lisp_string_t * a, const lisp_string_t * b);

int lisp_string_cmp_c_string(const lisp_string_t * a, const char * cstr);


/*****************************************************************
 *
 * cons objects
 *
 *****************************************************************/
/* @todo ensure invariant: black/root conses don't have white children 
 * @param cell unitialized cell or atom 
 */
int lisp_make_cons(lisp_vm_t   * vm,
                   lisp_cell_t * cell);

int lisp_make_cons_car_cdr(lisp_vm_t * vm,
                           lisp_cell_t * cell,
                           const lisp_cell_t * car, 
                           const lisp_cell_t * cdr);

int lisp_make_cons_typed(lisp_vm_t     * vm,
                         lisp_cell_t   * cell,
                         lisp_type_id_t  type_id);

int lisp_make_cons_typed_car_cdr(lisp_vm_t     * vm,
                                 lisp_cell_t   * cell,
                                 lisp_type_id_t  type_id,
                                 const lisp_cell_t * car, 
                                 const lisp_cell_t * cdr);

int lisp_make_cons_root(lisp_vm_t   * vm,
                        lisp_cell_t * cell);

int lisp_make_cons_root_car_cdr(lisp_vm_t * vm,
                                lisp_cell_t * cell,
                                const lisp_cell_t * car, 
                                const lisp_cell_t * cdr);

int lisp_make_cons_root_typed(lisp_vm_t     * vm,
                              lisp_cell_t   * cell,
                              lisp_type_id_t  type_id);

int lisp_make_cons_root_typed_car_cdr(lisp_vm_t     * vm,
                                      lisp_cell_t   * cell,
                                      lisp_type_id_t  type_id,
                                      const lisp_cell_t * car, 
                                      const lisp_cell_t * cdr);
                                      

/** 
 * Returns the number of reference of the cons 
 * object in the root set.
 * If cell is not a cons, return 0 by default.
 *
 * @param  vm virtual machine context 
 * @param  cell to be tested
 * @return reference count
 *
 */
lisp_ref_count_t lisp_root_refcount(lisp_vm_t         * vm,
				    const lisp_cell_t * cell);

int lisp_cons_root(lisp_vm_t * vm, lisp_cons_t * cons);
int lisp_cons_unroot(lisp_vm_t * vm, lisp_cons_t * cons);


/* set car and/or cdr 
 */
int lisp_cons_set_car_cdr(lisp_vm_t * vm, 
			  lisp_cons_t * cons, 
			  const lisp_cell_t * car,
			  const lisp_cell_t * cdr);

int lisp_make_list_root(lisp_vm_t         * vm,
			lisp_cell_t       * cell,
			const lisp_cell_t * elems,
			lisp_size_t         n);

int lisp_make_list_root_typed(lisp_vm_t         * vm,
                              lisp_cell_t       * cell,
                              lisp_type_id_t      type_id,
                              const lisp_cell_t * elems,
                              lisp_size_t         n);

int lisp_make_list(lisp_vm_t         * vm,
		   lisp_cell_t       * cell,
		   const lisp_cell_t * elems,
		   lisp_size_t         n);

/*****************************************************************
 *
 * integer
 *
 *****************************************************************/
void lisp_make_integer(lisp_cell_t * cell, lisp_integer_t value);


/*****************************************************************
 *
 * expression to string
 *
 *****************************************************************/
lisp_size_t lisp_object_to_c_str(lisp_vm_t * vm, char * buff, size_t maxn, const lisp_cell_t * cell);


/*****************************************************************
 *
 * create basic objects
 *
 *****************************************************************/
#ifdef DEBUG
void * lisp_malloc_object( const char      * file,
                           int               line, 
                           size_t            size,
                           lisp_ref_count_t  rcount);

void lisp_free_object( const char    * file,
                       int             line,
                       void          * ptr);

#define MALLOC_OBJECT(SIZE, RCOUNT) lisp_malloc_object(__FILE__,        \
                                                       __LINE__,        \
                                                       (SIZE),          \
                                                       (RCOUNT))

#define FREE_OBJECT(PTR) lisp_free_object(__FILE__,__LINE__,(PTR))

#else

void * lisp_malloc_object( size_t           size,
                           lisp_ref_count_t rcount );

void lisp_free_object( void * ptr);

/** Create a managed object of size SIZE with reference count RCOUNT
 */
#define MALLOC_OBJECT(SIZE, RCOUNT) lisp_malloc_object((SIZE),(RCOUNT))

/** 
 * Free a managed object.
 * Should be handled by the garbage collector
 */
#define FREE_OBJECT(PTR) lisp_free_object((PTR))

#endif

#endif
