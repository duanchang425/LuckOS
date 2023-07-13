[GLOBAL read_disk]

read_disk:
  push ebp
  mov ebp, esp
  push edi
  push esi
  push edx
  push ebx

  mov ebx, [esp + 24]
  mov eax, [esp + 28]
  mov ecx, [esp + 32]

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

  pop ebx
  pop edx
  pop esi
  pop edi
  pop ebp
  ret
