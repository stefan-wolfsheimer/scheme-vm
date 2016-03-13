#ifndef __LISP_PARSER_H__
#define __LISP_PARSER_H__

#include "lisp_vm.h"

#define LISP_PARSER_START         0x00
#define LISP_PARSER_CAR           0x01
#define LISP_PARSER_CDR           0x02

#define LISP_PARSER_STOP_MASK     0x80
#define LISP_PARSER_UNEXPECTED    0x80
#define LISP_PARSER_END           0xff


/* @todo replace with cons */
typedef struct lisp_parser_stack_entry_t
{
  lisp_cell_t    * cell;
  unsigned int     pop_size;
} lisp_parser_stack_entry_t;


typedef struct lisp_parser_t 
{
  unsigned short  state;
  lisp_vector_t   stack;
} lisp_parser_t;

/* @todo replace with lisp_parser_t */
typedef struct lisp_parser_state_t
{
  short                       state;
  size_t                      column;
  size_t                      line;
  lisp_cell_t                 expression;
  lisp_parser_stack_entry_t * stack;
  size_t                      stack_size;
  size_t                      stack_top;
} lisp_parser_state_t;

/* @todo implement */
int lisp_make_parser(lisp_vm_t * vm,
                     lisp_cell_t * cell);

/* @todo replace with lisp_make_parser */
void lisp_parser_init(lisp_vm_t * vm, lisp_parser_state_t * state);
void lisp_parser_reset(lisp_vm_t * vm, lisp_parser_state_t * state);
void lisp_parser_free(lisp_vm_t * vm, lisp_parser_state_t * state);

size_t lisp_parse_next_expression(lisp_vm_t           * vm,
                                  lisp_parser_state_t * state,
                                  const lisp_char_t   * str,
                                  size_t                begin,
                                  size_t                end);

size_t lisp_parse_next_expression_cstr(lisp_vm_t           * vm,
                                    lisp_parser_state_t * state,
                                    const lisp_char_t   * str);

/* ' '
// ' symbol '
// '"str"'
// '   123  '
// '  (cons "a" 1234)    ' */

#endif
