#include "LexAnaliz.h"
#include "Graphs.h"
#include <stack>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

// Фallback: на случай, если какие-то графы по каким-то причинам не были определены в Graphs.h
#ifndef GRAPH_MAIN
#define GRAPH_MAIN 4, \
    FST::NODE(1, FST::RELATION('m', 1)), \
    FST::NODE(1, FST::RELATION('a', 2)), \
    FST::NODE(1, FST::RELATION('i', 3)), \
    FST::NODE(1, FST::RELATION('n', 4)), \
    FST::NODE()
#endif

#ifndef GRAPH_CHAR
#define GRAPH_CHAR 4, \
    FST::NODE(1, FST::RELATION('c', 1)), \
    FST::NODE(1, FST::RELATION('h', 2)), \
    FST::NODE(1, FST::RELATION('a', 3)), \
    FST::NODE(1, FST::RELATION('r', 4)), \
    FST::NODE()
#endif

#ifndef GRAPH_STRING
#define GRAPH_STRING 6, \
    FST::NODE(1, FST::RELATION('s', 1)), \
    FST::NODE(1, FST::RELATION('t', 2)), \
    FST::NODE(1, FST::RELATION('r', 3)), \
    FST::NODE(1, FST::RELATION('i', 4)), \
    FST::NODE(1, FST::RELATION('n', 5)), \
    FST::NODE(1, FST::RELATION('g', 6)), \
    FST::NODE()
#endif

#ifndef GRAPH_INT
#ifdef GRAPH_INTEGER
#define GRAPH_INT GRAPH_INTEGER
#else
#define GRAPH_INT GRAPH_INTEGER
#endif
#endif

// Флаг для восьмеричного литерала (вспомогательная переменная)
static bool is_octal = false;

int DecimicalNotation(std::string input, int scaleofnot) {
	std::string num = input.substr(2);
	if (input[0] == '0') {
		return std::stoi(num, nullptr, scaleofnot);
	}
	else {
		return std::stoi(num, nullptr, scaleofnot) * (-1);
	}
}
namespace Lexer
{
	// Графы: используем имена лексем/графов из заголовков
	// Изменено: передаём в Graph указатели на динамически выделенные FST::FST
	Graph graphs[N_GRAPHS] =
	{
		{ LEX_SEPARATORS,  new FST::FST(GRAPH_SEPARATORS) },
		{ LEX_LITERAL,     new FST::FST(GRAPH_INT_LITERAL) },
		{ LEX_LITERAL,     new FST::FST(GRAPH_CHAR_LITERAL) },
		{ LEX_LITERAL,     new FST::FST(GRAPH_STRING_LITERAL) },
		{ LEX_LITERAL_OCT, new FST::FST(GRAPH_V_LITERAL) }, // восьмеричный литерал (0o...)
		{ LEX_DECL,        new FST::FST(GRAPH_DECL) },
		{ LEX_MAIN,        new FST::FST(GRAPH_MAIN) },
		{ LEX_ID_TYPE,     new FST::FST(GRAPH_CHAR) },
		{ LEX_ID_TYPE,     new FST::FST(GRAPH_INT) },
		{ LEX_ID_TYPE,     new FST::FST(GRAPH_STRING) },
		{ LEX_FUNC,        new FST::FST(GRAPH_FUNC) },
		{ LEX_RETURN,      new FST::FST(GRAPH_RETURN) },
		{ LEX_OUTPUT,      new FST::FST(GRAPH_OUTPUT) },
		{ LEX_INPUT,       new FST::FST(GRAPH_INPUT) },
		{ LEX_SWITCH,      new FST::FST(GRAPH_SWITCH) },
		{ LEX_CASE,        new FST::FST(GRAPH_CASE) },
		{ LEX_DEFAULT,     new FST::FST(GRAPH_DEFAULT) },
		{ LEX_BREAK,       new FST::FST(GRAPH_BREAK) },
		{ LEX_NEWLINE,     new FST::FST(GRAPH_NEWLINE) },
		{ LEX_ID,          new FST::FST(GRAPH_ID) },
		{ LEX_EXP,         new FST::FST(GRAPH_POWER) },
		{ LEX_SHARP,       new FST::FST(GRAPH_TRANSFORM) },
	};

	static const std::vector<std::string> FUNCTION_TYPES = { TYPE_CHAR, TYPE_INT, TYPE_STRING };

