AUTOMAKE_OPTIONS = subdir-objects
AM_CPPFLAGS=-I include/ -I../../include

noinst_LIBRARIES=libx86.a

# Estos CFLAGS son sagrados. Tócalos, y te cargas el kernel entero.

libx86_a_CFLAGS=-fstrength-reduce \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb -march=i686 \
		-D__386__ @AM_CFLAGS@

libx86_a_CCASFLAGS=-DASM -D__386__ @AM_CCASFLAGS@

libx86_a_SOURCES=8259-pic.c    \
                 arch.c        \
                 cpuid.c       \
                 gdt.c         \
                 interrupt.c   \
                 intr.S        \
                 irq.c         \
                 multiboot.c   \
                 paging.c      \
                 regdump.c     \
                 seg.S         \
		 upperhalf.c   \
                 task.c        \
                 video.c
