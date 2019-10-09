.data
printf_format_int: .string "%d\n"
printf_format_string: .string "%s\n"
printf_format_nl: .string "\n"
.align 8
herpderp_____: 
.space 16394
heap_pointer: 
.space 16394
heap_next: 
.quad 0

.bss
.text
.globl main
main:
pushq %rbp
movq %rsp, %rbp
pushq $0   #pushing empty value for later assignment of variable
pushq $0   #pushing empty value for later assignment of variable
pushq $0   #pushing empty value for later assignment of variable
pushq $0   #pushing empty value for later assignment of variable
pushq $0   #pushing empty value for later assignment of variable
pushq $0   #pushing empty value for later assignment of variable
pushq $1
popq %rcx
pushq %rbp     #push address of static link
popq %rbx
movq $heap_next, %r14
movq (%r14), %r14
movq $heap_pointer, %r15
addq %r14, %r15
movq %rcx, (%r15)
addq $8, %r15
movq %r15, -8(%rbx)
movq $heap_next, %r14
movq (%r14), %r14
inc %rcx
movq %rcx, %rax
movq $8, %r13
mulq %r13
addq %rax, %r14
movq $heap_next, %r15
movq %r14, (%r15)
pushq $20
popq %rcx
pushq %rbp     #push address of static link
popq %rbx
movq $heap_next, %r14
movq (%r14), %r14
movq $heap_pointer, %r15
addq %r14, %r15
movq %rcx, (%r15)
addq $8, %r15
movq %r15, -16(%rbx)
movq $heap_next, %r14
movq (%r14), %r14
inc %rcx
movq %rcx, %rax
movq $8, %r13
mulq %r13
addq %rax, %r14
movq $heap_next, %r15
movq %r14, (%r15)
pushq $42
popq %rcx
pushq %rbp     #push address of static link
popq %rbx
movq $heap_next, %r14
movq (%r14), %r14
movq $heap_pointer, %r15
addq %r14, %r15
movq %rcx, (%r15)
addq $8, %r15
movq %r15, -24(%rbx)
movq $heap_next, %r14
movq (%r14), %r14
inc %rcx
movq %rcx, %rax
movq $8, %r13
mulq %r13
addq %rax, %r14
movq $heap_next, %r15
movq %r14, (%r15)
pushq $500
popq %rcx
pushq %rbp     #push address of static link
popq %rbx
movq $heap_next, %r14
movq (%r14), %r14
movq $heap_pointer, %r15
addq %r14, %r15
movq %rcx, (%r15)
addq $8, %r15
movq %r15, -32(%rbx)
movq $heap_next, %r14
movq (%r14), %r14
inc %rcx
movq %rcx, %rax
movq $8, %r13
mulq %r13
addq %rax, %r14
movq $heap_next, %r15
movq %r14, (%r15)
pushq $1000
popq %rcx
pushq %rbp     #push address of static link
popq %rbx
movq $heap_next, %r14
movq (%r14), %r14
movq $heap_pointer, %r15
addq %r14, %r15
movq %rcx, (%r15)
addq $8, %r15
movq %r15, -40(%rbx)
movq $heap_next, %r14
movq (%r14), %r14
inc %rcx
movq %rcx, %rax
movq $8, %r13
mulq %r13
addq %rax, %r14
movq $heap_next, %r15
movq %r14, (%r15)
pushq $1337
popq %rcx
pushq %rbp     #push address of static link
popq %rbx
movq $heap_next, %r14
movq (%r14), %r14
movq $heap_pointer, %r15
addq %r14, %r15
movq %rcx, (%r15)
addq $8, %r15
movq %r15, -48(%rbx)
movq $heap_next, %r14
movq (%r14), %r14
inc %rcx
movq %rcx, %rax
movq $8, %r13
mulq %r13
addq %rax, %r14
movq $heap_next, %r15
movq %r14, (%r15)
pushq -8(%rbp)
popq %rax
subq $8, %rax
movq (%rax), %r14
pushq %r14
popq %rbx
leaq printf_format_int(%rip), %rdi
movq %rbx, %rsi
movq $0, %rax
call printf
pushq -16(%rbp)
popq %rax
subq $8, %rax
movq (%rax), %r14
pushq %r14
popq %rbx
leaq printf_format_int(%rip), %rdi
movq %rbx, %rsi
movq $0, %rax
call printf
pushq -24(%rbp)
popq %rax
subq $8, %rax
movq (%rax), %r14
pushq %r14
popq %rbx
leaq printf_format_int(%rip), %rdi
movq %rbx, %rsi
movq $0, %rax
call printf
pushq -32(%rbp)
popq %rax
subq $8, %rax
movq (%rax), %r14
pushq %r14
popq %rbx
leaq printf_format_int(%rip), %rdi
movq %rbx, %rsi
movq $0, %rax
call printf
pushq -40(%rbp)
popq %rax
subq $8, %rax
movq (%rax), %r14
pushq %r14
popq %rbx
leaq printf_format_int(%rip), %rdi
movq %rbx, %rsi
movq $0, %rax
call printf
pushq -48(%rbp)
popq %rax
subq $8, %rax
movq (%rax), %r14
pushq %r14
popq %rbx
leaq printf_format_int(%rip), %rdi
movq %rbx, %rsi
movq $0, %rax
call printf
movq %rbp, %rsp
popq %rbp
end_main:

