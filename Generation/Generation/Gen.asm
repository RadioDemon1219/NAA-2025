.586
.model flat, stdcall
includelib libucrt.lib
includelib kernel32.lib
includelib "../Debug/GenLib.lib"
ExitProcess PROTO:DWORD 
.stack 4096


 outnum PROTO : DWORD

 outstr PROTO : DWORD

 outchr PROTO : DWORD

 concat PROTO : DWORD, : DWORD, : DWORD

 lenght PROTO : DWORD, : DWORD

 atoii  PROTO : DWORD,  : DWORD

.const
		newline byte 13, 10, 0
		LTRL1 sdword 5
.data
		temp sdword ?
		buffer byte 256 dup(0)
		mainfirstnum sdword 0
.code

;----------- MAIN ------------
main PROC
push LTRL1
push LTRL1
push LTRL1
pop ebx
pop eax
imul eax, ebx
push eax
pop ebx
pop eax
add eax, ebx
push eax
push LTRL1
push LTRL1
pop ebx
pop eax
imul eax, ebx
push eax
push LTRL1
pop ebx
pop eax
add eax, ebx
push eax
push LTRL1
pop ebx
pop eax
imul eax, ebx
push eax
pop ebx
pop eax
add eax, ebx
push eax

pop ebx
mov mainfirstnum, ebx


push mainfirstnum
call outnum

push 0
call ExitProcess
main ENDP
end main
