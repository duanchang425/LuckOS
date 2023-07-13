; macros definition

MBR_BASE_ADDR          equ  0x7C00
LOADER_BASE_ADDR       equ  0x8000
LOADER_START_SECTOR    equ  1

;********************************** gdt ***************************************;
DESC_G_4K  equ  1 << 23
DESC_D_32  equ  1 << 22
DESC_L_32  equ  0 << 21
DESC_AVL   equ  0 << 20

DESC_P      equ  1 << 15

DESC_DPL_0  equ  00b << 13
DESC_DPL_1  equ  01b << 13
DESC_DPL_2  equ  10b << 13
DESC_DPL_3  equ  11b << 13

DESC_S_CODE  equ  1 << 12
DESC_S_DATA  equ  1 << 12
DESC_S_SYS   equ  0 << 12

DESC_TYPE_CODE equ 1000b << 8
DESC_TYPE_DATA equ 0010b << 8

; code segment
DESC_LIMIT_CODE_0_15    equ   0xFFFF
DESC_LIMIT_CODE_16_19   equ   0xF << 16

DESC_CODE_HIGH_32       equ   (0 << 24) + \
                              DESC_G_4K + \
                              DESC_D_32 + \
                              DESC_L_32 + \
                              DESC_AVL + \
                              DESC_LIMIT_CODE_16_19 + \
                              DESC_P + \
                              DESC_DPL_0 + \
                              DESC_S_CODE + \
                              DESC_TYPE_CODE + \
                              0

DESC_CODE_LOW_32        equ   (0 << 16) + DESC_LIMIT_CODE_0_15

; data segment
DESC_LIMIT_DATA_0_15    equ   0xFFFF
DESC_LIMIT_DATA_16_19   equ   0xF << 16

DESC_DATA_HIGH_32       equ   (0 << 24) + \
                              DESC_G_4K + \
                              DESC_D_32 + \
                              DESC_L_32 + \
                              DESC_AVL + \
                              DESC_LIMIT_DATA_16_19 + \
                              DESC_P + \
                              DESC_DPL_0 + \
                              DESC_S_DATA + \
                              DESC_TYPE_DATA + \
                              0

DESC_DATA_LOW_32        equ   (0 << 16) + DESC_LIMIT_DATA_0_15

; video segment
DESC_LIMIT_VIDEO_0_15   equ   0x0007  ; video has 8 pages
DESC_LIMIT_VIDEO_16_19  equ   0 << 16

DESC_VIDEO_HIGH_32      equ   (0 << 24) + \
                              DESC_G_4K + \
                              DESC_D_32 + \
                              DESC_L_32 + \
                              DESC_AVL + \
                              DESC_LIMIT_VIDEO_16_19 + \
                              DESC_P + \
                              DESC_DPL_0 + \
                              DESC_S_DATA + \
                              DESC_TYPE_DATA + \
                              0xB

DESC_VIDEO_LOW_32       equ   (0x8000 << 16) + DESC_LIMIT_VIDEO_0_15


;*************************** segment selector *********************************;
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b

TI_GDT equ 000b
TI_LDT equ 100b

SELECTOR_CODE    equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA    equ (0x0002 << 3) + TI_GDT + RPL0
SELECTOR_VIDEO   equ (0x0003 << 3) + TI_GDT + RPL0

;********************************* page ***************************************;
PAGE_SIZE                         equ  0x1000

PAGE_DIR_PHYISCAL_ADDR            equ  0x100000 + PAGE_SIZE
PAGE_DIR_VIRTUAL_ADDR             equ  0xC0701000
PAGE_TABLES_VIRTUAL_ADDR_START    equ  0xC0400000

PG_P     equ  1
PG_RW_R  equ  0 << 1
PG_RW_W  equ  1 << 1
PG_US_S  equ  0 << 2
PG_US_U  equ  1 << 2

;******************************** kernel **************************************;
; TODO: detect machine physical memory size
PHY_MEM_SIZE  equ  32 * 1024 * 1024

KERNEL_START_SECTOR     equ   9
KERNEL_BIN_MAX_SIZE     equ   1024 * 1024  ; 1MB
KERNEL_SECTORS          equ   KERNEL_BIN_MAX_SIZE / 512

KERNEL_BIN_LOAD_MEM_MAX         equ   0xFFFFFFFF
KERNEL_BIN_LOAD_VIRTUAL_ADDR    equ   KERNEL_BIN_LOAD_MEM_MAX - KERNEL_BIN_MAX_SIZE + 1  ; 0xFFF00000
KERNEL_BIN_LOAD_PHYSICAL_ADDR   equ   PHY_MEM_SIZE - PAGE_SIZE - KERNEL_BIN_MAX_SIZE

KERNEL_VIRTUAL_ADDR_START       equ   0xC0800000
KERNEL_PHYSICAL_ADDR_START      equ   0x200000  ; 2MB
KERNEL_SIZE_MAX                 equ   1024 * 1024

KERNEL_STACK_TOP                equ   0xF0000000
KERNEL_STACK_PHYSICAL_ADDR      equ   PHY_MEM_SIZE - PAGE_SIZE
