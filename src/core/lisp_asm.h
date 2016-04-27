#ifndef __LISP_ASM_H__
#define __LISP_ASM_H__

#include "lisp_type.h"

#define LISP_END                       0xffffff

#define LISP_PUSHD(__ARG1__)           0x10, (__ARG1__)
#define LISP_PUSHS(__ARG1__)           0x11, (__ARG1__)
#define LISP_PUSHV                     0x12
#define LISP_MOVS(__ARG1__, __ARG2__)  0x13, (__ARG1__), (__ARG2__)

#define LISP_CALLS(__ARG1__)           0x20, (__ARG1__)
#define LISP_JP(__ARG1__)              0x21, (__ARG1__)

#define LISP_CALLE(__ARG1__, __ARG2__) 0x2f, (__ARG1__), (__ARG2__)
#define LISP_CALLE_DISPLAY(__ARG1__)   0x2f, 0x01, (__ARG1__)
#define LISP_CALLE_READKEY(__ARG1__)   0x2f, 0x02, (__ARG1__)
#define LISP_CALLE_NE(__ARG1__)        0x2f, 0x03, (__ARG1__)
#define LISP_CALLE_PLUS(__ARG1__)      0x2f, 0x04, (__ARG1__)

#define LISP_RETURN                    0x30
#define LISP_RETURNIF                  0x31

/*****************************************************************************/

/* \param target uninitialized cell
 */
lisp_size_t lisp_disassemble_instr(lisp_vm_t    * vm,
                                   lisp_cell_t  * target,
                                   const lisp_instr_t * instr);

#endif
