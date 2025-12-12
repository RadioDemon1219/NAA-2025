#include "pch.h"
#include "Header.h"
#include <locale>
#include <codecvt>
#include <fstream>
#include <iostream>

int In::InWord::size = NULL;

namespace In
{
    IN getin(wchar_t infile[], std::ostream* stream)            // чтение и предобработка текста
    {
        unsigned char* text = new unsigned char[IN_MAX_LEN_TEXT];
        std::ifstream instream(infile, std::ios::binary);
        if (!instream.is_open())
            throw ERROR_THROW(102);
        IN in;
        in.size = 0;
        int pos = 1;
        bool isLiteral = false;
        bool isCharLiteral = false;

        // ѕрочитать первые 3 байта дл€ детекта BOM
        std::streampos startPos = instream.tellg();
        int b1 = instream.get();
        if (b1 == EOF) {
            text[0] = IN_CODE_NULL;
            in.text = text;
            instream.close();
            return in;
        }
        int b2 = instream.get();
        if (b1 == 0xEF && b2 != EOF) {
            int b3 = instream.get();
            if (!(b2 == 0xBB && b3 == 0xBF)) {
                // не UTF-8 BOM Ч вернуть на начало
                instream.clear();
                instream.seekg(startPos);
            }
            // если BOM Ч поток уже после BOM, читаем как UTF-8 дальше
        }
        else if (b1 == 0xFF && b2 == 0xFE) {
            // UTF-16 LE BOM Ч прочитать весь файл как UTF-16 LE и конвертировать в UTF-8
            instream.close();
            std::wifstream win(infile, std::ios::binary);
            if (!win.is_open())
                throw ERROR_THROW(102);
            win.imbue(std::locale(win.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>()));
            std::wstring wcontent((std::istreambuf_iterator<wchar_t>(win)), std::istreambuf_iterator<wchar_t>());
            win.close();
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            std::string utf8 = conv.to_bytes(wcontent);
            size_t len = utf8.size();
            if (len > IN_MAX_LEN_TEXT - 1) len = IN_MAX_LEN_TEXT - 1;
            memcpy(text, utf8.data(), len);
            in.size = static_cast<int>(len);
            text[in.size] = IN_CODE_NULL;
            in.text = text;
            return in;
        }
        else if (b1 == 0xFE && b2 == 0xFF) {
            // UTF-16 BE BOM
            instream.close();
            std::wifstream win(infile, std::ios::binary);
            if (!win.is_open())
                throw ERROR_THROW(102);
            win.imbue(std::locale(win.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, (std::codecvt_mode)std::consume_header>()));
            std::wstring wcontent((std::istreambuf_iterator<wchar_t>(win)), std::istreambuf_iterator<wchar_t>());
            win.close();
            std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
            std::string utf8 = conv.to_bytes(wcontent);
            size_t len = utf8.size();
            if (len > IN_MAX_LEN_TEXT - 1) len = IN_MAX_LEN_TEXT - 1;
            memcpy(text, utf8.data(), len);
            in.size = static_cast<int>(len);
            text[in.size] = IN_CODE_NULL;
            in.text = text;
            return in;
        }
        else {
            // не BOM Ч вернуть позицию на начало
            instream.clear();
            instream.seekg(startPos);
        }

        // „тение по байтам (предполагаем UTF-8/ASCII)
        while (true)
        {
            int gi = instream.get();
            if (gi == EOF)
                break;
            unsigned char uch = static_cast<unsigned char>(gi);

            // »гнорировать CR в CRLF (Windows)
            if (uch == '\r')
                continue;

            if (in.code[uch] == IN::Q) {
                isLiteral = !isLiteral;
            }
            else if (in.code[uch] == IN::C) {
                isCharLiteral = !isCharLiteral;
            }
            switch (in.code[uch])
            {
            case IN::N: // нова€ строка 
                text[in.size++] = uch;
                in.lines++;
                pos = -1;
                break;
            case IN::T:
            case IN::P:
            case IN::S:
            case IN::Q:
            case IN::C:
                text[in.size++] = uch;
                break;
            case IN::F: // недопустимый символ
                // ≈сли внутри строкового/символьного литерала Ч допускаем байты (UTF-8)
                if (isLiteral || isCharLiteral) {
                    text[in.size++] = uch;
                    break;
                }
                Log::writeError(stream, Error::GetError(200, in.lines, pos));
                instream.close();
                delete[] text;
                in.text = nullptr;
                throw ERROR_THROW(200);
                break;
            case IN::I:// игнорируемое
                in.ignor++;
                break;
            default:
                text[in.size++] = in.code[uch];
            }
            pos++;
            if (in.size >= IN_MAX_LEN_TEXT - 1) break;
        }
        text[in.size] = IN_CODE_NULL;
        in.text = text;
        instream.close();
        return in;
    }
	void addWord(InWord* words, char* word, int line)
	{
		if (*word == IN_CODE_NULL)
			return;
		words[InWord::size].line = line;
		strcpy_s(words[InWord::size].word, strlen(word) + 1, word);
		InWord::size++;
	}
	InWord* getWordsTable(std::ostream* stream, unsigned char* text, int* code, int textSize)
	{
		InWord* words = new InWord[MAXSIZE_LT];
		int bufpos = 0;
		int line = 1;
		char buffer[MAX_LEN_BUFFER] = "";
		for (int i = 0; i < textSize; i++)
		{
			switch (code[text[i]])
			{
			case IN::S:
			{
				if (text[i] == LEX_MINUS && isdigit(text[i + 1])) // минус как часть числа
				{
					buffer[bufpos++] = static_cast<char>(text[i]);
					buffer[bufpos] = IN_CODE_NULL;
					break;
				}
				char letter[] = { static_cast<char>(text[i]), IN_CODE_NULL };
				addWord(words, buffer, line);
				addWord(words, letter, line);
				*buffer = IN_CODE_NULL;
				bufpos = 0;
				break;
			}
			case IN::N:
			case IN::P:
				addWord(words, buffer, line);
				*buffer = IN_CODE_NULL;
				bufpos = 0;
				if (code[text[i]] == IN::N)
					line++;
				break;
			case IN::C:
			{
				if (text[i] == '\'' && text[i + 2] == '\'')
				{
					char charLiteral[] = { '\'', static_cast<char>(text[i + 1]), '\'', '\0' };
					addWord(words, charLiteral, line);
					i += 2;
				}

				else
				{
					char letter[] = { static_cast<char>(text[i]), IN_CODE_NULL };
					addWord(words, buffer, line);
					addWord(words, letter, line);
					*buffer = IN_CODE_NULL;
					bufpos = 0;
				}
				break;
			}

			case IN::Q:
			{
				addWord(words, buffer, line);
				*buffer = IN_CODE_NULL;
				bufpos = 0;
				bool isltrlgood = true;
				for (int j = i + 1; j < textSize && text[j] != IN_CODE_QUOTE; j++)
				{
					if (code[text[j]] == IN::N)
					{
						Log::writeError(stream, Error::GetError(311, line, 0));
						isltrlgood = false;
						break;
					}
				}
				if (isltrlgood)
				{
					buffer[bufpos++] = IN_CODE_QUOTE;
					for (int j = 1; ; j++)
					{
						if (j >= 256 || i + j == textSize)
						{
							Log::writeError(stream, Error::GetError(312, line, 0));
							break;
						}
						buffer[bufpos++] = static_cast<char>(text[i + j]);
						if (text[i + j] == IN_CODE_QUOTE)
						{
							buffer[bufpos] = IN_CODE_NULL;
							addWord(words, buffer, line);
							i = i + j;
							break;
						}
					}
				}
				*buffer = IN_CODE_NULL;  bufpos = 0;
				break;
			}
			default:
				buffer[bufpos++] = static_cast<char>(text[i]);
				buffer[bufpos] = IN_CODE_NULL;
			}
		}
		return words;
	}

	void printTable(InWord* table)
	{
		std::cout << " ------------------ —лова входа: ------------------" << std::endl;
		for (int i = 0; i < table->size; i++)
			std::cout << std::setw(2) << i << std::setw(3) << table[i].line << " |  " << table[i].word << std::endl;
	}
};