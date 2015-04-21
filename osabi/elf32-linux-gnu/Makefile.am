AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS=elf32-linux-gnu

OBJCOPYFLAGS=-R .note -R .note.gnu.build-id -R .comment
            
elf32_linux_gnu_LDFLAGS=--shared -nostdlib

elf32_linux_gnu_CFLAGS=-fstrength-reduce -fPIC \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb \
    -Iinclude  -I../../arch/include-x86 -I../../microkernel/loaders/elf -march=i686 -D__386__ @AM_CFLAGS@ 
    
elf32_linux_gnu_CCASFLAGS=-I. @AM_CCASFLAGS@

elf32_linux_gnu_SOURCES=\
ctype.h \
linux.c \
syscall_list.h \
stdlib.c \
linux.h \
system.c \
errno.h \
ipc.c \
unistd_32.h \
syscall_list.c \
stdlib.h \
sys/brk.c \
sys/open.c \
sys/uname.c \
sys/uname.h \
sys/writev.c \
main.c \
atomik.h
