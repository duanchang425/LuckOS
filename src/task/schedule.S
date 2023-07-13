[GLOBAL cpu_idle]
[GLOBAL context_switch]
[GLOBAL switch_to_user_mode]
[GLOBAL resume_thread]

[EXTERN interrupt_exit]

cpu_idle:
  hlt
  ret

context_switch:
  push eax
  push ecx
  push edx
  push ebx
  push ebp
  push esi
  push edi
  mov eax, [esp + 32]
  mov [eax], esp

  ; jump to next thread's kernel stack and recover its execution
  mov eax, [esp + 36]
  mov esp, [eax]

resume_thread:
  pop edi
  pop esi
  pop ebp
  pop ebx
  pop edx
  pop ecx
  pop eax

  sti
  ret


switch_to_user_mode:
  add esp, 8
  jmp interrupt_exit
