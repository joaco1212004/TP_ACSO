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
extern strlen
extern strcpy


string_proc_list_create_asm:
<<<<<<< HEAD
    push rbx

    mov edi, 16
    call malloc
    test rax, rax
    je .fail

    xor rbx, rbx
    mov [rax], rbx         
    mov [rax + 8], rbx     

    pop rbx
    ret

.fail:
    xor rax, rax
    pop rbx
    ret

string_proc_node_create_asm:
    push rbp
    mov rbp, rsp
    push rdi
    push rsi 
=======
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
>>>>>>> ff09292b57d1d0523dc2af2dda0e9af2efbde48e

    mov rdi, 32
    call malloc
    test rax, rax
    jz .return_null

    ; Restore type and hash
    pop rdx    
    pop rcx    

    ; Initialize node fields
    mov qword [rax], 0   
    mov qword [rax + 8], 0 
    mov qword [rax + 16], rcx 
    
    ; Check if hash is NULL
    test rdx, rdx
    jz .cleanup_and_return_null
    
    mov qword [rax + 24], rdx 
    
    mov rsp, rbp
    pop rbp
    ret

.cleanup_and_return_null:
    push rax
    mov rdi, rax
    call free
    pop rax

.return_null:
    xor rax, rax
    mov rsp, rbp
    pop rbp
    ret
string_proc_list_add_node_asm:
<<<<<<< HEAD
    push rbx
    push r12
    push r13

    mov r12, rdi       ; list
    movzx r13d, sil    ; type 
    mov rbx, rdx       ; hash

    test r12, r12
    je .end

    mov rdi, r13  
    mov rsi, rbx       
    call string_proc_node_create_asm
    test rax, rax
    je .end

    ; nuevo nodo → rax
    ; lista vacía: list->first == NULL
    cmp qword [r12], 0
    jne .append

    mov [r12], rax      ; first = nodo
    mov [r12 + 8], rax  ; last = nodo
    jmp .end

.append:
    mov rbx, [r12 + 8]   ; rbx = last
    mov [rbx], rax       ; last->next = nodo
    mov [rax + 8], rbx   ; nodo->prev = last
    mov [r12 + 8], rax   ; list->last = nodo

.end:
    pop r13
    pop r12
    pop rbx
    ret

string_proc_list_concat_asm:
    ; Esta función está correcta
    push rbp
    mov rbp, rsp
    sub rsp, 8          ; Alinear stack para llamadas
    push rbx
    push r12
    push r13
    push r14
    push r15

    ; Guardar argumentos
    mov r12, rdi               ; r12 = list
    mov r13b, sil              ; r13b = type
    mov r14, rdx               ; r14 = hash

    ; Verificar si la lista es NULL
    test r12, r12
    jz .return_null

    ; Verificar si el hash es NULL
    test r14, r14
    jz .return_null

    ; Crear resultado inicial como copia del hash de entrada
    mov rdi, r14
    call strlen
    inc rax                    ; +1 para el null-terminator
    mov rdi, rax
    call malloc
    test rax, rax
    jz .return_null
    
    mov r15, rax               ; r15 = result
    mov rdi, r15
    mov rsi, r14
    call strcpy

    ; Recorrer la lista y concatenar hashes de nodos del tipo indicado
    mov rbx, [r12]             ; rbx = list->first (nodo actual)

.loop:
    test rbx, rbx              ; if (current_node == NULL) terminar
    jz .done

    ; Comparar el tipo del nodo
    movzx eax, byte [rbx + 16] ; eax = current_node->type
    cmp al, r13b               ; if (current_node->type == type)
    jne .next_node

    ; Verificar si el hash del nodo es NULL
    mov rsi, [rbx + 24]        ; rsi = current_node->hash
    test rsi, rsi
    jz .next_node

    ; Concatenar hash
    mov rdi, r15               ; primer argumento: result
    call str_concat
    test rax, rax              ; Verificar si str_concat tuvo éxito
    jz .next_node
    
    ; El resultado se guarda en rax, liberar el anterior y actualizar r15
    mov rdi, r15
    mov r15, rax
    call free

.next_node:
    mov rbx, [rbx]             ; current_node = current_node->next
    jmp .loop

.done:
    mov rax, r15               ; return result
    jmp .return

.return_null:
    xor rax, rax

.return:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    add rsp, 8                 ; Restaurar alineación
    leave
    ret
=======
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
>>>>>>> ff09292b57d1d0523dc2af2dda0e9af2efbde48e
