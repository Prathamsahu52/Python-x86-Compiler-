global.print:
    pushq   %rbp
    movq    %rsp, %rbp
    pushq   %rbx
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15

    # Ensure stack alignment for printf
    movq    %rsp, %rax          # Store current rsp
    andq    $15, %rax           # Check lower 4 bits, rsp should be 16-byte aligned before call
    jz      align_ok            # If zero, alignment is okay
    subq    $8, %rsp            # Adjust stack to be 16-byte aligned
    movq    $1, -192(%rbp)        # Mark that we adjusted the stack

align_ok:
    # Check type in %rdi (1=string, otherwise integer)
    cmpq    $0, %rdi
    je      print_string

    # Setup for printing integer
    
    leaq    integer_format(%rip), %rdi # Load format string for integer
    movq    16(%rbp), %rsi             # Load integer value from stack
    xor     %rax, %rax                 # Float register count
    call    printf                     # Call printf
    jmp     cleanup                    # Jump to cleanup

print_string:
    # Setup for printing string
    leaq    string_format(%rip), %rdi  # Load format string for string
    movq    16(%rbp), %rsi             # Load string pointer from stack
    xor     %rax, %rax                 # Float register count
    call    printf                     # Call printf

cleanup:
    # Restore the stack if it was misaligned
    movb    -192(%rbp), %al              # Check if we marked the stack as adjusted
    testb   $1, %al
    jz      stack_ok
    addq    $8, %rsp                   # Restore original stack alignment

stack_ok:
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %rbx
    popq    %rbp
    ret   