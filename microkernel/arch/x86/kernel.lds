OUTPUT_FORMAT("elf32-i386")
ENTRY (start)


SECTIONS{
    /DISCARD/ : { *(.note.gnu.build-id) }
    /DISCARD/ : { *(.note.gnu.gold-version) }
    
    . = 0x00101000;
    kernel_start = .;
    .text :
    {
      text_start = .;
      *(.text)
      . = ALIGN (0x1000);
      text_end = .;
    }

    .data :
    {
      *(.data)
    }

    .rodata :
    {
      *(.rodata)
    }

    .bss :
    {
      bss_start = .;
      *(COMMON)
      *(.bss)
      bss_end = .;
    }
    
    . = ALIGN (0x1000);
    
    kernel_end = .;
}

