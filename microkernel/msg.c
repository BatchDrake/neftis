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
 
#include <types.h>

#include <mm/regions.h>
#include <mm/coloring.h>
#include <mm/vm.h>
#include <mm/vremap.h>

#include <misc/list.h>
#include <misc/object.h>
#include <misc/errno.h>

#include <task/msg.h>

#include <arch.h>
#include <kctx.h>

struct msg_body *
msg_body_new (busword_t size)
{
  struct msg_body *new;
  busword_t pages;
  
  if ((new = salloc_irq (sizeof (struct msg_body))) == NULL)
    return NULL;

  memset (new, 0, sizeof (struct msg));

  new->mb_size = size;
  
  if (size > 0)
  {
    pages = __UNITS (size, PAGE_SIZE);

    if ((new->mb_pages = page_alloc (pages)) == NULL)
    {
      sfree (new);

      return NULL;
    }
  }

  return new;
}

void
msg_body_destroy (struct msg_body *body)
{
  printk ("Removing body...\n");
  
  if (body->mb_size > 0)
    page_free (body->mb_pages, __UNITS (body->mb_size, PAGE_SIZE));

  sfree (body);
}

static int
__msg_remap_flags (struct msgq *msgq, struct msg *msg, DWORD flags)
{
  busword_t addr;
  struct msg_body *body = REFCAST (struct msg_body, msg->m_msg);
  
  if (body->mb_size == 0)
    return 0;

  if ((addr = vm_region_vremap_ensure (msgq->mq_region, __UNITS (body->mb_size, PAGE_SIZE))) == -1)
    return -1;

  if (vm_region_map_pages (msgq->mq_region, addr, (busword_t) body->mb_pages, flags, __UNITS (body->mb_size, PAGE_SIZE)) == -1)
  {
    (void) vm_region_vremap_release (msgq->mq_region, addr, __UNITS (body->mb_size, PAGE_SIZE));

    return -1;
  }

  msg->m_virt = addr;

  return 0;
}


static int
__msgq_alloc_id (struct msgq *msgq)
{
  int id;

  if ((id = msgq->mq_last_free_msg) == -1)
    return -1;

  ASSERT (!MSG_PTR_IS_VALID (msgq->mq_pending[id]));
  
  msgq->mq_last_free_msg = MSG_PTR_GET_IDX (msgq->mq_pending[id]);

  msgq->mq_pending[id] = NULL;

  return id;
}

static void
__msgq_release_id (struct msgq *msgq, int id)
{
  ASSERT (MSG_PTR_IS_VALID (msgq->mq_pending[id]));
    
  msgq->mq_pending[id] = MSG_PTR_IDX (msgq->mq_last_free_msg);

  msgq->mq_last_free_msg = id;
}

static struct msg *
__msg_get_ptr (struct msgq *msgq, int id)
{
  if (id < 0 || id >= MSG_PENDING_COUNT)
    return NULL;

  return MSG_PTR_IS_VALID (msgq->mq_pending[id]) ? msgq->mq_pending[id] : NULL;
}

/* Summary of message syscalls:

   msg_request: request a message (with virtual space for X bytes)
   
   msg_send:    send message (with ID to task ID)
   msg_recv:    receive message (returns ID)

   msg_read:    read micromessage data
   msg_write:   write micromessage data

   msg_map:     map virtual data (when possible)
   
   msg_getinfo: get message info (virtual space, size, sender, etc)
   
   msg_release: release a message

   Received messages can also be forwarded to new processes

 */

int
__msg_request (struct task *task, struct msgq *msgq, busword_t size)
{
  int id;

  struct msg_body *body = NULL;
  struct msg *msg = NULL;
  
  /* Create message and build msg, and put it to pending */
  /* Return pending msg id */

  if ((id = __msgq_alloc_id (msgq)) == -1)
    return -EMFILE;

  if ((body = msg_body_new (size)) == NULL)
    goto fail;

  if ((msg = msg_new_from_msg (task, body)) == NULL)
    goto fail;

  msgq->mq_pending[id] = msg;

  return id;
  
fail:

  if (msg != NULL)
    msg_destroy (msg);
  else if (body != NULL)
    msg_body_destroy (body);

  __msgq_release_id (msgq, id);
  
  return -ENOMEM;
}

