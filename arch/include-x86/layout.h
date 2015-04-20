#ifndef _ASM_LAYOUT_H
#define _ASM_LAYOUT_H

#define USER_TLS_PAGES           0x1

#define USER_MSGQ_VREMAP_START   0x00001000
#define USER_MSGQ_VREMAP_SIZE    0x00100000
#define USER_ABI_VDSO            0xa0000000

/* Note: this idea, from a security perspective, is
   horrible. Linux ABI uses TLS to save a stack canary.

   Placing the canary here, just after the stack is pretty
   much like asking for a massive overflow.

   TODO: put an unmapper guard page somewhere between
   the stack and TLS */

#define KERNEL_BASE              0xd0000000
#define USER_TLS_START           (KERNEL_BASE - (USER_TLS_PAGES << 12))

#define USER_TLS_BASE            (USER_TLS_START + 2048) /* Thread info starts here */

#define KERNEL_VREMAP_AREA_START 0xe0000000
#define KERNEL_VREMAP_AREA_SIZE  0x00100000
#define KERNEL_MSGQ_VREMAP_START (KERNEL_VREMAP_AREA_START + KERNEL_VREMAP_AREA_SIZE)
#define KERNEL_MSGQ_VREMAP_SIZE  0x00100000
/* Per process LDT is loaded always at a fixed address */

#endif /* _ASM_LAYOUT_H */
