
--args 0x200
-heap  0x1000
-stack 0x1000

MEMORY
{
    L2SRAM (RWX) : org = 0x800000, len = 0x100000
    DDR3 : org = 0xB0000000, len = 0x10000000
}

SECTIONS
{
    .text: load >> L2SRAM
    .ti.decompress: load > L2SRAM
    .stack: load > L2SRAM
    GROUP: load > L2SRAM
    {
        .bss:
        .neardata:
        .rodata:
    }
    .cinit: load > DDR3
    .pinit: load >> L2SRAM
    .init_array: load > L2SRAM
    .const: load >> L2SRAM
    .data: load >> L2SRAM
    .fardata: load >> L2SRAM
    .switch: load >> L2SRAM
    .sysmem: load > L2SRAM
    .far: load >> L2SRAM
    .args: load > L2SRAM align = 0x4, fill = 0 {_argsize = 0x200; }
    .cio: load >> L2SRAM
    .ti.handler_table: load > L2SRAM
    .c6xabi.exidx: load > L2SRAM
    .c6xabi.extab: load >> L2SRAM
    xdc.meta: load > L2SRAM, type = COPY

	.qmss: load > L2SRAM
	.cppi: load > L2SRAM
	.twiddles: load > L2SRAM

}
