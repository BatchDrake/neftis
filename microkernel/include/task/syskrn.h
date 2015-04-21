#ifndef _TASK_SYSKRN_H
#define _TASK_SYSKRN_H

SYSPROTO (syscall_krn_exit);
SYSPROTO (syscall_krn_debug_int);
SYSPROTO (syscall_krn_debug_string);
SYSPROTO (syscall_krn_debug_pointer);
SYSPROTO (syscall_krn_debug_buf);
SYSPROTO (syscall_krn_brk);

#endif /* _TASK_SYSKRN_H */
