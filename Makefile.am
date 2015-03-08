AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = common arch/x86 microkernel osabi/elf32-linux-gnu

initrd.img : osabi/elf32-linux-gnu/elf32-linux-gnu
	@echo '*****************************************'
	@echo '********* Building initrd image *********'
	@echo '*****************************************'
	@echo
	rm -Rfv initrd
	mkdir -p initrd/abi
	mkdir -p initrd/serv
	cp osabi/elf32-linux-gnu/elf32-linux-gnu initrd/abi
	tar cvf initrd.img initrd/*
	@echo
	@echo '*****************************************'
	@echo '*********        Done           *********'
	@echo '*****************************************'
	@echo 'Remember to copy file initrd.img to the corresponding boot directory.'
