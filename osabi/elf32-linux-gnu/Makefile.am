AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS=elf32-linux-gnu

OBJCOPYFLAGS=-R .note -R .note.gnu.build-id -R .comment
            
elf32_linux_gnu_LDFLAGS=--shared -nostdlib
elf32_linux_gnu_LDADD=../../user/libuser.a
elf32_linux_gnu_CFLAGS=-fstrength-reduce -fPIC \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -I../../user/include -ggdb \
    -Iinclude  -I../../arch/include-x86 -I../../microkernel/loaders/elf -march=i686 -D__386__ @AM_CFLAGS@ 
    
elf32_linux_gnu_CCASFLAGS=-I. @AM_CCASFLAGS@

elf32_linux_gnu_SOURCES=\
linux.c \
syscall_list.h \
linux.h \
unistd_32.h \
syscall_list.c \
sys/brk.c \
sys/exit.c \
sys/open.c \
sys/set_thread_area.c \
sys/uname.c \
sys/uname.h \
sys/write.c \
sys/writev.c \
main.c
