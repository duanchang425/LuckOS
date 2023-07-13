SYSCALL_EXIT_NUM          equ  0
SYSCALL_FORK_NUM          equ  1
SYSCALL_EXEC_NUM          equ  2
SYSCALL_YIELD_NUM         equ  3
SYSCALL_READ_NUM          equ  4
SYSCALL_WRITE_NUM         equ  5
SYSCALL_STAT_NUM          equ  6
SYSCALL_LISTDIR_NUM       equ  7
SYSCALL_PRINT_NUM         equ  8
SYSCALL_WAIT_NUM          equ  9
SYSCALL_THREAD_EXIT_NUM   equ  10
SYSCALL_READ_CHAR_NUM     equ  11
SYSCALL_MOVE_CURSOR_NUM   equ  12


%macro DEFINE_SYSCALL_TRIGGER_0_PARAM 2
  [GLOBAL trigger_syscall_%1]
  trigger_syscall_%1:
    mov eax, %2
    int 0x80
    ret
%endmacro

%macro DEFINE_SYSCALL_TRIGGER_1_PARAM 2
  [GLOBAL trigger_syscall_%1]
  trigger_syscall_%1:
    mov eax, %2
    mov ecx, [esp + 4]
    int 0x80
    ret
%endmacro

%macro DEFINE_SYSCALL_TRIGGER_2_PARAM 2
  [GLOBAL trigger_syscall_%1]
  trigger_syscall_%1:
    mov eax, %2
    mov ecx, [esp + 4]
    mov edx, [esp + 8]
    int 0x80
    ret
%endmacro

%macro DEFINE_SYSCALL_TRIGGER_3_PARAM 2
  [GLOBAL trigger_syscall_%1]
  trigger_syscall_%1:
    push ebx

    mov eax, %2
    mov ecx, [esp + 8]
    mov edx, [esp + 12]
    mov ebx, [esp + 16]
    int 0x80

    pop ebx
    ret
%endmacro

%macro DEFINE_SYSCALL_TRIGGER_4_PARAM 2
  [GLOBAL trigger_syscall_%1]
  trigger_syscall_%1:
    push ebx
    push esi

    mov eax, %2
    mov ecx, [esp + 12]
    mov edx, [esp + 16]
    mov ebx, [esp + 20]
    mov esi, [esp + 24]
    int 0x80

    pop esi
    pop ebx
    ret
%endmacro

%macro DEFINE_SYSCALL_TRIGGER_5_PARAM 2
  [GLOBAL trigger_syscall_%1]
  trigger_syscall_%1:
    push ebx
    push esi
    push edi

    mov eax, %2
    mov ecx, [esp + 16]
    mov edx, [esp + 20]
    mov ebx, [esp + 24]
    mov esi, [esp + 28]
    mov edi, [esp + 32]
    int 0x80

    pop edi
    pop esi
    pop ebx
    ret
%endmacro

; *****************************************************************************
DEFINE_SYSCALL_TRIGGER_1_PARAM   exit,         SYSCALL_EXIT_NUM
DEFINE_SYSCALL_TRIGGER_0_PARAM   fork,         SYSCALL_FORK_NUM
DEFINE_SYSCALL_TRIGGER_3_PARAM   exec,         SYSCALL_EXEC_NUM
DEFINE_SYSCALL_TRIGGER_0_PARAM   yield,        SYSCALL_YIELD_NUM
DEFINE_SYSCALL_TRIGGER_4_PARAM   read,         SYSCALL_READ_NUM
DEFINE_SYSCALL_TRIGGER_4_PARAM   write,        SYSCALL_WRITE_NUM
DEFINE_SYSCALL_TRIGGER_2_PARAM   stat,         SYSCALL_STAT_NUM
DEFINE_SYSCALL_TRIGGER_1_PARAM   listdir,      SYSCALL_LISTDIR_NUM
DEFINE_SYSCALL_TRIGGER_2_PARAM   print,        SYSCALL_PRINT_NUM
DEFINE_SYSCALL_TRIGGER_2_PARAM   wait,         SYSCALL_WAIT_NUM
DEFINE_SYSCALL_TRIGGER_0_PARAM   thread_exit,  SYSCALL_THREAD_EXIT_NUM
DEFINE_SYSCALL_TRIGGER_0_PARAM   read_char,    SYSCALL_READ_CHAR_NUM
DEFINE_SYSCALL_TRIGGER_2_PARAM   move_cursor,  SYSCALL_MOVE_CURSOR_NUM
