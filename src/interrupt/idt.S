[GLOBAL reload_idt]
[GLOBAL interrupt_exit]

reload_idt:
  mov eax, [esp + 4]  ; arg 1
  lidt [eax]
  ret

; ********************************* exceptions ************************************** ;
%macro DEFINE_ISR_NOERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro DEFINE_ISR_ERRCODE 1
  [GLOBAL isr%1]
  isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro

DEFINE_ISR_NOERRCODE  0
DEFINE_ISR_NOERRCODE  1
DEFINE_ISR_NOERRCODE  2
DEFINE_ISR_NOERRCODE  3
DEFINE_ISR_NOERRCODE  4
DEFINE_ISR_NOERRCODE  5
DEFINE_ISR_NOERRCODE  6
DEFINE_ISR_NOERRCODE  7
DEFINE_ISR_ERRCODE    8
DEFINE_ISR_NOERRCODE  9
DEFINE_ISR_ERRCODE    10
DEFINE_ISR_ERRCODE    11
DEFINE_ISR_ERRCODE    12
DEFINE_ISR_ERRCODE    13
DEFINE_ISR_ERRCODE    14
DEFINE_ISR_NOERRCODE  15
DEFINE_ISR_NOERRCODE  16
DEFINE_ISR_NOERRCODE  17
DEFINE_ISR_NOERRCODE  18
DEFINE_ISR_NOERRCODE  19
DEFINE_ISR_NOERRCODE  20
DEFINE_ISR_NOERRCODE  21
DEFINE_ISR_NOERRCODE  22
DEFINE_ISR_NOERRCODE  23
DEFINE_ISR_NOERRCODE  24
DEFINE_ISR_NOERRCODE  25
DEFINE_ISR_NOERRCODE  26
DEFINE_ISR_NOERRCODE  27
DEFINE_ISR_NOERRCODE  28
DEFINE_ISR_NOERRCODE  29
DEFINE_ISR_NOERRCODE  30
DEFINE_ISR_NOERRCODE  31

; ********************************* external interrupts ************************************** ;
DEFINE_ISR_NOERRCODE   32
DEFINE_ISR_NOERRCODE   33
DEFINE_ISR_NOERRCODE   34
DEFINE_ISR_NOERRCODE   35
DEFINE_ISR_NOERRCODE   36
DEFINE_ISR_NOERRCODE   37
DEFINE_ISR_NOERRCODE   38
DEFINE_ISR_NOERRCODE   39
DEFINE_ISR_NOERRCODE   40
DEFINE_ISR_NOERRCODE   41
DEFINE_ISR_NOERRCODE   42
DEFINE_ISR_NOERRCODE   43
DEFINE_ISR_NOERRCODE   44
DEFINE_ISR_NOERRCODE   45
DEFINE_ISR_NOERRCODE   46
DEFINE_ISR_NOERRCODE   47



; ************************************* isr_common_stub **************************************** ;
[EXTERN isr_handler]
[EXTERN schedule]

; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level isr handler,
; and finally restores the stack frame.
isr_common_stub:
  ; save common registers
  pusha

  ; save original data segment
  mov ax, ds
  push eax

  ; load the kernel data segment descriptor
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  call isr_handler

interrupt_exit:
  ; call scheduler
  call schedule

  ; recover the original data segment
  pop eax
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  popa
  ; clean up the pushed error code and pushed ISR number
  add esp, 8

  ; make sure interrupt is enabled
  sti
  ; pop cs, eip, eflags, user_ss, and user_esp by processor
  iret
