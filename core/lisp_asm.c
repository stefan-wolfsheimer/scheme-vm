#include "lisp_vm.h"
#include "lisp_asm.h"
#include <stdio.h>

typedef int (*calle_function_t)(lisp_vm_t   * vm, 
                                lisp_instr_t   nargs);

static int lisp_calle_display(lisp_vm_t   * vm, 
                              lisp_instr_t   nargs)

{
  printf("display\n");
  return 0;
}

static int lisp_calle_readkey(lisp_vm_t   * vm, 
                              lisp_instr_t  fid)
{
  char ch[2];
  scanf("%c", &ch[0]);
  ch[1] = '\0';
  printf("readkey %c\n",ch);
  /* todo unset value */
  lisp_make_string(vm, &vm->value, ch);
  return 0;
}

static int lisp_calle_neq(lisp_vm_t   * vm, 
                              lisp_instr_t  fid)
{
  printf("neq\n");
  return 0;
}

static int lisp_calle_plus(lisp_vm_t   * vm, 
                           lisp_instr_t  fid)
{
  printf("plus\n");
  return 0;
}

static const calle_function_t functions[5] = 
{
  NULL,
  lisp_calle_display,
  lisp_calle_readkey,
  lisp_calle_neq,
  lisp_calle_plus
};


/* asprintf  is not portable, here is own implementation
 */
#include<stdarg.h>
/*char * lisp_sprintf(lisp_vm_t * vm, 
//                    lisp_cell_t * cell, 
//                    const lisp_char_t * fmt,
//                    ...);

//{
//  va_list ap;
//int s = 
//va_start(ap, fmt);
//}
*/

lisp_size_t lisp_disassemble_instr(lisp_vm_t    * vm,
                                   lisp_cell_t  * target,
                                   const lisp_instr_t * instr)
{
  switch(*instr) 
  {
  case 0x10: /* PUSHD */
    lisp_sprintf(vm, target, "PUSHD %u", instr[1]);
    return 2;
  case 0x11: /* PUSHS */
    lisp_sprintf(vm, target, "PUSHS %d", instr[1]);
    return 2;
  case 0x12: /* PUSHV */
    lisp_sprintf(vm, target, "PUSHV");
    return 1;
  case 0x13: /* MOVS */
    lisp_sprintf(vm, target, "MOVS %u %u", instr[1], instr[2]);
    return 3;
  case 0x20: /* CALLS */
    lisp_sprintf(vm, target, "CALLS %d",instr[1]);
    return 2;
  case 0x21: /* JP */
    lisp_sprintf(vm, target, "JP 0x%x",instr[1]);
    return 2;
  case 0x2f: /* CALLE */
    switch(instr[1])
      {
      case 0x01: 
        lisp_sprintf(vm, target, "CALL DISPLAY %d", instr[2]); 
        break;
      case 0x02: 
        lisp_sprintf(vm, target, "CALL READKEY %d", instr[2]); 
        break;
      case 0x03: 
        lisp_sprintf(vm, target, "CALL NE %d", instr[2]); 
        break;
      case 0x04: 
        lisp_sprintf(vm, target, "CALL PLUS %d", instr[2]); 
        break;
      }
    return 3;
  case 0x30: /* RETURN */
    lisp_sprintf(vm, target, "RETURN");
    return 1;
  case 0x31: /* RETURNIF */
    lisp_sprintf(vm, target, "RETURNIF");
    return 1;
  case LISP_END:
    lisp_sprintf(vm, target, "END");
    return 1;
  }
  return 0;
}

static void lisp_step_debug(lisp_vm_t     * vm, 
                            lisp_lambda_t * lambda, 
                            lisp_instr_t    pc)
{
  const lisp_instr_t * instr = (const lisp_instr_t*)&lambda[1];
  lisp_cell_t str;
  lisp_disassemble_instr(vm, &str, &instr[pc]);
  printf("<%p>:%d\t%s\n", lambda, pc, lisp_c_string(&str));
}

#define LISP_STEP_DEBUG(__VM__, __LAMBDA__, __PC__) \
  lisp_step_debug((__VM__), (__LAMBDA__), (__PC__))

int lisp_vm_run(lisp_vm_t     * vm, 
                lisp_lambda_t * lambda)
{
#if 0
  register lisp_instr_t * instr = (lisp_instr_t*)&lambda[1];
  lisp_cell_t * data = NULL;
  lisp_instr_t nargs = 0;

  vm->call_stack[vm->call_stack_top].lambda = NULL;
  vm->call_stack_top++;

  if(lambda->data_size) 
  {
    data = LISP_LAMBDA_DATA(lambda);
  }
  size_t pc = 0;
  while(lambda != NULL) 
  {
    LISP_STEP_DEBUG(vm, lambda, pc);
    switch(instr[pc]) 
    {
    case 0x10: /* PUSHD */
      LISP_SET(vm, 
               &vm->data_stack[vm->data_stack_top], 
               &data[instr[++pc]]);
      vm->data_stack_top++;
      ++pc;
      break;

    case 0x11: /* PUSHS */
      LISP_SET(vm, 
               &vm->data_stack[vm->data_stack_top], 
               &vm->data_stack[vm->data_stack_top-instr[++pc]]);
      vm->data_stack_top++;
      ++pc;
      break;

    case 0x12: /* PUSHV */
      lisp_create_integer(vm, 
                          &vm->data_stack[vm->data_stack_top], 
                          0);
      vm->data_stack_top++;
      ++pc;
      break;

    case 0x13: /* MOVS */
      ++pc;
      ++pc;
      ++pc;
      break;

    case 0x20: /* CALLS */
      ++pc;
      nargs = instr[pc];
      ++pc;
      vm->call_stack[vm->call_stack_top] = (lisp_call_t){ pc, 
                                                          nargs,
                                                          lambda};
      vm->call_stack_top++;
      lambda = vm->data_stack[vm->data_stack_top - nargs].data.lambda;
      instr = (lisp_instr_t*)&lambda[1];
      pc = 0;
      break;

    case 0x21: /* JP */
      ++pc;
      ++pc;
      break;

    case 0x2f: /* CALLE */
      functions[instr[++pc]](vm, instr[++pc]);
      nargs = instr[pc];
      while(nargs) 
      {
        vm->data_stack_top--;
        nargs--;
      }
      ++pc;
      break;

    case 0x30: /* RETURN */
      vm->call_stack_top--;      
      lambda = vm->call_stack[vm->call_stack_top].lambda;
      nargs  = vm->call_stack[vm->call_stack_top].nargs;
      pc = vm->call_stack[vm->call_stack_top].pc_next;
      while(nargs) 
      {
        vm->data_stack_top--;
        nargs--;
      }
      instr = (lisp_instr_t*)&lambda[1];
      break;

    case 0x31: /* RETURNIF */
      vm->call_stack_top--;      
      lambda = vm->call_stack[vm->call_stack_top].lambda;
      nargs  = vm->call_stack[vm->call_stack_top].nargs;
      pc = vm->call_stack[vm->call_stack_top].pc_next;
      while(nargs) 
      {
        vm->data_stack_top--;
        nargs--;
      }
      instr = (lisp_instr_t*)&lambda[1];
    }
  }
#endif
  return 0;
}
