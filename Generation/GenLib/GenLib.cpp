#include <iostream>
#include <string>
#include <windows.h>

//       
void init_console()
{
	SetConsoleCP(1251);              //: Windows-1251
	SetConsoleOutputCP(1251);        // : Windows-1251
}

extern "C"
{
	// -----------------------------------------------------------
	// 
	// -----------------------------------------------------------

	//    ( 32- ,   ASM  push eax)
	int __stdcall outnum(int value)
	{
		static bool initialized = false;
		if (!initialized)
		{
			init_console();
			initialized = true;
		}

		std::cout << value;
		return 0;
	}

	// 
	int __stdcall outchar(char ch)
	{
		static bool initialized = false;
		if (!initialized)
		{
			init_console();
			initialized = true;
		}

		std::cout << ch;
		return 0;
	}

	//  
	int __stdcall outstr(char* ptr)
	{
		static bool initialized = false;
		if (!initialized)
		{
			init_console();
			initialized = true;
		}

		if (ptr == nullptr)
		{
			//  /null,    ()
			return 0;
		}

		std::cout << ptr;
		return 0;
	}

	//   (чтение строки из консоли)
	int __stdcall inputStr(char* buffer)
	{
		static bool initialized = false;
		if (!initialized)
		{
			init_console();
			initialized = true;
		}

		if (buffer == nullptr)
		{
			return 0;
		}

		std::string input;
		std::getline(std::cin, input);
		
		// Копируем введенную строку в буфер
		size_t len = input.length();
		if (len > 255) len = 255; // Ограничение размера буфера
		for (size_t i = 0; i < len; i++)
		{
			buffer[i] = input[i];
		}
		buffer[len] = '\0';
		
		return 0;
	}

	// -----------------------------------------------------------
	//   (NAA-2025: C++23 Req)
	// -----------------------------------------------------------

	/*
		 strSize
		   (  \0)
	*/
	int __stdcall strSize(char* str)
	{
		if (str == nullptr) return 0;

		int len = 0;
		while (str[len] != '\0')
		{
			len++;
		}
		return len;
	}

	/*
		 strCompare
		  .
		:
		 0,  
		 1, str1 > str2
		-1, str1 < str2
	*/
	int __stdcall strCompare(char* str1, char* str2)
	{
		if (str1 == nullptr && str2 == nullptr) return 0;
		if (str1 == nullptr) return -1;
		if (str2 == nullptr) return 1;

		int i = 0;
		while (str1[i] != '\0' && str2[i] != '\0')
		{
			if (str1[i] > str2[i]) return 1;
			if (str1[i] < str2[i]) return -1;
			i++;
		}

		//    
		if (str1[i] == '\0' && str2[i] == '\0') return 0; // 
		if (str1[i] == '\0') return -1; //  
		return 1; //  
	}

	// stoi - преобразование строки в целое число
	int __stdcall stoi(char* str)
	{
		if (str == nullptr) return 0;

		int result = 0;
		int sign = 1;
		int i = 0;

		// Пропускаем пробелы
		while (str[i] == ' ' || str[i] == '\t') i++;

		// Проверяем знак
		if (str[i] == '-')
		{
			sign = -1;
			i++;
		}
		else if (str[i] == '+')
		{
			i++;
		}

		// Преобразуем цифры
		while (str[i] >= '0' && str[i] <= '9')
		{
			result = result * 10 + (str[i] - '0');
			i++;
			
			// Проверка на переполнение (для 16-битного числа)
			if (result > 32767) break;
		}

		return result * sign;
	}
}
