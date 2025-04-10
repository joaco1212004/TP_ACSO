; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    push rdi
    mov rdi, 16
    call malloc
    test rax, rax
    jz .return_null

    mov qword [rax], NULL   ; list->first = NULL
    mov qword [rax + 8], NULL ; list->last = NULL

    pop rdi
    ret

.return_null:
    xor rax, rax
    pop rdi
    ret

string_proc_node_create_asm:
    push rdi
    mov rdi, 32
    call malloc
    test rax, rax
    jz .return_null

    mov qword [rax], NULL   ; node->next = NULL
    mov qword [rax + 8], NULL ; node->previous = NULL
    mov qword [rax + 16], rsi ; node->hash = hash
    mov byte [rax + 24], dl ; node->type = type

    pop rdi
    ret

.return_null:
    xor rax, rax
    pop rdi
    ret

string_proc_list_add_node_asm:
    ; rdi = list
    ; rsi = type
    ; rdx = hash

    ; Crear nodo 
    push rdi
    push rsi
    push rdx
    call string_proc_node_create_asm
    test rax, rax
    jz .return

    ; Add nodo
    mov r8, rdi             ; r8 = list
    mov r9, rax             ; r9 = node
    cmp qword [r8], NULL    ; list->first == NULL
    jnz .add_to_end         ; If not NULL, add nodo

    ; If empty, nodo is firts and last
    mov qword [r8], r9      ; list->first = node
    mov qword [r8 + 8], r9  ; list->last = node
    jmp .return

.add_to_end:
    mov r10, qword [r8 + 8] ; r10 = list->last
    mov qword [r10], r9     ; last->next = node
    mov qword [r9 + 8], r10 ; node->previous = last
    mov qword [r8 + 8], r9  ; list->last = node

.return:
    pop rdx
    pop rsi
    pop rdi
    ret

string_proc_list_concat_asm:
    ; rdi = list
    ; rsi = type
    ; rdx = hash

    ; Add nodo to list
    push rdi
    push rsi
    push rdx
    call string_proc_list_add_node_asm

    ; Concatenar first->hash con last->hash
    mov r8, rdi             ; r8 = list
    mov r9, qword [r8]      ; r9 = list->first
    mov r10, qword [r8 + 8] ; r10 = list->last
    mov rsi, qword [r9 + 16]; rsi = first->hash
    mov rdx, qword [r10 + 16]; rdx = last->hash
    call str_concat

    pop rdx
    pop rsi
    pop rdi
    ret