static int
__msg_unmap_m (struct task *task, struct msgq *msgq, struct msg *msg)
{
  if (REFCAST (struct msg_body, msg->m_msg)->mb_size > 0 && msg->m_virt != 0)
    return -EINVAL;
  
  vm_region_vremap_release (msgq->mq_region, msg->m_virt, __UNITS (REFCAST (struct msg_body, msg->m_msg)->mb_size, PAGE_SIZE));

  msg->m_virt = 0;
  
  return 0;
}

int
__msg_unmap (struct task *task, struct msgq *msgq, int id)
{
  struct msg *msg;

  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  return __msg_unmap_m (task, msgq, msg);
}

busword_t
__msg_map (struct task *task, struct msgq *msgq, int id)
{
  struct msg *msg;
  DWORD flags;
  
  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  flags = VM_PAGE_READABLE;

  if (!msg->m_ro)
    flags |= VM_PAGE_WRITABLE;
  
  if (__msg_remap_flags (msgq, msg, flags) == -1)
    return -ENOMEM;

  if (vm_update_region (REFCAST (struct vm_space, task->ts_vm_space), msgq->mq_region) == -1)
  {
    (void) __msg_unmap_m (task, msgq, msg);
    
    return -ENOMEM;
  }

  return msg->m_virt;
}

int
__msg_send (struct msgq *msgq, struct task *recipient_task, struct msgq *recipient_msgq, int id)
{
  /* Build msg based on reference */
  /* Put in recipient's incoming */
  /* Signal event */

  struct msg *msg;
  struct msg *new_msg;
  
  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  if ((new_msg = msg_new_from_ref (recipient_task, msg->m_msg)) == NULL)
    return -ENOMEM;

  new_msg->m_ro = 1;

  circular_list_insert_tail ((void **) &recipient_msgq->mq_incoming, new_msg);

  event_signal (recipient_msgq->mq_incoming_ready);

  return 0;
}

int
__msg_release (struct task *task, struct msgq *msgq, int id)
{
  /* Release message from pending list */

  struct msg *msg;

  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  if (msg->m_virt != 0)
    __msg_unmap_m (task, msgq, msg);

  __msgq_release_id (msgq, id);
    
  msg_destroy (msg);
  
  return 0;
}

int
__msg_recv (struct task *task, struct msgq *msgq)
{
  struct msg *msg;
  int id;
  
  if (circular_list_is_empty ((void *) &msgq->mq_incoming))
    return -EAGAIN;

  msg = circular_list_get_head ((void *) &msgq->mq_incoming);

  if ((id = __msgq_alloc_id (msgq)) == -1)
    return -EMFILE;

  msgq->mq_pending[id] = msg;

  circular_list_remove_element ((void *) &msgq->mq_incoming, msg);

  return id;
}

int
__msg_read_micro (struct task *task, struct msgq *msgq, int id, busword_t virt, unsigned int size)
{
  struct msg *msg;
  struct msg_body *body;
  
  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  body = REFCAST (struct msg_body, msg->m_msg);
  
  if (body->mb_micro_size < size)
    size = body->mb_micro_size;

  if (virt != 0)
    if (copy2virt (REFCAST (struct vm_space, task->ts_vm_space), virt, body->mb_bytes, size) < size)
      return -EFAULT;
  
  return size;
}

int
__msg_get_info (struct task *task, struct msgq *msgq, int id, busword_t virt)
{
  struct msg *msg;
  struct msg_body *body;
  struct msg_info info;

  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  body = REFCAST (struct msg_body, msg->m_msg);
  
  info.mi_virt_size  = body->mb_size;
  info.mi_micro_size = body->mb_micro_size;
  info.mi_sender_tid = msg->m_sender;

  if (virt != 0)
    if (copy2virt (REFCAST (struct vm_space, task->ts_vm_space), virt, body->mb_bytes, sizeof (struct msg_info)) < sizeof (struct msg_info))
      return -EFAULT;

  return 0;
}

