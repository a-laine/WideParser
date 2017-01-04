#ifndef LEXER_HPP
#define LEXER_HPP

#include <istream>
#include <string>


enum Token {
	NEW_LINE,
	ERROR,
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

class Lexer
{
	public:
		Lexer(std::istream& input);

		Token next();
		Token getToken();
		std::string getValue();
		int getIndentation();

	private:
		std::istream& stream;
		std::string value;
		int column;
		char charBuf;

		char getChar();
		char skipComments(bool multiline = false);
		std::string readBlockScalar();
		std::string readQuotedString(char endChar, bool escape);
		std::string readAnchorName();
		std::string readPlainScalar();
		std::string readTagName();
};


#endif // LEXER_HPP