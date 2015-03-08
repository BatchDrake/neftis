# Makefile para neftis kernel
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS=loaders/elf scheds/default
noinst_PROGRAMS=neftis

OBJCOPYFLAGS=-R .note -R .note.gnu.build-id -R .comment

neftis_LDADD=../common/libcommon.a ../arch/@AM_ARCH@/lib@AM_ARCH@.a loaders/elf/libelf.a scheds/default/libdefsched.a
            
neftis_LDFLAGS=-Wl,-T../arch/@AM_ARCH@/kernel.lds 

# con -fomit-frame-pointer me quedo sin punteros en EBP
# para un volcado de pila. Pff.

neftis_CFLAGS=-fstrength-reduce \
              -finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb \
              -Iinclude -march=i686 -D__386__ @AM_CFLAGS@ -I../arch/include \
              -I../arch/include-@AM_ARCH@ -I../common/include
    
neftis_CCASFLAGS=-Iinclude -I../arch/@AM_ARCH@/include @AM_CCASFLAGS@

neftis_SOURCES=../arch/@AM_ARCH@/boot.S \
               anonmap.c       \
               buddy.c         \
               console.c       \
               event.c         \
               exception.c     \
               irq.c           \
               kctx.c          \
               loader.c        \
               main.c          \
	       msg.c           \
               mutex.c         \
	       object.c        \
               printk.c        \
               sched.c         \
               slab.c          \
               salloc.c        \
	       serial.c        \
               srvrd.c         \
               syscall.c       \
               sysipc.c        \
               syskrn.c        \
               task.c          \
               timer.c         \
               util.c          \
               vremap.c        \
               vm.c            \
               waitqueue.c     \
               include/ctype.h \
               include/ansi.h \
               include/console/console.h \
               include/console/video.h \
               include/console/condefs.h \
               include/kctx.h \
	       include/lock/event.h \
               include/lock/lock.h \
               include/backend.h \
               include/mm/regions.h \
               include/mm/alloc.h \
               include/mm/anonmap.h \
               include/mm/kmalloc.h \
               include/mm/salloc.h \
               include/mm/slab.h \
               include/mm/coloring.h \
               include/mm/anon.h \
               include/mm/vm.h \
               include/mm/vremap.h \
               include/irq/timer.h \
               include/irq/irq.h \
               include/dev/serial.h \
               include/string.h \
               include/config.h \
               include/defines.h \
               include/task/waitqueue.h \
               include/task/task.h \
               include/task/exception.h \
               include/task/sched.h \
               include/task/loader.h \
               include/task/msg.h \
               include/task/syscall.h \
               include/task/sysipc.h \
               include/task/syskrn.h \
               include/stdarg.h \
               include/util.h \
               include/misc/errno.h \
               include/misc/msgsink.h \
               include/misc/vkprintf.h \
               include/misc/list.h \
	       include/misc/object.h \
               include/misc/radix_tree.h \
               include/misc/rbtree.h \
               include/misc/hook.h \
               include/arch.h \
               include/debug.h \
               include/values.h \
               include/types.h 
