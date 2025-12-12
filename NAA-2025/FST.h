#pragma once

#include "IT.h"

namespace FST
{
	struct RELATION		// отношение: символ -> индекс состояния
	{
		char  symbol;	// символ перехода
		short nnode;	// номер целевого узла
		// добавлены значения по умолчанию, чтобы RELATION имел конструктор по умолчанию
		RELATION(
			char c = 0,		// символ перехода (по умолчанию '\0')
			short ns = 0	// номер состояния (по умолчанию 0)
		);
	};

	struct NODE					//узел конечного автомата
	{
		short n_relation;		//число отношений/переходов
		RELATION* relations;	//массив отношений
		NODE();					//конструктор по умолчанию
		NODE(short n, RELATION rel, ...);  //создать узел с N отношениями, первый RELATION и далее varargs
	};

	struct FST   //описание конечного автомата
	{
		char* string;				//строка (nul-terminated)
		short position;				//позиция в строке
		short nstates;				//число состояний
		NODE* node;					//массив узлов
		short* rstates;				//временное состояние выполнения
		FST(short ns, NODE n, ...); //конструктор автомата по описанию
		FST(char* s, FST& fst);		//конструктор экземпляра автомата для строки s на основе описания fst
	};

	bool execute(FST& fst); //выполнить автомата для fst
};