#include "pch.h"
#include "Header.h"
#include <iostream>
#include <cstring>

namespace FST //finite state transitions
{
	RELATION::RELATION(char c /*= NULL*/, short ns /*= NULL*/)
	{
		this->symbol = c;
		this->nnode = ns;
	};

	NODE::NODE()
	{
		this->n_relation = NULL;
		this->relations = NULL;
	};

	NODE::NODE(short n, RELATION rel, ...)
	{
		RELATION* temp = &rel;
		this->relations = new RELATION[n];
		this->n_relation = n;
		for (short i = 0; i < n; i++)
			this->relations[i] = *(temp + i);
	};

	FST::FST(short ns, NODE n, ...)
	{
		this->node = new NODE[ns];
		NODE* temp = &n;
		this->nstates = ns;
		this->rstates = new short[ns];
		for (short i = 0; i < ns; i++)
			this->node[i] = *(temp + i);
		rstates[0] = 0;
		position = -1;
	}

	FST::FST(char* s, FST& fst)
	{
		this->node = new NODE[fst.nstates];
		NODE* temp = fst.node;
		this->string = s;
		this->nstates = fst.nstates;
		this->rstates = new short[this->nstates];
		for (short i = 0; i < this->nstates; i++)
			this->node[i] = *(temp + i);
		rstates[0] = 0;
		position = -1;

	}
	bool step(FST& fst, short*& rstates)           //one step
	{
		bool rc = false;
		std::swap(rstates, fst.rstates);
		for (short i = 0; i < fst.nstates; i++)
		{
			if (rstates[i] == fst.position)
			{
				for (short j = 0; j < fst.node[i].n_relation; j++)
				{
					// Поддержка "подстановочного" символа '\xFF' в отношениях:
					// если relation.symbol == 0xFF, то это соответствует любому символу (используется для поддержки UTF-8/кириллицы в литералах)
					char rel = fst.node[i].relations[j].symbol;
					unsigned char cur = static_cast<unsigned char>(fst.string[fst.position]);
					if (rel == static_cast<char>(0xFF) || static_cast<unsigned char>(rel) == cur)
					{
						fst.rstates[fst.node[i].relations[j].nnode] = fst.position + 1;
						rc = true;
					};
				};
			}

		};
		return rc;
	};
	bool execute(FST& fst)                      //run automaton
	{
		short* rstates = new short[fst.nstates];
		memset(rstates, 0xff, sizeof(short) * fst.nstates);
		int lstring = static_cast<int>(strlen(fst.string));
		bool rc = true;
		for (int i = 0; i < lstring && rc; i++)
		{
			fst.position++;
			rc = step(fst, rstates);
		};
		delete[] rstates;
		return (rc ? (fst.rstates[fst.nstates - 1] == lstring) : rc);
	};

}