; mbr
;------------------------------------------------------------
%include "src/boot/boot.inc"
SECTION mbr vstart=MBR_BASE_ADDR

mbr_entry:
  call init_segments
  call load_loader_img

  ; jump to loader
  jmp LOADER_BASE_ADDR

init_segments:
  mov ax, cs
  mov ds, ax
  mov es, ax
  mov ss, ax
  mov fs, ax

  ; let's move stack to 0x7b00 :)
  ; we need to copy the current return address to new stack base
  mov bx, sp
  mov ax, [bx]
  mov [0x7b00], ax
  mov sp, 0x7b00
  ret

load_loader_img:
  ; 28-bit LBA, so use 32-bit eax register
  mov eax, LOADER_START_SECTOR  ; 0x01
  mov bx, LOADER_BASE_ADDR  ; 0x8000
  mov cx, 0x08  ; loader max size 8 * 512 = 4KB
  call read_disk
  ret

read_disk:
  mov esi, eax
  mov di, cx

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
  out dx,al

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

.not_ready:
  nop
  in al, dx
  and al, 0x88  ; bit 7 (busy), bit 3 (data ready)
  cmp al, 0x08
  jnz .not_ready

  ; di = cx = sector count
  ; read 2 bytes time, so loop (sector count) * 512 / 2 times
  mov ax, di
  mov dx, 256
  mul dx
  mov cx, ax

  mov dx, 0x1f0

.go_on_read:
  in ax, dx
  mov [bx], ax
  add bx, 2
  loop .go_on_read
  ret


times 510-($-$$) db 0
db 0x55, 0xaa
