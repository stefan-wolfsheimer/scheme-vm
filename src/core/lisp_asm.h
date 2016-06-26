#ifndef __LISP_ASM_H__
#define __LISP_ASM_H__
#include "lisp_type.h"


#define LISP_ASM_LDVD      0x01
#define LISP_SIZ_LDVD      sizeof(lisp_instr_t) + sizeof(lisp_cell_t)

#define LISP_ASM_LDVR      0x02
#define LISP_SIZ_LDVR      sizeof(lisp_instr_t) + sizeof(lisp_cell_t)

#define LISP_ASM_PUSHD     0x03
#define LISP_SIZ_PUSHD     sizeof(lisp_instr_t) + sizeof(lisp_cell_t)

#define LISP_ASM_RET       0x10
#define LISP_SIZ_RET       sizeof(lisp_instr_t)

#define LISP_ASM_JP        0x11
#define LISP_SIZ_JP        sizeof(lisp_instr_t) + sizeof(lisp_size_t) + \
                           sizeof(lisp_lambda_t*)

#define LISP_ASM_HALT      0x12
#define LISP_SIZ_HALT      sizeof(lisp_instr_t)

#define LISP_ASM_BUILTIN   0x20
#define LISP_SIZ_BUILTIN   sizeof(lisp_instr_t) + \
                           sizeof(lisp_builtin_function_t)

#define LISP_INSTR_ARG(__INSTR__, __TYPE__)     \
  ((__TYPE__*)((__INSTR__) + 1))

#define LISP_INSTR_ARG_2(__INSTR__, __TYPE__, __TYPE2__)        \
  ((__TYPE2__*)(LISP_INSTR_ARG(__INSTR__,__TYPE__) + 1))

#define LISP_SET_INSTR(__TYPE_ID__, __INSTR__, __TYPE__, __VALUE__)  \
  {                                                                  \
    *(__INSTR__) = (__TYPE_ID__);                                    \
    *LISP_INSTR_ARG(__INSTR__, __TYPE__) = (__VALUE__);              \
  }

#define LISP_SET_INSTR_2(__TYPE_ID__, __INSTR__, __TYPE__, __VALUE__,   \
                         __TYPE2__, __VALUE2__)                         \
  {                                                                     \
    *(__INSTR__) = (__TYPE_ID__);                                       \
    *LISP_INSTR_ARG(__INSTR__, __TYPE__) = (__VALUE__);                 \
    *LISP_INSTR_ARG_2(__INSTR__,__TYPE__,__TYPE2__) = (__VALUE2__);     \
  }

#endif
