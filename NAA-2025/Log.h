#pragma once
#include "Parm.h"
#include "Error.h"
#include "In.h"
#include "LT.h"
#include "IT.h"
#include <iomanip>
#include <iostream>
#include <fstream>

namespace Log
{
	struct LOG
	{
		wchar_t logfile[PARM_MAX_SIZE];
		std::ofstream* stream;
		LOG() : stream(nullptr)
		{
			wcscpy_s(logfile, PARM_MAX_SIZE, L"");
		}
	};

	LOG getstream(wchar_t logfile[]);
	void writeLog(LOG& log);									//запись в журнал
	void writeLine(std::ostream* stream, char* c, ...);			//запись сообщения в поток
	void writeParm(LOG& log, Parm::PARM& parm);					//запись параметров в журнал
	void writeIn(std::ostream* stream, In::IN& in);				//запись входного кода
	void writeWords(std::ostream* stream, In::InWord* words);	//запись слов в поток
	void writeError(std::ostream* stream, Error::ERROR error);	//запись ошибки в поток
	void Close(LOG& log);										//закрыть журнал
}