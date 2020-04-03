#.file "test.lang"
.text
.extern printf
.extern scanf
.global main

_print_int:
    # printf("%d", %edi)
    # Params:
    #     %edi = integer to be printed
    mov %rdi, %rsi
    mov $int_fmt, %rdi
    call printf
    ret

_print_ln:
    # printf("\n")
    mov $newline_fmt, %rdi
    call printf
    ret

_print_string:
    # write(1, %edi, %esi)
    mov $4, %rax
    mov $1, %rbx
    mov %rdi, %rcx
    mov %rsi, %rdx
    int $0x80
    ret

_read_int:
    # scanf("%d", %edi)
    # Params:
    #     none
    # Return:
    #     %edi = set with input integer
    mov $read_fmt, %rdi
    mov $tmp_int, %rsi
    call scanf
    mov $tmp_int, %rsi
    mov (%rsi), %rdi
    ret

main:
    call _read_int
    call _print_int
    call _print_ln

    mov $1, %eax
    mov $0, %ebx
    int $0x80

.data
int_fmt: .asciz "%d"
read_fmt: .asciz "%d"
newline_fmt: .asciz "\n"
tmp_int: .int 0
