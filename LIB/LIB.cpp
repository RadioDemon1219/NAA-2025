#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>

extern "C"
{
	// Функции работы со строками
	int strToInt(const char* str)
	{
		if (str == nullptr || *str == '\0')
			return 0;

		try {
			return std::stoi(str);
		}
		catch (...) {
			return 0; // В случае ошибки возвращаем 0
		}
	}

	int strLength(const char* str)
	{
		if (str == nullptr)
			return 0;

		int length = 0;
		while (str[length] != '\0')
		{
			length++;
		}
		return length;
	}

	// Дополнительные полезные функции
	char* intToStr(int value)
	{
		char* result = (char*)malloc(32); // Достаточно для любого int
		if (result == nullptr)
			return nullptr;

		sprintf_s(result, 32, "%d", value);
		return result;
	}
}