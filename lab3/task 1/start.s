section .data
    inPutString: db "-i"
    outPutString: db "-o"

    new_line: db "",10      ; "\n"
    finishString: db "", 0  ; EOF.
    INPUT: dd 0         
    OUTPUT: dd 1

section .bss
    buffer resb 4

section .text
global _start
global system_call
global main

extern strncmp
extern strlen

_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
                            ; Back to caller
main:
    push    ebp                 
    mov     ebp, esp     

    mov edi, [ebp + 8]       
    mov esi, [ebp + 12]       
    
                                
    mainLoop:
        add esi, 4            
        dec edi
        jz loopArgs         
        
        mov edx, [esi]
        call handleI ;handle -i
        mov edx, [esi]
        call handleO ;handle -0

        jmp mainLoop

    ;;reads all args
    loopArgs:

        ; Load args for read.
        mov eax, 3         
        mov ebx, [INPUT]     
        mov ecx, buffer     
        mov edx, 1             

        int 0x80            

        cmp eax,0 ;check if 0 so finished
        
        jle finish           

        mov bl, byte [buffer]      
        mov bh, byte [finishString]
        cmp bl, bh              
        jz finish ;           

        mov bl, byte [buffer]      
        mov bh, byte [new_line]
        cmp bl, bh 
        
        jz jumper;             ; don't encrypt

        ; encrypt the letter
        mov ecx, [buffer]
        add ecx, 1
        mov [buffer], ecx 
        
        jumper:                

        ; Load args for write.
        mov eax, 4
        mov ebx, [OUTPUT]
        mov ecx, buffer
        mov edx, 1

        int 0x80           

        jmp loopArgs

    finish:
        
        mov eax, 0
        mov esp, ebp
        pop ebp
        ret


jmp loopArgs

;Checks if the current argument is -i and opens the input file for reading
handleI:             
    
    pushad
    push 2                      ; -i
    push edx                  
    push inPutString
    call strncmp               

    add esp, 12       

    cmp eax, 0                
    popad
    
    jnz stillDefInput          

    add edx, 2              
    
    mov eax, 5              
    mov ebx, edx          
    mov ecx, 2                  
    mov edx, 0777           
    
    int 0x80               

    mov [INPUT], eax           
    
    stillDefInput:
        ret


;;Checks if the current argument is -o and opens the output file for appending.
handleO:

    pushad
    push 2                      ; -o
    push edx                    
    push outPutString
    call strncmp                         
    
    add esp, 12              

    cmp eax, 0                
    popad
    jnz stillDefOutput
    add edx, 2                  ; skip the -o


    mov eax, 5                
    mov ebx, edx              
    mov ecx, 1024             
    or ecx, 1                 
    or ecx, 64                
    mov edx, 0777               ; read file
    int 0x80                  

    mov [OUTPUT], eax
    
    stillDefOutput:
        ret