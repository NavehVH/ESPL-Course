section .data
    hworld db 'hello world', 10  ; Define a string "hello world" followed by a newline character

section .text
    global _start                ; Define the entry point for the linker

    _start:
        ; Write message to stdout
        mov eax, 4               ; System call number for sys_write (4)
        mov ebx, 1               ; File descriptor for stdout (1)
        mov ecx, hworld          ; Pointer to the string to be printed
        mov edx, 12              ; Length of the string (12 bytes, including newline)
        int 0x80                 ; Trigger interrupt to make the system call

        ; Exit the program
        mov eax, 1               ; System call number for sys_exit (1)
        xor ebx, ebx             ; Exit status code (0)
        int 0x80                 ; Trigger interrupt to make the system call

    ; How to compile and run:
    ; nasm -f elf32 hello.asm -o hello.o    ; Assemble the source file into an object file
    ; ld -m elf_i386 hello.o -o hello       ; Link the object file to create an executable
    ; ./hello                               ; Run the executable