#include "lisp_parser.h"
#include "util/xmalloc.h"
#include <ctype.h>
#include <string.h>
#include <assert.h>

#define LISP_PARSER_STACK_PAGE 0x10

/* 

   START: 
     '(' : push($0, 1), state=CAR

   CAR:
     ')' : topv:= NIL, pop(), state=stack_empty?END:CDR
     '(' : c:=make_cons(), topv:= c, push(c.cdr,2), push(c.car, 1), state=CAR
   CDR:
     ')' : topv:= NIL, pop(), state=stack_empty?END:CDR
     '(' : c:=make_cons(), push(c.cdr, topc+1), push(c.car, 1), state=CAR

     ( ) 
   |          [ ], $0, START
     |        [ ($0,  1) ], $0, CAR
       |      [ ], $0=NIL, END

     ( ( ) ) 
   |          [ ], $0, START
     |        [ ($0, 1) ], $0, CAR
       |      [ (($1.$2), 1), ($2, 2), ($1, 1) ], $0=($1.$2), CAR
         |    [ ((NIL.$2), 1), ($2, 2) ], $0=(NIL.$2), CDR
           |  [ ], $0=(NIL.NIL), END

     ( ( ) ( ) ) 
   |              [ ], $0, START
     |            [ ($0, 1) ], $0, CAR
       |          [ (($1.$2), 1), ($2, 2), ($1, 1) ], $0=($1.$2), CAR
         |        [ ((NIL.$2), 1), ($2, 2) ], $0=(NIL.$2), CDR
           |      [ ((NIL.($3.$4),1),(($3.$4), 2), ($4, 2), ($3,1) ], $0=(NIL.($3.$4)), CAR
             |    [ ((NIL.(NIL.$4),1),((NIL.$4), 2), ($4, 3) ], $0=(NIL.(NIL.$4)), CDR
               |  [ ], $0=(NIL.(NIL.NIL)), END

     ( ( ) ( ) ( ) ) 
   |                 [ ], $0, START
     |               [ ($0, 1) ], $0, CAR
       |             [ (($1.$2), 1), ($2, 2), ($1, 1) ], $0=($1.$2), CAR
         |           [ ((NIL.$2), 1), ($2, 2) ], $0=(NIL.$2), CDR
           |         [ ((NIL.($3.$4),1),(($3.$4), 2), ($4, 2), ($3,1) ], $0=(NIL.($3.$4)), CAR
             |       [ ((NIL.(NIL.$4),1),((NIL.$4), 2), ($4, 3) ], $0=(NIL.(NIL.$4)), CDR
               |     [ ((NIL.(NIL.($5.$6)),1),((NIL.($5,$6)), 2), (($5.$6), 3), ($6,4), ($5,1)], $0=(NIL.(NIL.($5.$6))), CAR
                 |   [ ((NIL.(NIL.(NIL.$6),1),((NIL.(NIL,$6)), 2), ((NIL.$6), 3), ($6,4)], $0=(NIL.(NIL.(NIL.$6))), CDR
                   | [ ], $0=(NIL.(NIL.(NIL.NIL))), END
      
*/

static inline void _parser_push(lisp_vm_t           * vm,
                                lisp_parser_state_t * state,
                                lisp_cell_t         * cell,
                                unsigned int          pop_size);

static inline unsigned short _parser_pop(lisp_vm_t           * vm,
                                         lisp_parser_state_t * state);


int lisp_make_parser(lisp_vm_t * vm, lisp_cell_t * cell)
{
  lisp_parser_t * parser = MALLOC_OBJECT(sizeof(lisp_parser_t), 1);
  if(parser) 
  {
    cell->type_id = LISP_TID_PARSER;
    cell->data.ptr = parser;
    return 0;
  }
  else 
  {
    /* @todo allocation error code */
    return 1;
  }
}


void lisp_parser_init(lisp_vm_t * vm, lisp_parser_state_t * state)
{
  state->state      = LISP_PARSER_START;
  state->column     = 0;
  state->line       = 0;
  state->expression = lisp_nil;
  state->stack      = NULL;
  state->stack_size = 0;
  state->stack_top  = 0;
}

void lisp_parser_reset(lisp_vm_t * vm, lisp_parser_state_t * state)
{
  LISP_UNSET(vm, &state->expression);
  state->state      = LISP_PARSER_START;
  state->column     = 0;
  state->line       = 0;
  state->expression = lisp_nil;
  state->stack_top  = 0;
}

void lisp_parser_free(lisp_vm_t * vm, lisp_parser_state_t * state)
{
  if(state->stack) 
  {
    FREE(state->stack);
  }
}

