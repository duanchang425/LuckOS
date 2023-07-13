%include "src/boot/boot.inc"

SECTION loader vstart=LOADER_BASE_ADDR

loader_entry:
  jmp loader_start

;***************************** data section  **********************************;
cursor_row: dd 0
cursor_col: dd 0

setup_protection_mode_message:
  db "setup protection mode ... "

setup_protection_mode_message_length equ $ - setup_protection_mode_message

setup_page_message:
  db "setup page ... "
  db 0

init_kernel_message:
  db "init kernel ... "
  db 0

panic_message:
  db "PANIC!"
  db 0

; gdt
GDT_BASE:
  dd 0x00000000
  dd 0x00000000

CODE_DESC:
  dd DESC_CODE_LOW_32
  dd DESC_CODE_HIGH_32

DATA_DESC:
  dd DESC_DATA_LOW_32
  dd DESC_DATA_HIGH_32

VIDEO_DESC:
  dd DESC_VIDEO_LOW_32
  dd DESC_VIDEO_HIGH_32

; reserve 60 gdt entries space
times 60 dq 0

GDT_SIZE    equ   $ - GDT_BASE
GDT_LIMIT   equ   GDT_SIZE - 1

gdt_ptr:
  dw GDT_LIMIT
  dd GDT_BASE

;*************************** 16-bits real mode ********************************;
loader_start:
  call clear_screen
  call setup_protection_mode

  jmp $

clear_screen:
  mov byte ah, 0x06
  mov byte al, 0x00
  mov byte bh, 0x07
  ; start (0, 0)
  mov byte cl, 0x00
  mov byte ch, 0x00
  ; end: (dl, dh) = (x:79, y:24)
  mov byte dl, 0x4f
  mov byte dh, 0x18

  int 0x10
  ret

; args:
;  - ax message
;  - cx length
;  - dx position
print_message_real_mode:
  mov bp, ax
  mov ah, 0x13  ; int num
  mov al, 0x01
  mov bh, 0x00  ; page number
  mov bl, 0x07
  int 0x10
  ret

setup_protection_mode:
  mov ax, setup_protection_mode_message
  mov dx, 0x00
  mov cx, setup_protection_mode_message_length
  call print_message_real_mode

  ; enable A20
  in al, 0x92
  or al, 0000_0010b
  out 0x92, al

  ; load GDT
  lgdt [gdt_ptr]

  ; open protection mode - set cr0 bit 0
  mov eax, cr0
  or eax, 0x00000001
  mov cr0, eax

  ; refresh pipeline
  jmp dword SELECTOR_CODE:protection_mode_entry


;************************ 32-bits protection mode *****************************;
[bits 32]
protection_mode_entry:
  ; set data segments
  mov ax, SELECTOR_DATA
  mov ds, ax
  mov es, ax
  mov ss, ax

  ; set video segment
  mov ax, SELECTOR_VIDEO
  mov gs, ax

  ; set cursor
  push setup_protection_mode_message_length
  call move_cursor
  add esp, 4

  ; set protection mode OK
  call print_OK
  call print_new_line

  ; setup page
  push setup_page_message
  call print_message
  add esp, 4

  call setup_page

  call print_OK
  call print_new_line

  ; load kernel
  push init_kernel_message
  call print_message
  add esp, 4

  call init_kernel

  ; should never reach here - we are into the kernel
  push panic_message
  call print_message
  add esp, 4

  jmp $
  ret

; let's use stack to pass arguments
print_message:
  push ebp
  mov ebp, esp
  push esi

  mov esi, [ebp + 8]   ; arg: message ptr

  call get_current_cursor_addr
  mov ecx, eax
  mov eax, 0
.print_byte:
  mov byte dl, [esi + eax]
  cmp dl, 0
  je  .print_end
  mov byte [gs:ecx], dl
  mov byte [gs:ecx+1], 0x7  ; white on black
  add eax, 1
  add ecx, 2
  jmp .print_byte

.print_end:
  push eax
  call move_cursor
  add esp, 4

  pop esi
  pop ebp
  ret


; argument eax: start position to print
print_OK:
  ; let's test video memory - a green [OK] after message1
  call get_current_cursor_addr

  mov byte [gs:(eax+0)], '['
  mov byte [gs:(eax+2)], 'O'
  mov byte [gs:(eax+3)], 0010b
  mov byte [gs:(eax+4)], 'K'
  mov byte [gs:(eax+5)], 0010b
  mov byte [gs:(eax+6)], ']'

  push 4
  call move_cursor
  add esp, 4

  ret

print_new_line:
  mov eax, [cursor_row]
  add eax, 1
  mov dword [cursor_row], eax
  mov dword [cursor_col], 0
  push 0
  call move_cursor
  add esp, 4
  ret

get_current_cursor_addr:
  mov ecx, [cursor_row]
  mov edx, [cursor_col]
  
  mov eax, ecx
  shl eax, 6
  shl ecx, 4
  add eax, ecx  ; ecx * 64 + ecx * 16 = row * 80
  add eax, edx
  shl eax, 1

  ret