	char* getScopeName(IT::IdTable idtable, char* prevword)
	{
		char* a = new char[5];
		a[0] = 'm'; a[1] = 'a'; a[2] = 'i'; a[3] = 'n'; a[4] = '\0';
		if (strcmp(prevword, MAIN) == 0)
			return a;

		for (int i = idtable.size - 1; i >= 0; i--)
			if (idtable.table[i].idtype == IT::IDTYPE::F)
				return idtable.table[i].id;
		return nullptr;
	}

	int getLiteralIndex(IT::IdTable ittable, char* value, IT::IDDATATYPE type)
	{
		for (int i = 0; i < ittable.size; i++)
		{
			if (ittable.table[i].idtype == IT::IDTYPE::L && ittable.table[i].iddatatype == type)
			{
				switch (type)
				{
				case IT::IDDATATYPE::INT:
					if (is_octal)
					{
						int oct_val = DecimicalNotation(value, 8);
						if (ittable.table[i].value.vint == oct_val)
							return i;
					}
					else if (ittable.table[i].value.vint == atoi(value)) {
						return i;
					}
					break;
				case IT::IDDATATYPE::STR:
				{
					char buf[STR_MAXSIZE];
					size_t vlen = strlen(value);
					if (vlen >= 2) {
						size_t copylen = vlen - 2;
						if (copylen >= sizeof(buf))
							copylen = sizeof(buf) - 1;
						if (copylen > 0)
							memcpy(buf, value + 1, copylen);
						buf[copylen] = '\0';
					}
					else {
						buf[0] = '\0';
					}
					if (strcmp(ittable.table[i].value.vstr.str, buf) == 0)
						return i;
					break;
				}
				case IT::IDDATATYPE::CHR:
					if (ittable.table[i].value.vchar == value[1])
						return i;
					break;
				}
			}
		}
		return NULLIDX_TI;
	}

	IT::IDDATATYPE getType(char* curword, char* idtype)
	{
		if (idtype != nullptr) {
			if (!strcmp(TYPE_PROCEDURE, idtype))
				return IT::IDDATATYPE::PROC;
			if (!strcmp(TYPE_CHAR, idtype))
				return IT::IDDATATYPE::CHR;
			if (!strcmp(TYPE_STRING, idtype))
				return IT::IDDATATYPE::STR;
			if (!strcmp(TYPE_INT, idtype))
				return IT::IDDATATYPE::INT;
		}

		if (curword != nullptr && (isdigit((unsigned char)*curword) || *curword == LEX_MINUS))
		{
			if ((curword[0] == '0' && curword[1] == 'o') || (curword[0] == '0' && curword[1] != '\0'))
				is_octal = true;
			else
				is_octal = false;
			return IT::IDDATATYPE::INT;
		}
		else if (curword != nullptr && *curword == IN_CODE_QUOTE)
			return IT::IDDATATYPE::STR;
		else if (curword != nullptr && *curword == IN_CODE_QUOTE2 && strlen(curword) == 3)
			return IT::IDDATATYPE::CHR;

		return IT::IDDATATYPE::UNDEF;
	}

	int getIndexInLT(LT::LexTable& lextable, int itTableIndex)
	{
		if (itTableIndex == NULLIDX_TI)
			return lextable.size;
		for (int i = 0; i < lextable.size; i++)
			if (itTableIndex == lextable.table[i].idxTI)
				return i;
		return NULLIDX_TI;
	}

	bool isLiteral(char* id)
	{
		if (id == nullptr) return false;
		if (isdigit((unsigned char)*id) || *id == IN_CODE_QUOTE || *id == LEX_MINUS || *id == IN_CODE_QUOTE2)
			return true;
		return false;
	}

	// Расширено: поддержка алиасов "int"/"len" -> встроенные функции ATOII/LENGHT
	IT::STDFNC getStandFunction(char* id)
	{
		if (id == nullptr) return IT::STDFNC::F_NOT_STD;
		if (!strcmp(CONCAT, id))
			return IT::STDFNC::F_CONCAT;
		if (!strcmp(LENGHT, id) || !strcmp("len", id))
			return IT::STDFNC::F_LENGHT;
		if (!strcmp(ATOII, id) || !strcmp("int", id))
			return IT::STDFNC::F_ATOII;
		return IT::STDFNC::F_NOT_STD;
	}

