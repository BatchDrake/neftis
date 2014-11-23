/*
 *    Message queues
 *    Copyright (C) 2014  Gonzalo J. Carracedo
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef _TASK_MSG_H
#define _TASK_MSG_H

#include <types.h>

#include <mm/vm.h>
#include <misc/object.h>

#include <task/waitqueue.h>
#include <lock/lock.h>
#include <lock/event.h>

#define MSG_PENDING_COUNT 64

#define MSG_MICRO_SIZE 32

#define MSG_RECV_BLOCK    0
#define MSG_RECV_NONBLOCK 1

struct msg_body
{
  KERNEL_OBJECT;
  
  busword_t mb_size;
  busword_t mb_micro_size;
  uint8_t   mb_bytes[MSG_MICRO_SIZE];
  void     *mb_pages;
};

struct msg
{
  LINKED_LIST;

  int m_ro; /* Received messages are ready-only */
  busword_t m_virt;
  objref_t *m_msg;
  busword_t m_sender;
};

struct msg_info
{
  busword_t mi_virt_size;
  busword_t mi_micro_size;
  busword_t mi_sender_tid;
};

#define MSG_PTR_IS_VALID(ptr) (!((int) (ptr) & 1))
#define MSG_PTR_GET_IDX(ptr) ((int) (ptr) >> 1)
#define MSG_PTR_IDX(idx) ((struct msg *) (((idx) << 1) | 1))

/* As message queues are structures that can be modified
   from different tasks, their access should be atomic.

   Using a mutex here is a bad idea. What would happen if
   several tasks are waiting for the structure and then
   the recipient task exits? 
*/

struct msgq
{
  LINKED_LIST;

  event_t *mq_incoming_ready; /* Signaled when incoming queue has at least one packet */
  
  struct msg *mq_incoming;  /* Double-linked list */
  
  struct msg *mq_pending[MSG_PENDING_COUNT];

  int mq_last_free_msg; /* Last free outcoming message */

  struct vm_region *mq_region; /* This is a vremap */
};

struct msg_body *msg_body_new (busword_t);
void msg_body_destroy (struct msg_body *);

int __msg_request (struct task *, struct msgq *, busword_t);
int __msg_unmap (struct task *, struct msgq *, int);
busword_t __msg_map (struct task *, struct msgq *, int);
int __msg_send (struct msgq *, struct task *, struct msgq *, int);
int __msg_release (struct task *, struct msgq *, int);
int __msg_recv (struct task *, struct msgq *);
int __msg_read_micro (struct task *, struct msgq *, int, busword_t, unsigned int);
int __msg_get_info (struct task *, struct msgq *, int, busword_t);
int __msg_write_micro (struct task *, struct msgq *, int, busword_t, unsigned int);

/* These ones are the actual system calls and they can be called safely */

int sys_msg_request (busword_t size);
busword_t sys_msg_map (int id);
int sys_msg_unmap (int);
int sys_msg_send (int, int);
int sys_msg_recv (int);
int sys_msg_read_micro (int, void *, unsigned int);
int sys_msg_write_micro (int, const void *, unsigned int);
int sys_msg_get_info (int, struct msg_info *);
int sys_msg_release (int);


struct msg *msg_new (void);
void msg_destroy (struct msg *);
struct msg *msg_new_from_ref (struct task *, objref_t *);
struct msg *msg_new_from_msg (struct task *, struct msg_body *);
struct msgq *msgq_new (struct task *, struct vm_region *);
void msgq_destroy (struct msgq *msgq);
void init_msg_queues (void);

#endif /* _TASK_MSG_H */
