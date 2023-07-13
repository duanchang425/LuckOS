[GLOBAL atomic_exchange]

atomic_exchange:
  mov ecx, [esp + 4]
  mov eax, [esp + 8]
  xchg [ecx], eax
  ret

compare_and_exchange:
  mov edx, [esp + 4]
  mov ecx, [esp + 8]
  mov eax, 0
  lock cmpxchg [edx], ecx
  ret
