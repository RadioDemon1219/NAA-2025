.586
.model flat, stdcall
includelib libucrt.lib
includelib kernel32.lib
includelib "../Debug/GenLib.lib"
ExitProcess PROTO:DWORD
.stack 4096

 outnum PROTO : DWORD

 outstr PROTO : DWORD

 outchar PROTO : BYTE

 strSize PROTO : DWORD

 strCompare PROTO : DWORD, : DWORD
.data
		newline byte 13, 10, 0
		temp_switch sword 0
		mainnum1 sword 0
		LTRL1 sword 62
		mainnum2 sword 0
		LTRL2 sword 10
		mainsymbol byte 0
		LTRL3 byte 70
		LTRL4 byte "Переменные: ", 0
		byte 243 dup(0)
		LTRL5 byte ", ", 0
		byte 253 dup(0)
		LTRL6 byte ", символ: ", 0
		byte 245 dup(0)
		LTRL7 byte "После инкремента num1 = ", 0
		byte 231 dup(0)
		LTRL8 byte "После декремента num2 = ", 0
		byte 231 dup(0)
		mainflag byte 0
		LTRL9 byte 0
		LTRL10 byte "Инверсия false = ", 0
		byte 238 dup(0)
		mainstr1 dword 0
		LTRL11 byte "Window", 0
		byte 249 dup(0)
		mainstr2 dword 0
		mainstr3 dword 0
		LTRL12 byte "Linux", 0
		byte 250 dup(0)
		LTRL13 byte "Сравнение str1 и str2: ", 0
		byte 232 dup(0)
		LTRL14 byte "Сравнение str1 и str3: ", 0
		byte 232 dup(0)
		LTRL15 byte "Длина строки str1: ", 0
		byte 236 dup(0)
		maincode sword 0
		LTRL16 sword 3
		LTRL17 sword 1
		LTRL18 byte "Вы выбрали пункт 1 — Информация", 0
		byte 224 dup(0)
		LTRL19 sword 2
		LTRL20 byte "Вы выбрали пункт 2 — Настройки", 0
		byte 225 dup(0)
		LTRL21 byte "Вы выбрали пункт 3 — Выход", 0
		byte 229 dup(0)
		LTRL22 byte "Неизвестный пункт меню", 0
		byte 233 dup(0)
		LTRL23 byte "Программа выполнена успешно!", 0
		byte 227 dup(0)
.code
main PROC

mov ax, LTRL1
mov mainnum1, ax

mov ax, LTRL2
mov mainnum2, ax

mov al, 70
mov mainsymbol, al

push offset LTRL4
call outstr
movsx eax, mainnum1
push eax
call outnum
push offset LTRL5
call outstr
movsx eax, mainnum2
push eax
call outnum
push offset LTRL6
call outstr
mov al, mainsymbol
push eax
call outchar
push offset newline
call outstr

inc mainnum1

dec mainnum2

push offset LTRL7
call outstr
movsx eax, mainnum1
push eax
call outnum
push offset newline
call outstr

push offset LTRL8
call outstr
movsx eax, mainnum2
push eax
call outnum
push offset newline
call outstr

mov al, 1
mov mainflag, al

push offset LTRL10
call outstr
movsx eax, mainflag
push eax
call outnum
push offset newline
call outstr

mov eax, offset LTRL11
mov mainstr1, eax

mov eax, offset LTRL11
mov mainstr2, eax

mov eax, offset LTRL12
mov mainstr3, eax

push offset LTRL13
call outstr

push mainstr2
push mainstr1
call strCompare
push eax
call outnum
push offset newline
call outstr

push offset LTRL14
call outstr

push mainstr3
push mainstr1
call strCompare
push eax
call outnum
push offset newline
call outstr

push offset LTRL15
call outstr

push mainstr1
call strSize
push eax
call outnum
push offset newline
call outstr

mov ax, LTRL16
mov maincode, ax

mov ax, maincode
mov temp_switch, ax

mov ax, temp_switch
cmp ax, LTRL17
jne label2

push offset LTRL18
call outstr
push offset newline
call outstr

jmp label1

label2:
mov ax, temp_switch
cmp ax, LTRL19
jne label3

push offset LTRL20
call outstr
push offset newline
call outstr

jmp label1

label3:
mov ax, temp_switch
cmp ax, LTRL16
jne label4

push offset LTRL21
call outstr
push offset newline
call outstr

jmp label1

label4:

push offset LTRL22
call outstr
push offset newline
call outstr

jmp label1

label1:

push offset LTRL23
call outstr
push offset newline
call outstr

main ENDP
end main