move_cursor:
  push ebp
  mov ebp, esp

  mov eax, [ebp + 8]
  mov ecx, [cursor_row]
  mov edx, [cursor_col]
  add edx, eax

.start_new_row:
  cmp edx, 80
  jle .do_move
  inc ecx
  sub edx, 80
  jmp .start_new_row

.do_move:
  mov dword [cursor_row], ecx
  mov dword [cursor_col], edx

  call get_current_cursor_addr
  shr eax, 1
  mov ecx, eax

  mov al, 14
  mov dx, 0x3D4
  out dx, al
  mov al, ch
  mov dx, 0x3D5
  out dx, al

  mov al, 15
  mov dx, 0x3D4
  out dx, al
  mov al, cl
  mov dx, 0x3D5
  out dx, al

  pop ebp
  ret

;****************************** setup paging **********************************;
setup_page:
  ; clear the memory space for page directory
  push PAGE_SIZE
  push PAGE_DIR_PHYISCAL_ADDR
  call clear_memory
  add esp, 8

  ; create page directory
.create_pde:
  ; first pde
  mov eax, PAGE_DIR_PHYISCAL_ADDR - PAGE_SIZE
  or eax, PG_US_U | PG_RW_W | PG_P
  mov [PAGE_DIR_PHYISCAL_ADDR + 0], eax
  mov [PAGE_DIR_PHYISCAL_ADDR + 768 * 4], eax

  ; the second pde - we use this 4MB virtual space for all kernel page tales :)
  mov eax, PAGE_DIR_PHYISCAL_ADDR
  or eax, PG_US_U | PG_RW_W | PG_P
  mov [PAGE_DIR_PHYISCAL_ADDR + 769 * 4], eax

  ; other kernel pde
  mov eax, PAGE_DIR_PHYISCAL_ADDR + PAGE_SIZE
  or eax, PG_US_U | PG_RW_W | PG_P
  mov ecx, 254
  mov edx, PAGE_DIR_PHYISCAL_ADDR + 770 * 4
.create_kernel_pde:
  mov [edx], eax
  add eax, PAGE_SIZE
  add edx, 4
  loop .create_kernel_pde

  ; create the first page table, for 1MB low memory
  mov eax, 0
  or eax, PG_US_U | PG_RW_W | PG_P
  mov ecx, 256
  mov edx, PAGE_DIR_PHYISCAL_ADDR - PAGE_SIZE
.create_pte:
  mov [edx], eax
  add eax, PAGE_SIZE
  add edx, 4
  loop .create_pte

  call enable_page
  ret

; set contineous kernel pages mapping for virtual memory -> physical memory
;  - arg 1: virtual addr start (page aligned)
;  - arg 2: physical addr start (page aligned)
;  - arg 3: num of pages
;
; caller must make sure the physical memory starting from arg2 is free to use
set_pages_mapping:
  push ebp
  mov ebp, esp
  push esi
  push edi
  push ebx

  mov esi, [ebp + 8]    ; virtual addr start
  mov edi, [ebp + 12]   ; physical addr start
  mov ecx, [ebp + 16]   ; pages

  shr esi, 12  ; page table index
  or edi, PG_US_U | PG_RW_W | PG_P
.map_next_page:
  ; Note we use the VIRTUAL address of page tables, starting from 0xC0400000
  mov [PAGE_TABLES_VIRTUAL_ADDR_START + esi * 4], edi
  add esi, 1
  add edi, PAGE_SIZE
  loop .map_next_page

  pop ebx
  pop edi
  pop esi
  pop ebp
  ret

; clear contineous kernel pages mapping for virtual memory
;  - arg 1: virtual addr start (page aligned)
;  - arg 2: num of pages
clear_pages_mapping:
  push ebp
  mov ebp, esp
  push esi
  push ebx

  mov esi, [ebp + 8]    ; virtual addr start
  mov ecx, [ebp + 12]   ; pages

  shr esi, 12  ; page table index
.clear_next_page_map:
  ; Note we use the VIRTUAL address of page tables, starting from 0xC0400000
  mov dword [PAGE_TABLES_VIRTUAL_ADDR_START + esi * 4], 0
  add esi, 1
  add edi, PAGE_SIZE
  loop .clear_next_page_map

  pop ebx
  pop esi
  pop ebp
  ret

enable_page:
  sgdt [gdt_ptr]

  ; move the video segment to > 0xC0000000
  mov ebx, [gdt_ptr + 2]
  or dword [ebx + 0x18 + 4], 0xC0000000

  ; move gdt to > 0xC0000000
  add dword [gdt_ptr + 2], 0xC0000000

  ; move stack to 0xC0000000
  mov eax, [esp]
  add esp, 0xc0000000
  mov [esp], eax

  ; set page directory address to cr3 register 
  mov eax, PAGE_DIR_PHYISCAL_ADDR
  mov cr3, eax
  
  ; enable paging on cr0 register
  mov eax, cr0
  or eax, 0x80000000
  mov cr0, eax

  ; load gdt again - gdt has been moved to > 0xC0000000
  lgdt [gdt_ptr]

  ; refresh video segment selector cache
  mov ax, SELECTOR_VIDEO
  mov gs, ax

  ret


