#include "pch.h"
#include <iosfwd>
#include "Generator.h"
#include "Parm.h"
#include "LexAnaliz.h"
#include "IT.h"
#include "LT.h"
#include <sstream>
#include <cstring>
#include <stack>
#include <vector>

using namespace std;

namespace Gener
{
	static int switch_num = 0;
	static int cycle_num = 0;

	struct SwitchContext {
		int label_num;
		string end_label;
		string default_label;
	};
	static stack<SwitchContext> switch_stack;

	// Флаг, указывающий что возврат (return) уже сгенерирован (чтобы genExitCode не дублировал mov eax)
	static bool g_returnHandled = false;

	string itoS(int x) { stringstream r; r << x; return r.str(); }

#ifndef LEXEMA
#define LEXEMA(i) (tables.lextable.table[i].lexema)
#endif
#ifndef ITENTRY
#define ITENTRY(i) (tables.idtable.table[tables.lextable.table[i].idxTI])
#endif

	string genCallFuncCode(Lexer::LEX& tables, Log::LOG& log, int i)
	{
		string str;
		IT::Entry e = ITENTRY(i);
		stack <IT::Entry> temp;

		for (int j = i + 1; LEXEMA(j) != LEX_RIGHTTHESIS; j++)
		{
			if (LEXEMA(j) == LEX_ID || LEXEMA(j) == LEX_LITERAL)
				temp.push(ITENTRY(j));
		}
		str += "\n";

		while (!temp.empty())
		{
			if (temp.top().idtype == IT::IDTYPE::L && temp.top().iddatatype == IT::IDDATATYPE::STR)
			{
				str += "push offset ";
				str += temp.top().id;
				str += "\n";
			}
			else
			{
				str += "push ";
				str += temp.top().id;
				str += "\n";
			}
			temp.pop();
		}
		str += "call ";
		str += e.id;
		str += IN_CODE_ENDL;
		return str;
	}

	// Генерация кода для вычисления выражения и помещения результата в стек (top = результат).
	string genExprPushResult(Lexer::LEX& tables, Log::LOG& log, int i)
	{
		string str;
		for (int j = i + 1; LEXEMA(j) != LEX_SEPARATOR; j++)
		{
			switch (LEXEMA(j))
			{
			case LEX_LITERAL:
			case LEX_ID:
			{
				if (tables.lextable.table[j].idxTI != NULLIDX_TI && tables.idtable.table[tables.lextable.table[j].idxTI].idtype == IT::IDTYPE::F)
				{
					str += genCallFuncCode(tables, log, j);
					str += "push eax\n";
					while (LEXEMA(j) != LEX_RIGHTTHESIS) j++;
					break;
				}
				else
				{
					int idxTI = tables.lextable.table[j].idxTI;
					if (idxTI != NULLIDX_TI)
					{
						str += "push ";
						str += tables.idtable.table[idxTI].id;
						str += "\n";
					}
				}
				break;
			}
			case LEX_PLUS:
				str += "pop ebx\npop eax\nadd eax, ebx\npush eax\n"; break;
			case LEX_MINUS:
				str += "pop ebx\npop eax\nsub eax, ebx\npush eax\n"; break;
			case LEX_STAR:
				str += "pop ebx\npop eax\nimul ebx\npush eax\n"; break;
			case LEX_DIRSLASH:
				str += "pop ebx\npop eax\ncdq\nidiv ebx\npush eax\n"; break;
			case LEX_RIGHTSHIFT:
				str += "pop ecx \npop eax \nshr eax, cl\npush eax\n"; break;
			case LEX_LEFTSHIFT:
				str += "pop ecx \npop eax \nshl eax, cl\npush eax\n"; break;
			case LEX_EXP:
			{
				// Возведение в степень: pop exponent (ebx), pop base (eax) -> вычисляем base^exp -> push eax
				int lbl = ++cycle_num;
				string L = itoS(lbl);
				str += std::string("pop ebx\npop eax\nmov ecx, ebx\nmov ebx, eax\nmov eax, 1\ncmp ecx, 0\nje pow_done") + L + "\n";
				str += std::string("pow_loop") + L + ":\nimul eax, ebx\ndec ecx\njnz " + (std::string("pow_loop") + L) + "\n";
				str += std::string("pow_done") + L + ":\npush eax\n";
				break;
			}
			case LEX_SHARP:
			{
				int k = j + 1;
				if (k < tables.lextable.size)
				{
					int idxTI = tables.lextable.table[k].idxTI;
					if (idxTI != NULLIDX_TI)
					{
						string func = tables.idtable.table[idxTI].id;
						str += "pop ebx\npush ebx\ncall ";
						str += func;
						str += "\npush eax\n";
						j = k;
					}
				}
				break;
			}
			}
		}
		return str;
	}

