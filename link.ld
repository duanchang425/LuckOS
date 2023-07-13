ENTRY(main)
SECTIONS
{
  .text 0xC0800000:
  {
    code = .; _code = .; __code = .;
    *(.text)
  }

  .data ALIGN(4096):
  {
     data = .; _data = .; __data = .;
     *(.data)
     *(.rodata)
  }

  .bss ALIGN(4096):
  {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(4096);
  }

  end = .; _end = .; __end = .;
}
