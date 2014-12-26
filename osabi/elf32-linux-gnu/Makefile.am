AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS=elf32-linux-gnu

OBJCOPYFLAGS=-R .note -R .note.gnu.build-id -R .comment
            
elf32_linux_gnu_LDFLAGS=--shared -nostdlib

elf32_linux_gnu_CFLAGS=-fstrength-reduce -fPIC \
		-finline-functions -nostdinc -nostdlib -fno-builtin -I. -ggdb \
    -Iinclude -march=i686 -D__386__
    
elf32_linux_gnu_CCASFLAGS=-I.

elf32_linux_gnu_SOURCES=atomik.h ipc.c linux.c linux.h 	main.c system.c
