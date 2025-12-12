#pragma once
#include "LT.h"
#include "In.h"
#include "Log.h"
#include "FST.h"
#include "Parm.h"

// Простые строковые константы для типов и ключевых слов
#define TYPE_CHAR       "char"
#define TYPE_INT        "integer"
#define TYPE_STRING     "string"
#define TYPE_PROCEDURE  "procedure"
#define FUNC            "func"
#define DECL            "decl"
#define OUTPUT          "output"
#define INPUT           "input"
#define MAIN            "main"
#define SWITCH          "switch"
#define CASE            "case"
#define BREAK           "break"
#define DEFAULT         "default"

// Вспомогательные названия встроенных функций (если проект ожидает такие имена)
#define CONCAT          "concat"
#define LENGHT          "lenght"
#define ATOII           "atoii"

#define ISTYPE(str) ( !strcmp(str, TYPE_CHAR) || !strcmp(str, TYPE_INT) || !strcmp(str, TYPE_STRING) )

namespace Lexer
{
	struct LEX
	{
		LT::LexTable lextable;
		IT::IdTable	idtable;
		LEX() {}
	};

#ifndef LEXEMA
#define LEXEMA(i) (tables.lextable.table[i].lexema)
#endif
#ifndef ITENTRY
#define ITENTRY(i) (tables.idtable.table[tables.lextable.table[i].idxTI])
#endif

	struct Graph
	{
		char lexema;
		// Хранится указатель на FST::FST (FST может быть некопируемым)
		FST::FST* graph;
		Graph() : lexema(0), graph(nullptr) {}
		// Конструктор для инициализации из списка {lex, new FST::FST(...)}
		Graph(char l, FST::FST* g) : lexema(l), graph(g) {}
	};

	IT::Entry* getEntry		// формирует и возвращает запись в ТИ
	(
		Lexer::LEX& tables,						// ТЛ + ТИ
		char lex,								// лексема
		char* id,								// идентификатор
		char* idtype,							// предыдущая (тип)
		bool isParam,							// признак параметра функции
		bool isFunc,							// признак функции
		Log::LOG log,							// протокол
		int line,								// строка в исходном тексте
		bool& rc_err							// флаг ошибки(по ссылке)
	);

	struct ERROR_S									// тип исключения для throw ERROR_THROW | ERROR_THROW_IN
	{
		int id;
		char message[ERROR_MAXSIZE_MESSAGE];
		struct
		{
			short line = -1;
			short col = -1;
		} position;
	};
	bool analyze(LEX& tables, In::IN& in, Log::LOG& log, Parm::PARM& parm);
	int	getIndexInLT(LT::LexTable& lextable, int itTableIndex);					// индекс первой встречи в таблице лексем
};