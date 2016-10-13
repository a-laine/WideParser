#ifndef LEXER_HPP
#define LEXER_HPP

#include <istream>
#include <string>

enum Token {
	NO_TOKEN,
	ERROR,
	END_STREAM,
	NEW_LINE,
	MAP_FLOW_BEGIN,
	SEQ_FLOW_BEGIN,
	MAP_FLOW_END,
	SEQ_FLOW_END
};

enum Context {
	FLOW_MAP,
	FLOW_SEQ,
	BLOCK_MAP,
	BLOCK_SEQ
};

class Lexer
{
	public:
		Lexer(std::istream& input);

		Token next(Context context);
		Token token();
		std::string value();

	private:
		std::istream& stream;
		Token token;
		char charBuf;
		std:string value;

		bool skipComments();
		std::string readString(char endChar, bool escape);
};


#endif // LEXER_HPP