;****************************** init kernel ***********************************;
init_kernel:
  call allocate_pages_for_kernel
  call load_hd_kernel_image
  call do_load_kernel
  
  ; init floating point unit before entering the kernel
  finit

  ; move stack to 0xF0000000
  mov esp, KERNEL_STACK_TOP - 16
  mov ebp, esp

  ; let's jump to kernel entry :)
  jmp eax
  ret

allocate_pages_for_kernel:
  ; allocate pages to load kernel bin image
  mov ecx, KERNEL_BIN_MAX_SIZE
  shr ecx, 12
  push ecx  ; arg 3: num of pages to load kernel bin image
  push KERNEL_BIN_LOAD_PHYSICAL_ADDR  ; arg 2: physical addr
  push KERNEL_BIN_LOAD_VIRTUAL_ADDR   ; arg 1: virtual addr
  call set_pages_mapping
  add esp, 12

  ; allocate pages to place kernel sections
  mov ecx, KERNEL_SIZE_MAX
  shr ecx, 12
  push ecx  ; arg 3: num of pages to place kernel
  push KERNEL_PHYSICAL_ADDR_START  ; arg 2: physical addr
  push KERNEL_VIRTUAL_ADDR_START   ; arg 1: virtual addr
  call set_pages_mapping
  add esp, 12

  ; allocate one page for kernel stack
  mov ecx, 2
  push ecx  ; arg 3: num of pages
  push KERNEL_STACK_PHYSICAL_ADDR        ; arg 2: physical addr
  push KERNEL_STACK_TOP - PAGE_SIZE      ; arg 1: virtual addr
  call set_pages_mapping
  add esp, 12

  ret

load_hd_kernel_image:
  mov eax, KERNEL_START_SECTOR
  mov ebx, KERNEL_BIN_LOAD_VIRTUAL_ADDR
  mov ecx, KERNEL_SECTORS
  call read_disk_32
  ret

; load kernel sections to memory
; return the kernel entry point address
do_load_kernel:
  xor eax, eax
  xor ebx, ebx
  xor ecx, ecx
  xor edx, edx

  mov dx, [KERNEL_BIN_LOAD_VIRTUAL_ADDR + 42]   ; e_phentsize
  mov ebx, [KERNEL_BIN_LOAD_VIRTUAL_ADDR + 28]  ; e_phoff
  add ebx, KERNEL_BIN_LOAD_VIRTUAL_ADDR
  mov cx, [KERNEL_BIN_LOAD_VIRTUAL_ADDR + 44]   ; e_phnum

.load_each_segment:
  cmp byte [ebx + 0], 0
  je .next_prgoram_header

  push dword [ebx + 16]  ; p_filesz
  mov eax, [ebx + 4]     ; p_offset
  add eax, KERNEL_BIN_LOAD_VIRTUAL_ADDR
  push eax
  push dword [ebx + 8]   ; p_vaddr
  call memcpy
  add esp, 12

.next_prgoram_header:
  add ebx, edx
  loop .load_each_segment

  ; return the kernel entry address
  mov eax, [KERNEL_BIN_LOAD_VIRTUAL_ADDR + 24]
  ret

;****************************** init kernel ***********************************;
memcpy:
  cld
  push ebp
  mov ebp, esp
  push esi
  push edi
  push ecx

  mov edi, [ebp + 8]    ; dst
  mov esi, [ebp + 12]   ; src
  mov ecx, [ebp + 16]   ; size
  rep movsb

  pop ecx
  pop edi
  pop esi
  pop ebp
  ret

clear_memory:
  push ebp
  mov ebp, esp

  mov eax, [ebp + 8]   ; start addr
  mov ecx, [ebp + 12]  ; size
.clear_byte:
  mov byte [eax], 0
  add eax, 1
  loop .clear_byte

  pop ebp
  ret

; read data from disk, in 32-bit mode
read_disk_32:
  mov esi, eax
  mov edi, ecx

  ; sector count
  mov dx, 0x01f2
  mov al, cl
  out dx, al

  mov eax, esi

  ; LBA low
  mov dx, 0x1f3
  out dx, al

  ; LBA mid
  shr eax, 8
  mov dx, 0x1f4
  out dx, al

  ; LBA high
  shr eax, 8
  mov dx, 0x1f5
  out dx, al

  ; device reg: LBA[24:28]
  shr eax, 8
  and al, 0x0f

  or al, 0xe0  ; 0x1110, LBA mode
  mov dx, 0x1f6
  out dx, al

  ; command reg: 0x2 read, start reading
  mov dx, 0x1f7
  mov al, 0x20
  out dx, al

.hd_not_ready:
  nop
  in al, dx
  and al, 0x88  ; bit 7 (busy), bit 3 (data ready)
  cmp al, 0x08
  jnz .hd_not_ready

  ; di = cx = sector count
  ; read 2 bytes time, so loop (sector count) * 512 / 2 times
  mov eax, edi
  mov edx, 256
  mul edx
  mov ecx, eax

  mov dx, 0x1f0

.go_on_read_data:
  in ax, dx
  mov [ebx], ax
  add ebx, 2
  loop .go_on_read_data

  ret