static inline void _parser_push(lisp_vm_t           * vm,
                                lisp_parser_state_t * state,
                                lisp_cell_t         * cell,
                                unsigned int          pop_size)
{

  if(state->stack_top >= state->stack_size) 
  {
    state->stack_size += LISP_PARSER_STACK_PAGE;
    state->stack = REALLOC(state->stack, 
                           sizeof(lisp_parser_stack_entry_t) * 
                           state->stack_size);
  }
  assert(cell != NULL);
  state->stack[state->stack_top].cell = cell;
  state->stack[state->stack_top++].pop_size = pop_size;
}


static unsigned short _parser_pop(lisp_vm_t           * vm,
                                  lisp_parser_state_t * state)
{
  assert(state->stack_top);
  unsigned int n = state->stack[state->stack_top-1].pop_size;
  while(n) 
  {
    assert(state->stack_top);
    state->stack[--state->stack_top].cell = NULL;
    --n;
  }
  return 0;
}


static size_t _parse_state_start(lisp_vm_t           * vm,
                                 lisp_parser_state_t * state,
                                 const lisp_char_t   * str,
                                 size_t                pos,
                                 size_t                end)
{
  while(pos < end && isspace(str[pos])) 
  {
    pos++;
  }
  if(pos != end) 
  {
    if(str[pos] == '(')
    {
      /* Todo: check mem avail */
      state->state      = LISP_PARSER_CAR;
      state->expression = lisp_nil;
      _parser_push(vm,
                   state,
                   &state->expression,
                   1);
      pos++;
    }
    else if(str[pos] == ')')
    {
      state->state = LISP_PARSER_UNEXPECTED;
    }
  }
  return pos;
}

static size_t _parse_state_car(lisp_vm_t           * vm,
                               lisp_parser_state_t * state,
                               const lisp_char_t   * str,
                               size_t                pos,
                               size_t                end)
{
  while(pos < end && isspace(str[pos])) 
  {
    pos++;
  }
  if(pos != end) 
  {
    if(str[pos] == '(')
    {
      lisp_make_cons(vm, 
                     state->stack[state->stack_top-1].cell);
      lisp_cons_t * cons = state->stack[state->stack_top-1].cell->data.cons;
      _parser_push(vm, state, &cons->cdr, 2);
      _parser_push(vm, state, &cons->car, 1);
      state->state = LISP_PARSER_CAR;
      pos++;
    }
    else if(str[pos] == ')')
    {
      assert(state->stack_top);
      LISP_UNSET(vm, state->stack[state->stack_top-1].cell);
      _parser_pop(vm, state);
      state->state = state->stack_top ? LISP_PARSER_CDR : LISP_PARSER_END;
      pos++;
    }
  }
  return pos;
}

static size_t _parse_state_cdr(lisp_vm_t           * vm,
                               lisp_parser_state_t * state,
                               const lisp_char_t   * str,
                               size_t                pos,
                               size_t                end)
{
  while(pos < end && isspace(str[pos])) 
  {
    pos++;
  }
  if(pos != end) 
  {
    //')' : topv:= NIL, pop(), state=stack_empty?END:CDR
    //'(' : c:=make_cons(), push(c.cdr, topc+1), push(c.car, 1), state=CAR
    if(str[pos] == '(') 
    {
      assert(state->stack_top);
      lisp_make_cons(vm, 
                     state->stack[state->stack_top-1].cell);
      lisp_cons_t * cons = state->stack[state->stack_top-1].cell->data.cons;
      _parser_push(vm, 
                   state, 
                   &cons->cdr, 
                   state->stack[state->stack_top-1].pop_size + 1);
      _parser_push(vm, state, &cons->car, 1);
      state->state = LISP_PARSER_CAR;
      pos++;
    }
    else if(str[pos] == ')') 
    {
      assert(state->stack_top);
      LISP_UNSET(vm, state->stack[state->stack_top-1].cell);
      _parser_pop(vm, state);
      state->state = state->stack_top ? LISP_PARSER_CDR : LISP_PARSER_END;
      pos++;
    }
  }
  else 
  {
    /* Todo: error! */
  }
  return pos;
}

size_t lisp_parse_next_expression(lisp_vm_t           * vm,
                                  lisp_parser_state_t * state,
                                  const lisp_char_t   * str,
                                  size_t                begin,
                                  size_t                end)
{
  size_t pos = begin;
  while(pos < end && !( state->state & LISP_PARSER_STOP_MASK))
  {
    switch(state->state) 
    {
    case LISP_PARSER_START: 
      pos = _parse_state_start(vm, state, str, pos, end);
      break;

    case LISP_PARSER_CAR:
      pos = _parse_state_car(vm, state, str, pos, end);
      break;

    case LISP_PARSER_CDR:
      pos = _parse_state_cdr(vm, state, str, pos, end); 
      break;
    }
  }
  return pos;
}

size_t lisp_parse_next_expression_cstr(lisp_vm_t           * vm,
                                       lisp_parser_state_t * state,
                                       const lisp_char_t   * str)
{
  return lisp_parse_next_expression(vm, 
                                    state,
                                    str,
                                    0,
                                    strlen(str));
}

