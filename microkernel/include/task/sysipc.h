#ifndef _TASK_SYSIPC_H
#define _TASK_SYSIPC_H

SYSPROTO (syscall_ipc_msg_request);
SYSPROTO (syscall_ipc_msg_map);
SYSPROTO (syscall_ipc_msg_unmap);
SYSPROTO (syscall_ipc_msg_send);
SYSPROTO (syscall_ipc_msg_recv);
SYSPROTO (syscall_ipc_msg_read_micro);
SYSPROTO (syscall_ipc_msg_write_micro);
SYSPROTO (syscall_ipc_msg_get_info);
SYSPROTO (syscall_ipc_msg_release);
SYSPROTO (syscall_ipc_msg_read_by_type);
SYSPROTO (syscall_ipc_msg_read);
SYSPROTO (syscall_ipc_msg_write);

#endif /* _TASK_SYSIPC_H */
