#ifndef LEXER_HPP
#define LEXER_HPP

#include <istream>
#include <string>


enum Token {
	END_STREAM,
	BLOCK_MAP_ENTRY,
	BLOCK_SEQ_ENTRY,
	MAP_KEY_DELIMITER,
	FLOW_MAP_BEGIN,
	FLOW_MAP_END,
	FLOW_SEQ_BEGIN,
	FLOW_SEQ_END,
	FLOW_DELIMITER,
	SCALAR,
	ALIAS,
	ANCHOR,
	TAG
};

enum Encoding {
	UTF8,
	UTF16_LE,
	UTF16_BE,
	UTF32_LE,
	UTF32_BE
};

class Lexer
{
	public:
		struct TokenInfo
		{
			size_t indent;
			Token token;
		};

	public:
		Lexer(std::istream& input);

		Encoding readEncoding();
		TokenInfo next(size_t indentation);
		std::string getValue();

	private:
		char getChar();
		void ungetChar();
		char peekChar();
		bool eof();

		void skipComments(bool multiline = false);
		void readBlockScalar(bool folded);
		void readQuotedString(char endChar, bool escape);
		void readAnchorName();
		void readPlainScalar(bool inFlow);
		void readTagName();

		void parseEscape(std::string& result);

	private:
		std::istream& stream;
		std::string value;
		size_t column;
		size_t prevIndent;
		char charBuf;
};


#endif // LEXER_HPP