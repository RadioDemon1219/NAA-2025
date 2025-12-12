#include "pch.h"
#include "Header.h"
#include <iomanip>
#define W(x, y)\
		<< std::setw(x) << (y) <<
#define STR(n, line, type, id)\
		"|" W(4,n) " |  " W(5,line) "    |" W(17,type) " |  " W(SCOPED_ID_MAXSIZE, id) " |"


namespace IT
{
	IdTable Create(int size)
	{
		if (size > MAXSIZE_TI)
			throw ERROR_THROW(203);
		IdTable idtable;
		idtable.maxsize = size;
		idtable.table = new Entry[idtable.maxsize];
		idtable.size = 0;
		return idtable;
	}

	void Add(IdTable& idtable, Entry entry)
	{
		if (idtable.size >= idtable.maxsize)
			throw ERROR_THROW(203);
		idtable.table[idtable.size++] = entry;
	}

	int isId(IdTable& idtable, char id[SCOPED_ID_MAXSIZE])
	{
		for (int i = 0; i < idtable.size; i++)
		{
			if (strcmp(idtable.table[i].id, id) == 0)
				return i;
		}
		return NULLIDX_TI;
	}

	bool SetValue(IT::IdTable& idtable, int index, char* value)
	{
		return SetValue(&(idtable.table[index]), value);
	}

	bool SetValue(IT::Entry* entry, char* value)
	{
		bool rc = true;
		if (entry->iddatatype == INT)
		{
			int temp = atoi(value);
			if (temp > NUM_MAXSIZE || temp < NUM_MINSIZE)
			{
				if (temp > NUM_MAXSIZE)
					temp = NUM_MAXSIZE;
				if (temp < NUM_MINSIZE)
					temp = NUM_MINSIZE;
				rc = false;
			}
			entry->value.vint = temp;
		}
		else if (entry->iddatatype == IT::IDDATATYPE::CHR)
		{
			if (strlen(value) == 3 && value[0] == '\'' && value[2] == '\'')
			{
				entry->value.vchar = value[1];
			}
			else
			{
				rc = false;
			}
		}
		else
		{
			size_t vlen = strlen(value);
			for (size_t i = 1; i + 1 < vlen; i++)
				entry->value.vstr.str[i - 1] = value[i];
			entry->value.vstr.str[(vlen >= 2) ? (vlen - 2) : 0] = '\0';
			entry->value.vstr.len = (vlen >= 2) ? static_cast<int>(vlen - 2) : 0;
		}
		return rc;
	}
	void writeIdTable(std::ostream* stream, IT::IdTable& idtable)
	{
		*stream << "---------------------------- Таблица идентификаторов ------------------------\n" << std::endl;
		*stream << "|  N  |индекс в лт| тип идентификатора |        имя        | значение (по умолчанию)" << std::endl;
		for (int i = 0; i < idtable.size; i++)
		{
			IT::Entry* e = &idtable.table[i];
			char type[50] = "";

			switch (e->iddatatype)
			{
			case IT::IDDATATYPE::INT:
				strcat(type, "  number ");
				break;
			case IT::IDDATATYPE::STR:
				strcat(type, " string  ");
				break;
			case IT::IDDATATYPE::PROC:
				strcat(type, "   proc  ");
				break;
			case IT::IDDATATYPE::UNDEF:
				strcat(type, "UNDEFINED");
				break;
			case IT::IDDATATYPE::CHR:
				strcat(type, "  char   ");
				break;
			}

			switch (e->idtype)
			{
			case IT::IDTYPE::V:
				strcat(type, "  variable");
				break;
			case IT::IDTYPE::F:
				strcat(type, "  function");
				break;
			case IT::IDTYPE::P:
				strcat(type, " parameter");
				break;
			case IT::IDTYPE::L:
				strcat(type, "   literal");
				break;
			case IT::IDTYPE::S: strcat(type, "  LIB FUNC"); break;
			default:
				strcat(type, "UNDEFINED ");
				break;
			}

			*stream << STR(i, e->idxfirstLE, type, e->id);
			if (e->idtype == IT::IDTYPE::L || e->idtype == IT::IDTYPE::V && e->iddatatype != IT::IDDATATYPE::UNDEF)
			{
				if (e->iddatatype == IT::IDDATATYPE::INT)
					*stream << e->value.vint;
				else if (e->iddatatype == IT::IDDATATYPE::CHR)
					*stream << "'" << e->value.vchar << "'";
				else
					*stream << "[" << (int)e->value.vstr.len << "]" << e->value.vstr.str;
			}
			if (e->idtype == IT::IDTYPE::F || e->idtype == IT::IDTYPE::S)
			{
				for (int i = 0; i < e->value.params.count; i++)
				{
					*stream << " P" << i << ":";
					switch (e->value.params.types[i])
					{
					case IT::IDDATATYPE::INT:
						*stream << "NUMBER |";
						break;
					case IT::IDDATATYPE::STR:
						*stream << "STRING |";
						break;
					case IT::IDDATATYPE::PROC:
					case IT::IDDATATYPE::UNDEF:
						*stream << "UNDEFINED";
						break;
					}
				}
			}
			*stream << std::endl;
		}
		*stream << "\n-------------------------------------------------------------------------\n\n";
	}
};