#include "pch.h"
#include "IT.h"
#include "LT.h"
#include "Error.h"
#include "LexAnaliz.h"
#include "SemAnaliz.h"

namespace Semantic
{
	bool Semantic::semanticsCheck(Lexer::LEX& tables, Log::LOG& log)
	{
		bool sem_ok = true;

		for (int i = 0; i < tables.lextable.size; i++)
		{
			switch (tables.lextable.table[i].lexema)
			{
			case '@':
			{
				if (tables.lextable.table[i + 1].lexema != LEX_ID_TYPE)
				{
					sem_ok = false;
					Log::writeError(log.stream, Error::GetError(303, tables.lextable.table[i].sn, 0));
				}
				break; // Добавлен break
			}
			case LEX_DIRSLASH:
			{
				// Проверка деления на ноль для ЛИТЕРАЛОВ (единственная надежная проверка на этапе семантики)
				if (tables.lextable.table[i + 1].lexema == LEX_LITERAL)
				{
					// Проверить, что операнд — это числовой литерал, равный 0
					if (tables.lextable.table[i + 1].idxTI != NULLIDX_TI)
					{
						IT::Entry literal_entry = tables.idtable.table[tables.lextable.table[i + 1].idxTI];
						if (literal_entry.iddatatype == IT::IDDATATYPE::INT && literal_entry.value.vint == 0)
						{
							sem_ok = false;
							Log::writeError(log.stream, Error::GetError(318, tables.lextable.table[i].sn, 0)); // Используем 'i', где обнаружен '/'
						}
					}
				}
				// Проверку для ID пропускаем, т.к. ее нельзя надежно выполнить статически.
				break;
			}
			case LEX_EQUAL: // выражение
			{
				if (i > 0 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI) // левый операнд
				{
					IT::IDDATATYPE lefttype = tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype;
					bool in_function_call = false; // Флаг для игнорирования параметров внутри вызова функции

					for (int k = i + 1; tables.lextable.table[k].lexema != LEX_SEPARATOR; k++)
					{
						if (k >= tables.lextable.size)
							break; // синтакс ошибка - нет ;

						// --- Обработка вызова функции в выражении ---
						if (tables.lextable.table[k].lexema == LEX_ID &&
							(tables.idtable.table[tables.lextable.table[k].idxTI].idtype == IT::IDTYPE::F ||
								tables.idtable.table[tables.lextable.table[k].idxTI].idtype == IT::IDTYPE::S))
						{
							// Если это функция, проверяем тип возвращаемого значения с левым операндом
							IT::IDDATATYPE func_return_type = tables.idtable.table[tables.lextable.table[k].idxTI].iddatatype;
							if (lefttype != func_return_type)
							{
								Log::writeError(log.stream, Error::GetError(314, tables.lextable.table[k].sn, 0));
								sem_ok = false;
								break;
							}

							in_function_call = true; // Начинаем игнорировать параметры
							continue; // Переходим к следующей лексеме после имени функции
						}

						// --- Внутри вызова функции игнорируем токены до ')' ---
						if (in_function_call)
						{
							if (tables.lextable.table[k].lexema == LEX_RIGHTTHESIS)
							{
								in_function_call = false; // Выход из вызова
							}
							continue;
						}
						// ----------------------------------------------------

						// --- Проверка типа операнда (не функции и не внутри ее вызова) ---
						if (tables.lextable.table[k].idxTI != NULLIDX_TI)
						{
							IT::IDDATATYPE righttype = tables.idtable.table[tables.lextable.table[k].idxTI].iddatatype;

							// Сравнение типа правого операнда (переменной/литерала) с типом левой части
							if (lefttype != righttype)
							{
								Log::writeError(log.stream, Error::GetError(314, tables.lextable.table[k].sn, 0));
								sem_ok = false;
								break;
							}
						}

						// --- Проверка недопустимых операций для строкового типа ---
						if (lefttype == IT::IDDATATYPE::STR)
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_MINUS || l == LEX_STAR || l == LEX_DIRSLASH) // + обычно используется для конкатенации, -* / недопустимы
							{
								Log::writeError(log.stream, Error::GetError(316, tables.lextable.table[k].sn, 0));
								sem_ok = false;
								break;
							}
						}
					}
				}
				break;
			}
			case LEX_ID: // проверка типа возвращаемого значения и параметров
			{
				IT::Entry e = tables.idtable.table[tables.lextable.table[i].idxTI];

				// Проверка возвращаемого значения функции (если это объявление LEX_FUNC)
				if (i > 0 && tables.lextable.table[i - 1].lexema == LEX_FUNC)
				{
					if (e.idtype == IT::IDTYPE::F)
					{
						bool return_found = false;
						for (int k = i + 1; ; k++)
						{
							char l = tables.lextable.table[k].lexema;
							if (l == LEX_RETURN)
							{
								return_found = true;
								int next_idxTI = tables.lextable.table[k + 1].idxTI;

								// Проверка типа
								if (next_idxTI != NULLIDX_TI)
								{
									// тип функции и возвращаемого значения не совпадают
									if (tables.idtable.table[next_idxTI].iddatatype != e.iddatatype)
									{
										Log::writeError(log.stream, Error::GetError(315, tables.lextable.table[k].sn, 0));
										sem_ok = false;
									}
								}
								break;
							}
							// Добавить проверку на конец функции (например, "}")
							if (l == '}') break;
							if (k >= tables.lextable.size) break;
						}
						// Если функция объявлена с типом, но нет return
						// if (e.iddatatype != IT::IDDATATYPE::PROC && !return_found) { ... }
					}
				}

				// Проверка вызова функции (LEX_ID LEX_LEFTHESIS)
				if (tables.lextable.table[i + 1].lexema == LEX_LEFTHESIS && tables.lextable.table[i - 1].lexema != LEX_FUNC)
				{
					if (e.idtype == IT::IDTYPE::F || e.idtype == IT::IDTYPE::S) // точно функция
					{
						int paramscount_actual = 0;

						// Начинаем проверку параметров после '(' (т.е. с i + 2)
						for (int j = i + 2; tables.lextable.table[j].lexema != LEX_RIGHTTHESIS; j++)
						{
							char l = tables.lextable.table[j].lexema;
							// Проверяем только ID или LITERAL, игнорируя запятые
							if (l == LEX_ID || l == LEX_LITERAL)
							{
								if (paramscount_actual >= MAX_PARAMS_COUNT) // Используйте MAX_PARAMS_COUNT из IT.h
								{
									Log::writeError(log.stream, Error::GetError(307, tables.lextable.table[i].sn, 0));
									sem_ok = false;
									break;
								}

								if (e.value.params.count != 0 && paramscount_actual < e.value.params.count)
								{
									IT::IDDATATYPE ctype = tables.idtable.table[tables.lextable.table[j].idxTI].iddatatype;

									// Проверка соответствия типов
									if (ctype != e.value.params.types[paramscount_actual])
									{
										Log::writeError(log.stream, Error::GetError(309, tables.lextable.table[i].sn, 0));
										sem_ok = false;
										break;
									}
								}
								paramscount_actual++;
							}

							if (j >= tables.lextable.size) // Защита от выхода за границы
								break;
						}

						// Проверка количества параметров
						if (paramscount_actual != e.value.params.count)
						{
							// Количество передаваемых и принимаемых параметров не совпадает
							Log::writeError(log.stream, Error::GetError(308, tables.lextable.table[i].sn, 0));
							sem_ok = false;
						}
					}
				}
				break;
			}
			case LEX_MORE:	case LEX_LESS: case LEX_EQUALS:case LEX_NOTEQUALS:
			{
				// левый и правый операнд - числовой тип
				bool flag = true;

				// Проверка левого операнда
				if (i > 1 && tables.lextable.table[i - 1].idxTI != NULLIDX_TI)
				{
					// КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: IDDATATYPE::NUM -> IDDATATYPE::INT
					if (tables.idtable.table[tables.lextable.table[i - 1].idxTI].iddatatype != IT::IDDATATYPE::INT)
						flag = false;
				}

				// Проверка правого операнда
				if (tables.lextable.table[i + 1].idxTI != NULLIDX_TI)
				{
					// КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: IDDATATYPE::NUM -> IDDATATYPE::INT
					if (tables.idtable.table[tables.lextable.table[i + 1].idxTI].iddatatype != IT::IDDATATYPE::INT)
						flag = false;
				}

				// Здесь нужно добавить проверку на ЛИТЕРАЛЫ, которые не являются INT
				// ... (если это необходимо по вашему языку)

				if (!flag)
				{
					// строка или неизвестный ид в условии
					Log::writeError(log.stream, Error::GetError(317, tables.lextable.table[i].sn, 0));
					sem_ok = false;
				}
				break;
			}
			}
		}

		return sem_ok;
	}
};