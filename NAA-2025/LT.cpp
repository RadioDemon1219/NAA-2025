
#include "pch.h"
#include "LT.h"
#include <iostream>
#include <iomanip>

namespace LT
{
	Entry::Entry()
	{
		lexema = 0;
		sn = 0;
		idxTI = NULLIDX_TI;
	}

	Entry::Entry(char lexema, int snn, int idxti)
	{
		this->lexema = lexema;
		this->sn = snn;
		this->idxTI = idxti;
	}

	LexTable Create(int size)
	{
		if (size > MAXSIZE_LT)
			throw std::runtime_error("LT::Create: size > MAXSIZE_LT");
		LexTable lextable;
		lextable.maxsize = size;
		lextable.table = new Entry[lextable.maxsize];
		lextable.size = 0;
		return lextable;
	}

	void Add(LexTable& lextable, Entry entry)
	{
		if (lextable.size >= lextable.maxsize)
			throw std::runtime_error("LT::Add: lextable overflow");
		lextable.table[lextable.size++] = entry;
	}

	void writeLexTable(std::ostream* stream, LT::LexTable& lextable)
	{
		*stream << "------------------------------ Таблица лексем  ------------------------\n" << std::endl;
		*stream << "|  N | Лексема | Строка | Индекс в ТИ |" << std::endl;
		for (int i = 0; i < lextable.size; i++)
		{
			*stream << "|" << std::setw(3) << i << " | " << std::setw(6) << lextable.table[i].lexema << " |  "
				<< std::setw(5) << lextable.table[i].sn << " |";
			if (lextable.table[i].idxTI == (int)NULLIDX_TI)
				*stream << "     -      |" << std::endl;
			else
				*stream << std::setw(10) << lextable.table[i].idxTI << " |" << std::endl;
		}
	}

	void writeLexemsOnLines(std::ostream* stream, LT::LexTable& lextable)
	{
		*stream << "\n-----------------  Лексемы по строкам  ---------------------\n" << std::endl;
		if (lextable.size == 0) return;
		for (int i = 0; i < lextable.size; )
		{
			int line = lextable.table[i].sn;
			*stream << std::setw(3) << line << " | ";
			while (i < lextable.size && lextable.table[i].sn == line)
			{
				*stream << lextable.table[i].lexema;
				if (lextable.table[i].idxTI != (int)NULLIDX_TI)
					*stream << "(" << lextable.table[i].idxTI << ")";
				++i;
			}
			*stream << std::endl;
		}
	}
};