int
__msg_write_micro (struct task *task, struct msgq *msgq, int id, busword_t virt, unsigned int size)
{
  struct msg *msg;
  struct msg_body *body;
  
  if ((msg = __msg_get_ptr (msgq, id)) == NULL)
    return -EBADF;

  body = REFCAST (struct msg_body, msg->m_msg);
  
  if (size > MSG_MICRO_SIZE)
    size = MSG_MICRO_SIZE;
  
  body->mb_micro_size = size;

  if (virt != 0)
    if (copy2phys (REFCAST (struct vm_space, task->ts_vm_space), body->mb_bytes, virt, size) < size)
      return -EFAULT;

  return size;
}

static void
msg_body_dtor (void *ptr)
{
  printk ("Calling dtor on %p\n", ptr);
  
  msg_body_destroy ((struct msg_body *) ptr);
}

struct kernel_class msg_body_class =
{
  .name = "msg-body",
  .dtor = msg_body_dtor
};

struct msg *
msg_new (void)
{
  struct msg *new;

  if ((new = salloc_irq (sizeof (struct msg))) == NULL)
    return NULL;

  memset (new, 0, sizeof (struct msg));

  return new;
}

void
msg_destroy (struct msg *msg)
{
  if (msg->m_msg != NULL)
    kernel_object_ref_close (msg->m_msg);
  
  sfree (msg);
}

struct msg *
msg_new_from_ref (struct task *task, objref_t *ref)
{
  struct msg *new;
  
  if ((new = msg_new ()) == NULL)
    return NULL;
       
  if ((new->m_msg = kernel_object_open_task (REFOBJ (ref), task)) == NULL)
  {
    msg_destroy (new);

    return NULL;
  }

  return new;
}

struct msg *
msg_new_from_msg (struct task *task, struct msg_body *body)
{
  struct msg *new;

  if ((new = msg_new ()) == NULL)
    return NULL;
       
  if ((new->m_msg = kernel_object_instance_task (&msg_body_class, body, task)) == NULL)
  {
    msg_destroy (new);

    return NULL;
  }

  return new;
}

struct msgq *
msgq_new (struct task *task, struct vm_region *region)
{
  struct msgq *new;
  int i;

  ASSERT (vm_region_is_vremap (region));
  
  if ((new = salloc_irq (sizeof (struct msgq))) == NULL)
    return NULL;

  memset (new, 0, sizeof (struct msgq));

  new->mq_region = region;
  
  if ((new->mq_incoming_ready = event_new ()) == NULL)
    goto fail;

  for (i = 0; i < MSG_PENDING_COUNT - 1; ++i)
    new->mq_pending[i] = MSG_PTR_IDX (i + 1);

  new->mq_pending[i] = MSG_PTR_IDX (-1);
  
  return new;
  
fail:

  if (new->mq_incoming_ready != NULL)
    event_destroy (new->mq_incoming_ready);
  
  sfree_irq (new);
  
  return NULL;
}

void
msgq_destroy (struct msgq *msgq)
{
  struct msg *this, *next, *first;
  int i;
  
  event_destroy (msgq->mq_incoming_ready);

  first = this = msgq->mq_incoming;

  while (this != NULL)
  {
    next = LIST_NEXT (this);
    
    msg_destroy (next);
    
    if ((this = next) == first)
      break;
  }

  for (i = 0; i < MSG_PENDING_COUNT; ++i)
    if (MSG_PTR_IS_VALID (msgq->mq_pending[i]))
      msg_destroy (msgq->mq_pending[i]);
  
  /* Region is not freed as it must be allocated prior to any message queue allocation */
  
  sfree_irq (msgq);
}

