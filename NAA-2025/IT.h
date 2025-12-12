#pragma once
#include <iostream>

#define MAXSIZE_ID	8					//���� ����� �������� ��������������
#define SCOPED_ID_MAXSIZE   MAXSIZE_ID*2 //���� ����� �������� ������������� + ������� ���������
#define MAXSIZE_TI		4096			//���� ����� ���������� ����� � ������� ���������������	
#define NUM_DEFAULT	0x00000000		//�������� �� ��������� ��� int
#define STR_DEFAULT	0x00			//�������� �� ��������� ��� sting

#ifndef NULLIDX_TI
#define NULLIDX_TI		0xffffffff		//��� �������� ������� ���������������
#endif

#define STR_MAXSIZE	255				//������������ ����� ���������� ��������
#define NUM_MAXSIZE   0x7fffffff		//������������ �������� ��� ���� number
#define NUM_MINSIZE  0x80000000		
#define MAX_PARAMS_COUNT 20				
#define CONCAT_PARAMS_CNT 2			
#define LENGHT_PARAMS_CNT 1	
#define ATOII_PARAMS_CNT 1 
#define CONCAT_TYPE IT::IDDATATYPE::STR
#define LENGHT_TYPE IT::IDDATATYPE::INT
#define ATOII_TYPE IT::IDDATATYPE::INT



namespace IT
{
	enum IDDATATYPE { INT = 1, STR = 2, PROC = 3, CHR = 4, UNDEF };
	enum IDTYPE { V = 1, F = 2, P = 3, L = 4, S = 5 };	

	
	enum STDFNC { F_NOT_STD = 0, F_CONCAT = 1, F_LENGHT = 2, F_ATOII = 3 };

	static const IDDATATYPE CONCAT_PARAMS[] = { IT::IDDATATYPE::STR, IT::IDDATATYPE::STR };
	static const IDDATATYPE LENGHT_PARAMS[] = { IT::IDDATATYPE::STR };
	static const IDDATATYPE ATOII_PARAMS[] = { IT::IDDATATYPE::STR };
	struct Entry
	{
		union
		{
			int	vint;            			
			char vchar;						
			struct
			{
				int len;					
				char str[STR_MAXSIZE - 1];
			} vstr;							
			struct
			{
				int count;					
				IDDATATYPE* types;			
			} params;
		} value;						
		int			idxfirstLE;						
		char		id[SCOPED_ID_MAXSIZE];	
		IDDATATYPE	iddatatype;				
		IDTYPE		idtype;					

		Entry()							
		{
			this->value.vint = NUM_DEFAULT;
			this->value.vstr.len = 0;
			this->value.params.count = 0;
		};
		Entry(char* id, int idxLT, IDDATATYPE datatype, IDTYPE idtype) 
		{
			strncpy_s(this->id, id, SCOPED_ID_MAXSIZE - 1);
			this->idxfirstLE = idxLT;
			this->iddatatype = datatype;
			this->idtype = idtype;
		};
	};
	struct IdTable		
	{
		int maxsize;
		int size;		
		Entry* table;	
	};
	IdTable Create(int size = NULL);	
	void Add(					
		IdTable& idtable,		
		Entry entry);			
	int isId(					
		IdTable& idtable,		
		char id[SCOPED_ID_MAXSIZE]);	
	bool SetValue(IT::Entry* entry, char* value);	
	bool SetValue(IT::IdTable& idtable, int index, char* value);
	void writeIdTable(std::ostream* stream, IT::IdTable& idtable); 
};