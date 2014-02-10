;===[ Начало сегмента кода ]==== 
			MYCODE: segment .code 
			START:	;---[ Точка старта ]----
			push eax
			push ebp
			mov ebp, esp

 mov eax, 2
 mov [ebp - 200], eax
 fild dword[ebp - 200]
 fld dword[ebp - 4]
 fxch
 fistp dword[ebp - 4]
 fld dword[F0]
 fld dword[ebp - 8]
 fxch
 fstp dword[ebp - 8]
 mov eax, 2
 mov [ebp - 200], eax
 fild dword[ebp - 200]
 mov eax, 2
 mov [ebp - 200], eax
 fild dword[ebp - 200]
 fmul
 fld dword[ebp - 4]
 fxch
 fistp dword[ebp - 4]
 fld dword[ebp - 4]
 fld dword[ebp - 8]
 fmul
 fld dword[ebp - 8]
 fxch
 fstp dword[ebp - 8]
;---[ Стандартное завершение программы ]---
		mov AX, 4C00h
		int 21h
;===[ Начало сегмента данных ]===
		; ...
		align 16, db 90h
		db '=MY='
		segment .dat
		MYDAT:

 F0: dd 0.45