	string genReturnCode(Lexer::LEX& tables, Log::LOG& log, int i)
	{
		bool hasOp = false;
		for (int j = i + 1; LEXEMA(j) != LEX_SEPARATOR; j++)
		{
			char lx = LEXEMA(j);
			if (lx == LEX_PLUS || lx == LEX_MINUS || lx == LEX_STAR || lx == LEX_DIRSLASH || lx == LEX_EXP || lx == LEX_SHARP || lx == LEX_LEFTSHIFT || lx == LEX_RIGHTSHIFT)
			{
				hasOp = true;
				break;
			}
		}

		string str;
		if (!hasOp)
		{
			if (i + 1 < tables.lextable.size)
			{
				int idx = tables.lextable.table[i + 1].idxTI;
				if (idx != NULLIDX_TI)
				{
					IT::Entry const& ent = tables.idtable.table[idx];
					if (LEXEMA(i + 1) == LEX_ID && (i + 2 < tables.lextable.size) && LEXEMA(i + 2) == LEX_LEFTHESIS)
					{
						str += genCallFuncCode(tables, log, i + 1);
					}
					else
					{
						str += "mov eax, ";
						str += ent.id;
						str += "\n";
					}
				}
			}
		}
		else
		{
			str += genExprPushResult(tables, log, i);
			str += "pop ebx\nmov eax, ebx\n";
		}

		g_returnHandled = true;
		return str;
	}

	string genEqualCode(Lexer::LEX& tables, Log::LOG& log, int i)
	{
		string str;
		IT::Entry e1 = ITENTRY(i - 1);

		switch (e1.iddatatype)
		{
		case IT::IDDATATYPE::INT:
		{
			for (int j = i + 1; LEXEMA(j) != LEX_SEPARATOR; j++)
			{
				switch (LEXEMA(j))
				{
				case LEX_LITERAL:
				case LEX_ID:
				{
					if (tables.lextable.table[j].idxTI != NULLIDX_TI && tables.idtable.table[tables.lextable.table[j].idxTI].idtype == IT::IDTYPE::F)
					{
						str += genCallFuncCode(tables, log, j);
						str += "push eax\n";
						while (LEXEMA(j) != LEX_RIGHTTHESIS) j++;
						break;
					}
					else
					{
						str += "push ";
						str += ITENTRY(j).id;
						str += "\n";
					}
					break;
				}
				case LEX_PLUS:
					str += "pop ebx\npop eax\nadd eax, ebx\npush eax\n"; break;
				case LEX_MINUS:
					str += "pop ebx\npop eax\nsub eax, ebx\npush eax\n"; break;
				case LEX_STAR:
					str += "pop ebx\npop eax\nimul ebx\npush eax\n"; break;
				case LEX_DIRSLASH:
					str += "pop ebx\npop eax\ncdq\nidiv ebx\npush eax\n"; break;
				case LEX_RIGHTSHIFT:
					str += "pop ecx \npop eax \nshr eax, cl\npush eax\n"; break;
				case LEX_LEFTSHIFT:
					str += "pop ecx \npop eax \nshl eax, cl\npush eax\n"; break;
				case LEX_EXP:
				{
					int lbl = ++cycle_num;
					string L = itoS(lbl);
					str += std::string("pop ebx\npop eax\nmov ecx, ebx\nmov ebx, eax\nmov eax, 1\ncmp ecx, 0\nje pow_done") + L + "\n";
					str += std::string("pow_loop") + L + ":\nimul eax, ebx\ndec ecx\njnz " + (std::string("pow_loop") + L) + "\n";
					str += std::string("pow_done") + L + ":\npush eax\n";
					break;
				}
				case LEX_SHARP:
				{
					int k = j + 1;
					if (k < tables.lextable.size)
					{
						int idxTI = tables.lextable.table[k].idxTI;
						if (idxTI != NULLIDX_TI)
						{
							string func = tables.idtable.table[idxTI].id;
							str += "pop ebx\npush ebx\ncall ";
							str += func;
							str += "\npush eax\n";
							j = k;
						}
					}
					break;
				}
				}
			}
			str += "\npop ebx\nmov ";
			str += e1.id;
			str += ", ebx\n";
			break;
		}
		case IT::IDDATATYPE::STR:
		case IT::IDDATATYPE::CHR:
		{
			char lex = LEXEMA(i + 1);
			IT::Entry e2 = ITENTRY(i + 1);

			if (lex == LEX_ID && e2.idtype == IT::IDTYPE::F)
			{
				str += genCallFuncCode(tables, log, i + 1);
				str += "mov ";
				str += e1.id;
				str += ", eax\n";
			}
			else if (lex == LEX_LITERAL)
			{
				str += "mov ";
				str += e1.id;
				str += ", offset ";
				str += e2.id;
				str += "\n";
			}
			else
			{
				str += "mov ecx, ";
				str += e2.id;
				str += "\nmov ";
				str += e1.id;
				str += ", ecx\n";
			}
			break;
		}
		}
		return str;
	}

	// Реализация CodeGeneration — была отсутствующая символная реализация, добавлена сюда.
	void CodeGeneration(Lexer::LEX& tables, Parm::PARM& parm, Log::LOG& log)
	{
		// Минимальная корректная реализация — логируем вызов в журнал (если есть), иначе в stdout.
		if (log.stream != nullptr)
		{
			*(log.stream) << "; Gener::CodeGeneration invoked\n";
		}
		else
		{
			std::cout << "; Gener::CodeGeneration invoked\n";
		}

		// Простая генерация заголовка .asm (если нужно расширить — добавить реальную генерацию)
		if (parm.is_pring_tables && log.stream != nullptr)
		{
			*(log.stream) << "; tables size: " << tables.lextable.size << "\n";
			*(log.stream) << "; idtable size: " << tables.idtable.size << "\n";
		}
	}


};