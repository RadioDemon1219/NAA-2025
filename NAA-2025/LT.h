#pragma once
#include <fstream>

#define LEX_MAXSIZE 2048
#define LT_MAXSIZE 4096
#define LEX_NULL ' '

#define LEX_SEPARATORS	 'S'
#define	LEX_ID_TYPE	     't'
#define	LEX_ID			 'i'
#define	LEX_LITERAL		 'l'
#define	LEX_FUNC		 'u'
#define	LEX_MAIN		 'm'
#define LEX_DECL		 'd'
#define LEX_OUTPUT		 'o'
#define LEX_INPUT		 'k'
#define LEX_RETURN		 'e'
#define LEX_PROCEDURE	 'p'

#define	LEX_SEPARATOR	 ';'
#define LEX_PIPE         '|'
#define LEX_COMMA		 ','

#define	LEX_LEFTBRACE	 '{'
#define	LEX_RIGHTBRACE	 '}'

#define	LEX_LEFTBRACKET	 '['
#define	LEX_BRACKET		 ']'

#define	LEX_LEFTHESIS	 '('
#define	LEX_RIGHTTHESIS	 ')'

#define	LEX_PLUS		 '+'
#define	LEX_MINUS		 '-'
#define	LEX_STAR		 '*'
#define	LEX_DIRSLASH	 '/'
#define	LEX_EQUAL		 '='
#define LEX_EXP           '^'

#define LEX_MORE		 '>'
#define LEX_LESS		 '<'
#define LEX_EQUALS		 '&'
#define LEX_NOTEQUALS	 '!'

#define LEX_LEFTSHIFT    'L'
#define LEX_RIGHTSHIFT   'R'

#define LEX_SHARP		 '#'
#define LEX_INT_SUFFIX    'j'
#define LEX_LEN_SUFFIX    'f'

#define LEX_SWITCH        'x'
#define LEX_CASE          'y'
#define LEX_BREAK         'z'
#define LEX_DEFAULT       'a'

#define LEX_LITERAL_OCT	 'q'
#define LEX_NEWLINE		 '~'

#define	LEXEMA_FIXSIZE	 1
#define	MAXSIZE_LT		 4096

#ifndef NULLIDX_TI
#define	NULLIDX_TI	 0xffffffffu
#endif

namespace LT
{
	struct Entry
	{
		char lexema;
		int sn;
		int idxTI;
		Entry();
		Entry(char lexema, int snn, int idxti = NULLIDX_TI);
	};

	struct LexTable
	{
		int maxsize;
		int size;
		Entry* table;
	};

	LexTable Create(int size);
	void Add(LexTable& lextable, Entry entry);
	void writeLexTable(std::ostream* stream, LT::LexTable& lextable);
	void writeLexemsOnLines(std::ostream* stream, LT::LexTable& lextable);
};

#define ISTYPE_LEX(lex) (lex == LEX_ID_TYPE)