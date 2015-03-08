AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES=libx86.a

libx86_a_CFLAGS=-fstrength-reduce -finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb -march=i686 -D__386__ -Iinclude/ -I../../microkernel/include -I../include-x86 -I../include -I../../common/include @AM_CFLAGS@

libx86_a_CCASFLAGS=-DASM -D__386__ -Iinclude -I../include-x86 -I../include @AM_CCASFLAGS@

libx86_a_SOURCES=8259-pic.c    \
                 arch.c        \
                 cpuid.c       \
                 gdt.c         \
                 interrupt.c   \
                 intr.S        \
                 irq.c         \
		 lock.c        \
                 multiboot.c   \
                 paging.c      \
                 regdump.c     \
                 seg.S         \
	         serial.c      \
		 syscall.c     \
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
                 include/asm/syscall.h \
                 include/asm/upperhalf.h \
                 include/asm/irq.h