int
sys_msg_request (busword_t size)
{
  int ret;
  struct task *task;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();
  
  if (task->ts_msgq != NULL)
    ret = __msg_request (task, task->ts_msgq, size);
  else
    ret = -ENOSYS;

  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

int
sys_msg_unmap (int id)
{
  int ret;
  struct task *task;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
    ret = __msg_unmap (task, task->ts_msgq, id);
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

busword_t
sys_msg_map (int id)
{
  busword_t ret;
  struct task *task;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
    ret = __msg_map (task, task->ts_msgq, id);
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

int
sys_msg_send (int id, int tid)
{
  int ret;
  struct task *task;
  struct task *recipient;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
  {
    if ((recipient = get_task (tid)) == NULL || recipient->ts_msgq == NULL)
      ret = -ESRCH;
    else
      ret = __msg_send (task->ts_msgq, recipient, recipient->ts_msgq, id);
  }
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

int
sys_msg_release (int id)
{
  int ret;
  struct task *task;

  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
    ret = __msg_release (task, task->ts_msgq, id);
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

int
sys_msg_recv (int nonblock)
{
  int ret;
  struct task *task;

  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
  {
    do
    {
      ret = __msg_recv (task, task->ts_msgq);

      if (ret == -EAGAIN && !nonblock)
      {
	TASK_ATOMIC_LEAVE (msg);
	
	event_wait (task->ts_msgq->mq_incoming_ready);

	TASK_ATOMIC_ENTER (msg);
      }
    }
    while (ret == -EAGAIN && !nonblock);
  }
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg); /* Wait here */

  return ret;
}

int
sys_msg_read_micro (int id, void *virt, unsigned int size)
{
  int ret;
  struct task *task;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
    ret = __msg_read_micro (task, task->ts_msgq, id, (busword_t) virt, size);
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

int
sys_msg_write_micro (int id, const void *virt, unsigned int size)
{
  int ret;
  struct task *task;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
    ret = __msg_write_micro (task, task->ts_msgq, id, (busword_t) virt, size);
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

int
sys_msg_get_info (int id, struct msg_info *virt)
{
  int ret;
  struct task *task;
  
  DECLARE_CRITICAL_SECTION (msg);

  TASK_ATOMIC_ENTER (msg);

  task = get_current_task ();

  if (task->ts_msgq != NULL)
    ret = __msg_get_info (task, task->ts_msgq, id, (busword_t) virt);
  else
    ret = -ENOSYS;
  
  TASK_ATOMIC_LEAVE (msg);

  return ret;
}

void
init_msg_queues (void)
{
  kernel_class_register (&msg_body_class);
}

DEBUG_FUNC (msg_body_new);
DEBUG_FUNC (msg_body_destroy);
DEBUG_FUNC (__msg_remap_flags);
DEBUG_FUNC (__msgq_alloc_id);
DEBUG_FUNC (__msgq_release_id);
DEBUG_FUNC (__msg_get_ptr);
DEBUG_FUNC (__msg_request);
DEBUG_FUNC (__msg_unmap_m);
DEBUG_FUNC (__msg_unmap);
DEBUG_FUNC (__msg_map);
DEBUG_FUNC (__msg_send);
DEBUG_FUNC (__msg_release);
DEBUG_FUNC (__msg_recv);
DEBUG_FUNC (__msg_read_micro);
DEBUG_FUNC (__msg_get_info);
DEBUG_FUNC (__msg_write_micro);
DEBUG_FUNC (msg_body_dtor);
DEBUG_FUNC (msg_new);
DEBUG_FUNC (msg_destroy);
DEBUG_FUNC (msg_new_from_ref);
DEBUG_FUNC (msg_new_from_msg);
DEBUG_FUNC (msgq_new);
DEBUG_FUNC (msgq_destroy);
DEBUG_FUNC (sys_msg_request);
DEBUG_FUNC (sys_msg_unmap);
DEBUG_FUNC (sys_msg_map);
DEBUG_FUNC (sys_msg_send);
DEBUG_FUNC (sys_msg_release);
DEBUG_FUNC (sys_msg_recv);
DEBUG_FUNC (sys_msg_read_micro);
DEBUG_FUNC (sys_msg_write_micro);
DEBUG_FUNC (sys_msg_get_info);
DEBUG_FUNC (init_msg_queues);
