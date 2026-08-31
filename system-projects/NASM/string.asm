section .text
    global stringlen
    global stringcpy
    global stringcmp
    global stringequ
    global memocpy

; size_t stringlen(const char *s)
stringlen:
    xor eax, eax
.loop:
    cmp byte [rdi + rax], 0 ; проверяем конец строки
    je .done
    inc rax                 ; переходим к следующему символу
    jmp .loop               ; и увеличиваем счетчик символов
.done:
    ret

; char *stringcpy(char *dst, const char *src)
stringcpy:
    mov rax, rdi  ; сохраняем адрес dest
.loop:
    mov dl, [rsi] ; копируем байт по адресу rsi
    mov [rdi], dl ; копируем по адресу rdi байт
    inc rsi       ; переход к следующему символу
    inc rdi
    test dl, dl   ; проверка на конец строки
    jnz .loop     ; если не 0, то повторяем цикл
    ret           ; возврат, адрес dst лежит в rax

; int stringcmp(const char *s1, const char *s2)
stringcmp:
.loop:
    movzx eax, byte [rdi] ; сохраняем символ приемника
    movzx edx, byte [rsi] ; сохраняем символ источника
    cmp eax, edx          ; сравниваем их
    jne .notequ
    test eax, eax         ; проверка на конец строки
    je .done
    inc rdi               ; переход к следующему символу
    inc rsi
    jmp .loop
.notequ:
    sub eax, edx          ; возвращаем разность
    ret                   ; при неравенстве
.done:
    xor eax, eax          ; возвращаем 0
    ret

; int stringequ(const char *s1, const char *s2)
; аналогична strcmp, но 1 в случае равенства
; в противном случае - 0
stringequ:
.loop:
    movzx eax, byte [rdi]
    movzx edx, byte [rsi]
    cmp eax, edx
    jne .false
    test eax, eax
    je .true
    inc rdi
    inc rsi
    jmp .loop
.false:
    xor eax, eax
    ret
.true:
    mov eax, 1
    ret

; void *memocpy(void *dst, cond void *src, size_t nbytes)
memocpy:
    mov rax, rdi     ; сохранние адреса приемника
    mov rcx, rdx     ; становление счетчика байтов
    test rcx, rcx    ; проверка на 0
    je .done
.loop:
    mov dl, [rsi]  ; копирование байта rsi
    mov [rdi], dl   ; копирование байта по адресу rdi
    inc rsi
    inc rdi
    dec rcx          ; уменьшение счетчика
    test rcx, rcx    ; проверка на 0
    je .done
    jmp .loop
.done:
    ret

; информация для ассемблера, что стек не является исполняемым
section .note.GNU-stack noalloc noexec nowrite progbits
