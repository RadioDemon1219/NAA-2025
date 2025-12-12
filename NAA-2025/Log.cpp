#include "pch.h"
#include "Header.h"
#include <iostream>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <sstream> // добавлено: требуется для std::ostringstream

#define W(x, y)  << std::setw(x) << (y) <<
#define STR(n, line, type, id)\
	"|" W(4,n) " |  " W(5,line) "    |" W(17,type) " |  " W(SCOPED_ID_MAXSIZE, id) " |"


namespace Log
{
	LOG getstream(wchar_t  logfile[])
	{
		LOG stream;
		stream.stream = new std::ofstream;
		stream.stream->open(logfile);
		if (!stream.stream->is_open())
			throw ERROR_THROW(103);
		wcscpy_s(stream.logfile, logfile);
		return stream;
	}

	void writeLog(LOG& log)
	{
		char buffer[80];
		time_t seconds = time(NULL);
		tm* timeinfo = localtime(&seconds);
		const char* format = "%d.%m.%Y %H:%M:%S";
		strftime(buffer, 80, format, timeinfo);
		*log.stream << "\n----------- Журнал ------------ время: " << buffer << " ------------ \n\n";
	}

	void writeLine(std::ostream* stream, char* c, ...)		// запись в поток форматированная
	{
		char** ptr = &c;
		char* result;
		result = (char*)malloc(15);
		size_t size = 0;

		while (strcmp(*ptr, "") != 0)
		{
			size_t slen = strlen(*ptr);
			result = (char*)realloc(result, size + slen + 1);
			result[size] = '\0';
			strcat_s(result, size + slen + 1, *ptr);
			size += slen;
			ptr++;
		}
		*stream << result << std::endl;
		free(result);
	}

	void writeParm(LOG& log, Parm::PARM& parm)
	{
		char inTxt[PARM_MAX_SIZE],
			outTxt[PARM_MAX_SIZE],
			logTxt[PARM_MAX_SIZE];
		wcstombs(inTxt, parm.in, wcslen(parm.in) + 1);
		wcstombs(outTxt, parm.out, wcslen(parm.out) + 1);
		wcstombs(logTxt, parm.log, wcslen(parm.log) + 1);
		*log.stream << "\n----- Параметры --------";
		*log.stream << "\n-in: " << inTxt
			<< "\n-out: " << outTxt
			<< "\n-log: " << logTxt;
	}

	void writeIn(std::ostream* stream, In::IN& in)
	{
		if (stream == nullptr) return;
		*stream << "\n---- Входной файл ------\n";
		*stream << "Всего слов: " << std::setw(3) << in.size << "\n";
		*stream << "Игнорируемых: " << std::setw(3) << in.ignor << "\n";
		*stream << "Количество строк: " << std::setw(3) << in.lines << "\n\n";
	}

	void writeWords(std::ostream* stream, In::InWord* words)
	{
		if (stream == nullptr) return;
		*stream << " ------------------ Слова входа: ------------------" << std::endl;
		for (int i = 0; i < words->size; i++)
			*stream << std::setw(2) << i << std::setw(3) << words[i].line << " |  " << words[i].word << std::endl;
		*stream << "\n-------------------------------------------------------------------------\n\n";
	}

	void writeError(std::ostream* stream, Error::ERROR e)
	{
		// Формирование текста ошибки
		std::ostringstream oss;
		if (e.position.line == -1 && e.position.col == -1)
		{
			oss << std::endl << "Ошибка N" << e.id << ": " << e.message;
		}
		else if (e.position.col == -1)
		{
			oss << std::endl << "Ошибка N" << e.id << ": " << e.message
				<< " строка: " << e.position.line;
		}
		else
		{
			oss << std::endl << "Ошибка N" << e.id << ": " << e.message
				<< " строка: " << e.position.line
				<< " столбец: " << e.position.col;
		}

		// Запись и на экран, и в поток журнала (если задан)
		if (stream == nullptr)
		{
			std::cout << oss.str() << std::endl;
		}
		else
		{
			(*stream) << oss.str() << std::endl;
			std::cout << oss.str() << std::endl;
		}
	}

	void Close(LOG& log)
	{
		if (log.stream != nullptr)
			(*log.stream).close();
	}
};