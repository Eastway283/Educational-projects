section .text
    global bitcount
    global is_power_of_two
    global ctz
    global clz
    global parity
    global set_bit
    global clear_bit
    global toggle_bit
    global mask_low_bits


; size_t bitcount(uint32_t n)
bitcount:
    xor eax, eax        ; обнуляем регистр
.loop:
    test edi, edi       ; проверка на 0
    je .done            ; если истино, то выходим
    inc eax             ; увеличиваем счетчик
    lea esi, [edi - 1]  ; используем трюк x & (x - 1)
    and edi, esi
    jmp .loop           ; повторяем цикл
.done:
    ret

; uint32_t set_bit(uint32_t x, int n)
set_bit:
    mov eax, edi
    mov ecx, esi
    mov edx, 1
    shl edx, cl
    or eax, edx
    ret

; uint32_t clear_bit(uint32_t x. int n)
clear_bit:
    mov eax, edi
    mov ecx, esi
    mov edx, 1
    shl edx, cl
    not edx
    and eax, edx
    ret

; uint32_t toggle_bit(uint32_t x, int n)
toggle_bit:
    mov eax, edi
    mov ecx, esi
    mov edx, 1
    shl edx, cl
    xor eax, edx
    ret

; int is_power_of_two(uint32_t n)
is_power_of_two:
    lea esi, [edi - 1]
    and esi, edi        ; esi = (edi - 1) & edi
    test esi, esi       ; проверяем, степень ли
    sete al             ; если да, то устанавливаем al
    test edi, edi       ; проверка на изначальный ноль
    setne cl            ; если не равно нулю, то ставим cl
    and al, cl          ; проверка двух условий
    movzx eax, al
    ret

; int parity(uint32_t x)
parity:
    xor eax, eax
.loop:
    test edi, edi
    je .done
    inc eax
    lea esi, [edi - 1]
    and edi, esi
    jmp .loop
.done:
    and eax, 1          ; тоже самое, что и bitcount, но проверка на четность 
    ret

; unsigned int mask_low_bits(uint32_t n)
; возвращает установленную маску в 0 - n-1 
mask_low_bits:
    xor eax, eax    
    test edi, edi   ; проверка на 0
    jle .done
    cmp edi, 32     ; нельза маскировать больше 32 бит
    jge .all
    mov esi, 1      ; esi = 1
    mov cl, edi     ; cl = n
    shl esi, cl     ; esi = esi << n
    dec esi         ; esi = (esi << n) - 1
    mov eax, esi    ; return mask
.done:
    ret
.all:
    not eax         ; toggle all bits
    ret

; int ctz(uint32_t n)
; возвращает количество нулевых битов справа
; аналогична clz, но порядок другой
; (см. ниже)
ctz:
    test edi, edi
    je .zero
    xor eax, eax
.loop:
    shr edi, 1
    jc .done
    inc eax
    jmp .loop
.zero:
    mov eax, -1
.done:
    ret

; int clz(uint32_t n)
; возвращает количество битов слева
clz:
    test edi, edi        ; проверка на 0
    je .zero             ; n == 0 -> 32
    xor eax, eax
.loop:
    shl edi, 1           ; edi = (edi << 1)
    jc .done             ; старший бит выталкивается в CF
    inc eax              ; единицы нету, продолжение
    jmp .loop
.zero:
    mov eax, 32
.done:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
