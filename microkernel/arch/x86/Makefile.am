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
	         serial.c      \
		 upperhalf.c   \
                 task.c        \
                 video.c       \
                 include/multiboot.h \
                 include/vga/crtc.h \
                 include/asm/cpuid.h \
                 include/asm/8259-pic.h \
                 include/asm/timer.h \
                 include/asm/seg.h \
                 include/asm/com.h \
                 include/asm/regs.h \
                 include/asm/io.h \
                 include/asm/task.h \
                 include/asm/ports.h \
                 include/asm/pagedir.h \
                 include/asm/interrupt.h \
                 include/asm/upperhalf.h \
                 include/asm/irq.h
