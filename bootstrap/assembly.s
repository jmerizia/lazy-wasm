.file "test.lang"
.text
.globl main
.type main, @function

main:
    mov $4, %eax
    mov $1, %ebx
    mov $msg, %ecx
    int $0x80

.data
    msg: .ascii "Hello, world!"
