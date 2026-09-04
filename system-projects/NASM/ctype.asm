section .text
    global isdigit
    global isalpha
    global isalnum
    global isspace
    global to_lower
    global to_upper

; int isdigit(int c)
isdigit:
    sub edi, '0'    ; получаем численное значение символа
    cmp edi, 9      ; проверяем диапазон от 0 до 9
    setbe al        ; al = 1, если edi <= 9, иначе 0
    movzx eax, al   ; расширяем результат
    ret

; int isalpha(int c)
isalpha:
    mov esi, edi    ; сохраняем значение символа
    sub edi, 'A'    ; отнимаем значение А
    cmp edi, 25     ; если меньше 25 - то буква
    setbe cl        ; cl = 1, если edi <= 25, иначе 0
    sub esi, 'a'    ; проделываем тоже самое с нижним регистром
    cmp esi, 25
    setbe sil
    mov al, cl      ; аккумулируем результат
    or al, sil      ; соединяем результаты проверок
    movzx eax, al
    ret

; int isalnum(int c)
isalnum:
    mov esi, edi
    mov edx, edi
    sub edi, '0'
    cmp edi, 9
    setbe dil       ; dil содержит результат проверки на цифру
    sub esi, 'A'
    cmp esi, 25
    setbe sil       ; sil содержит результат проверки на букву в верхнем регистре
    sub edx, 'a'
    cmp edx, 25
    setbe dl        ; dl содержит результат проверки на букву в нижнем регистре
    mov al, dil
    or al, sil
    or al, dl
    movzx eax, al
    ret

; int isspace(int c)
isspace:
    mov esi, edi
    sub esi, 9
    cmp esi, 4      ; проверяем принадлежность символа к диапазону от 9 до 13
    setbe sil       ; устанавливаем sil, если это так
    cmp edi, 32     ; сравниваем символ с ' '
    sete al
    or al, sil      ; устанавливаем итоговый результат 
    movzx eax, al
    ret

; int to_lower(int c)
to_lower:
    mov eax, edi            ; если это не буква, ничего не делаем
    lea edx, [eax - 32]     ; помещаем в edx значение из регистра eax - 32
    sub edi, 'A'            ; проверка на принадлежность к букве верхнего регистра
    cmp edi, 25             
    cmovbe eax, edx         ; если проверка пройдена, то помещаем букву в нижнем регистре в eax
    ret                     ; если нет, то в eax уже лежит неизмененный символ

; int to_upper(int c)
to_upper:
    mov eax, edi            ; аналогично to_lower, но с приведением к верхнему регистру
    lea edx, [eax + 32]
    sub edi, 'a'
    cmp edi, 25
    cmovbe eax, edx
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
