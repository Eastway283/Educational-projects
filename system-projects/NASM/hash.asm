; hash.asm
; unsigned long hash(const char *str)

section .text
    global hash

hash:
    push rbx      ; сохраняем rbx, чтобы затем восстановить
    mov rbx, 5381 ; начальное значение h

.loop:
    movzx eax, byte [rdi] ; загружаем очередной байт строки (без знака)
    inc rdi               ; сдвигаем указатель на следующий символ
    test eax, eax         ; проверка на конец строки
    je .done              ; если да - выходим

    ; h = h * 33 + c
    mov rcx, rbx          ; rcx = h
    shl rcx, 5            ; rcx = h << 5
    add rbx, rcx          ; rbx = (h << 5) + h
    add rbx, rax          ; rbx = (h << 5) + h + c

    jmp .loop

.done:
    mov rax, rbx          ; сохраняем возвращаемое значение
    pop rbx               ; восстанавливаем rbx
    ret
