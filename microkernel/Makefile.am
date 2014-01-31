# Makefile para neftis kernel
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS=arch/x86
noinst_PROGRAMS=neftis

OBJCOPYFLAGS=-R .note -R .note.gnu.build-id -R .comment

neftis_LDADD=arch/x86/libx86.a
            
neftis_LDFLAGS=-Wl,-Tarch/x86/kernel.lds 

# con -fomit-frame-pointer me quedo sin punteros en EBP
# para un volcado de pila. Pff.

neftis_CFLAGS=-fstrength-reduce \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb \
    -Iinclude -march=i686 -D__386__ @AM_CFLAGS@
    
neftis_CCASFLAGS=-I../include -Iarch/x86/include @AM_CFLAGS@

neftis_SOURCES=arch/x86/boot.S \
               console.c       \
               hook.c          \
               irq.c           \
               kctx.c          \
               kmalloc.c       \
               list.c          \
               loader.c        \
               lock.c          \
               main.c          \
               mm.c            \
               mutex.c         \
               printk.c        \
               sched.c         \
               spalloc.c       \
               stdlib.c        \
               task.c          \
               timer.c         \
               util.c          \
               vm.c            \
               waitqueue.c
               
               