	char* getNextLiteralName()
	{
		static int literalNumber = 1;
		char* buf = new char[SCOPED_ID_MAXSIZE];
		char num[12];
		strcpy_s(buf, SCOPED_ID_MAXSIZE, "LTRL");
		_itoa_s(literalNumber++, num, sizeof(num), 10);
		strcat_s(buf, SCOPED_ID_MAXSIZE, num);
		return buf;
	}

	IT::Entry* getEntry(
		Lexer::LEX& tables,
		char lex,
		char* id,
		char* idtype,
		bool isParam,
		bool isFunc,
		Log::LOG log,
		int line,
		bool& lex_ok)
	{
		IT::IDDATATYPE type = getType(id, idtype);
		int index = IT::isId(tables.idtable, id);

		if (lex == LEX_LITERAL || lex == LEX_LITERAL_OCT)
			index = getLiteralIndex(tables.idtable, id, type);

		if (index != NULLIDX_TI)
			return nullptr;

		IT::Entry* itentry = new IT::Entry;
		memset(itentry, 0, sizeof(IT::Entry));
		itentry->iddatatype = type;
		itentry->idxfirstLE = getIndexInLT(tables.lextable, index);

		if (lex == LEX_LITERAL || lex == LEX_LITERAL_OCT)
		{
			bool int_ok = true;

			if (type == IT::IDDATATYPE::INT)
			{
				if (lex == LEX_LITERAL_OCT)
				{
					itentry->value.vint = DecimicalNotation(id, 8);
				}
				else
				{
					itentry->value.vint = atoi(id);
				}

				if (itentry->value.vint < NUM_MINSIZE || itentry->value.vint > NUM_MAXSIZE)
					int_ok = false;
			}
			else
			{
				int_ok = IT::SetValue(itentry, id);
			}

			if (!int_ok)
			{
				Log::writeError(log.stream, Error::GetError(313, line, 0));
				lex_ok = false;
			}

			if (itentry->iddatatype == IT::IDDATATYPE::STR && itentry->value.vstr.len == 0)
			{
				Log::writeError(log.stream, Error::GetError(310, line, 0));
				lex_ok = false;
			}
			if (itentry->iddatatype == IT::IDDATATYPE::CHR && itentry->value.vchar == '\0')
			{
				Log::writeError(log.stream, Error::GetError(311, line, 0));
				lex_ok = false;
			}

			strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, getNextLiteralName());
			itentry->idtype = IT::IDTYPE::L;
		}
		else
		{
			// Установить дефолтные значения для типов
			switch (type)
			{
			case IT::IDDATATYPE::STR:
				strcpy_s(itentry->value.vstr.str, STR_MAXSIZE, "");
				itentry->value.vstr.len = STR_DEFAULT;
				break;
			case IT::IDDATATYPE::INT:
				itentry->value.vint = NUM_DEFAULT;
				break;
			case IT::IDDATATYPE::CHR:
				itentry->value.vchar = STR_DEFAULT;
				break;
			case IT::IDDATATYPE::PROC:
				itentry->value.params.count = 0;
				itentry->value.params.types = nullptr;
				break;
			}

			// Расширено: если идентификатор соответствует стандартной встроенной функции — зарегистрировать как LIB FUNC
			IT::STDFNC sf = getStandFunction(id);
			if (sf != IT::STDFNC::F_NOT_STD)
			{
				switch (sf)
				{
				case IT::STDFNC::F_CONCAT:
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = CONCAT_TYPE;
					itentry->value.params.count = CONCAT_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[CONCAT_PARAMS_CNT];
					for (int k = 0; k < CONCAT_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::CONCAT_PARAMS[k];
					strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, CONCAT);
					break;
				case IT::STDFNC::F_LENGHT:
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = LENGHT_TYPE;
					itentry->value.params.count = LENGHT_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[LENGHT_PARAMS_CNT];
					for (int k = 0; k < LENGHT_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::LENGHT_PARAMS[k];
					strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, LENGHT);
					break;
				case IT::STDFNC::F_ATOII:
					itentry->idtype = IT::IDTYPE::S;
					itentry->iddatatype = ATOII_TYPE;
					itentry->value.params.count = ATOII_PARAMS_CNT;
					itentry->value.params.types = new IT::IDDATATYPE[ATOII_PARAMS_CNT];
					for (int k = 0; k < ATOII_PARAMS_CNT; k++)
						itentry->value.params.types[k] = IT::ATOII_PARAMS[k];
					strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, ATOII);
					break;
				default:
					itentry->idtype = IT::IDTYPE::F;
					strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, id);
					break;
				}
			}
			else
			{
				// Обычное поведение: функция/параметр/переменная
				if (isFunc)
				{
					itentry->idtype = IT::IDTYPE::F;
					strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, id);
				}
				else if (isParam)
					itentry->idtype = IT::IDTYPE::P;
				else
					itentry->idtype = IT::IDTYPE::V;

				// записать исходное имя, если ещё не записано
				if (itentry->id[0] == '\0')
					strcpy_s(itentry->id, SCOPED_ID_MAXSIZE, id);
			}
		}

		if (itentry->iddatatype == IT::IDDATATYPE::UNDEF)
		{
			Log::writeError(log.stream, Error::GetError(300, line, 0));
			lex_ok = false;
		}
		return itentry;
	}

	bool analyze(LEX& tables, In::IN& in, Log::LOG& log, Parm::PARM& parm)
	{
		static bool lex_ok = true;
		tables.lextable = LT::Create(MAXSIZE_LT);
		tables.idtable = IT::Create(MAXSIZE_TI);

		bool isParam = false, isFunc = false;
		int enterPoint = 0;
		char curword[STR_MAXSIZE], nextword[STR_MAXSIZE];
		int curline;
		std::stack <char*> scopes;

		for (int i = 0; i < in.size; i++)
		{
			is_octal = false;

			strcpy_s(curword, STR_MAXSIZE, in.words[i].word);
			if (i < in.size - 1)
			{
				if (strlen(in.words[i + 1].word) <= STR_MAXSIZE)
					strcpy_s(nextword, STR_MAXSIZE, in.words[i + 1].word);
				else
				{
					lex_ok = false;
					Log::writeError(log.stream, Error::GetError(319, curline, 0));
					break;
				}
			}
			else
			{
				nextword[0] = '\0';
			}
			curline = in.words[i].line;

			isFunc = false;
			int idxTI = NULLIDX_TI;

			for (int j = 0; j < N_GRAPHS; j++)
			{
				// Используем разыменованный граф (graphs[j].graph — указатель)
				FST::FST fst(curword, *graphs[j].graph);
				if (FST::execute(fst))
				{
					char lexema = graphs[j].lexema;
					switch (lexema)
					{
					case LEX_MAIN:
						enterPoint++;
						break;
					case LEX_SEPARATORS:
					{
						switch (*curword)
						{
						case LEX_LEFTHESIS:
						{
							isParam = true;
							if (i > 0 && (strcmp(in.words[i - 1].word, FUNC) == 0 || strcmp(in.words[i - 1].word, MAIN) == 0))
							{
								char* functionname = new char[MAXSIZE_ID];
								char* scopename = getScopeName(tables.idtable, in.words[i - 1].word);
								if (scopename != nullptr)
								{
									strcpy_s(functionname, MAXSIZE_ID, scopename);
									scopes.push(functionname);
								}
							}
							break;
						}
						case LEX_RIGHTTHESIS:
						{
							isParam = false;
							break;
						}
						case LEX_LEFTBRACE:
						{
							if (i > 0 && !ISTYPE(in.words[i - 1].word) && strcmp(in.words[i - 1].word, FUNC) != 0 && strcmp(in.words[i - 1].word, MAIN) != 0)
							{
								if (scopes.empty())
								{
									char* functionname = new char[MAXSIZE_ID];
									char* scopename = getScopeName(tables.idtable, in.words[i - 1].word);
									if (scopename != nullptr)
									{
										strcpy_s(functionname, MAXSIZE_ID, scopename);
										scopes.push(functionname);
									}
								}
							}
							break;
						}
						case LEX_RIGHTBRACE:
						{
							if (!scopes.empty())
								scopes.pop();
							break;
						}
						case LEX_LEFTBRACKET:
						{
							break;
						}
						case LEX_BRACKET:
						{
							break;
						}
						}
						lexema = *curword;
						break;
					}
					case LEX_LITERAL_OCT:
					{
						is_octal = true;
						lexema = LEX_LITERAL;
					}
					case LEX_ID:
					case LEX_LITERAL:
					{
						char id[SCOPED_ID_MAXSIZE] = "";
						idxTI = NULLIDX_TI;

						if (*nextword == LEX_LEFTHESIS)
							isFunc = true;

						// раньше: жесткая ошибка при встрече идентификатора вне области
						// теперь: разрешаем LHS присваивания и объявления (DECL) без ошибки (автоматическая регистрация)
						if (scopes.empty() && !isFunc && lexema == LEX_ID) {
							bool isAssignmentLHS = (nextword[0] == LEX_EQUAL);
							bool isDeclaration = (i > 0 && strcmp(in.words[i - 1].word, DECL) == 0);
							if (!isAssignmentLHS && !isDeclaration) {
								Log::writeError(log.stream, Error::GetError(616, curline, 0));
								lex_ok = false;
								return lex_ok;
							}
							// если LHS присваивания — пропустим ошибку: getEntry добавит переменную при необходимости
						}

						char* idtype = (isFunc && i > 1 && strcmp(in.words[i - 1].word, FUNC) == 0) ?
							in.words[i - 2].word : in.words[i - 1].word;

						if (strlen(curword) > MAXSIZE_ID && lexema == LEX_ID) {
							Log::writeError(log.stream, Error::GetError(204, curline, 0));
							lex_ok = false;
							return lex_ok;
						}

						if (!isFunc && !scopes.empty()) {
							strcpy_s(id, SCOPED_ID_MAXSIZE, scopes.top());
							// безопасная конкатенация
							strcat_s(id, SCOPED_ID_MAXSIZE, ".");
						}
						strcat_s(id, SCOPED_ID_MAXSIZE, curword);

						if (isLiteral(curword))
							strcpy_s(id, SCOPED_ID_MAXSIZE, curword);

						IT::Entry* itentry = getEntry(tables, lexema, id, idtype, isParam, isFunc, log, curline, lex_ok);
						if (itentry != nullptr)
						{
							if (isFunc)
							{
								if (getStandFunction(curword) == IT::STDFNC::F_NOT_STD)
								{
									// обработка параметров при необходимости
								}
							}
							IT::Add(tables.idtable, *itentry);
							idxTI = tables.idtable.size - 1;
						}
						else
						{
							int last_lt_index = tables.lextable.size - 1;
							if (last_lt_index >= 0)
							{
								char last_lexema = tables.lextable.table[last_lt_index].lexema;
								if (last_lexema == LEX_DECL || last_lexema == LEX_FUNC || last_lexema == LEX_ID_TYPE)
								{
									Log::writeError(log.stream, Error::GetError(305, curline, 0));
									lex_ok = false;
								}
							}

							idxTI = IT::isId(tables.idtable, id);
							if (lexema == LEX_LITERAL)
							{
								IT::IDDATATYPE lit_type = getType(curword, in.words[i - 1].word);
								idxTI = getLiteralIndex(tables.idtable, curword, lit_type);
							}

							if (idxTI == NULLIDX_TI && lexema == LEX_ID)
							{
								Log::writeError(log.stream, Error::GetError(308, curline, 0));
								lex_ok = false;
							}
						}
					}
					break;
					}

					LT::Entry ltentry(lexema, curline, idxTI);

					// Поддержка: считать '|' как разделитель параметров/объявлений — сопоставляем его с запятой
					if (ltentry.lexema == LEX_PIPE)
						ltentry.lexema = LEX_COMMA;

					// Принять перевод строки как разделитель операторов (позволяет опустить ';' в отдельных строках)
					if (ltentry.lexema == LEX_NEWLINE)
						ltentry.lexema = LEX_SEPARATOR;
					LT::Add(tables.lextable, ltentry);
					break;
				}
				else if (j == N_GRAPHS - 1)
				{
					Log::writeError(log.stream, Error::GetError(201, curline, 0));
					lex_ok = false;
				}
			}

			if (!lex_ok) break;
		}

		if (enterPoint == 0)
		{
			Log::writeError(log.stream, Error::GetError(301));
			lex_ok = false;
		}
		if (enterPoint > 1)
		{
			Log::writeError(log.stream, Error::GetError(302));
			lex_ok = false;
		}
		return lex_ok;
	}
};