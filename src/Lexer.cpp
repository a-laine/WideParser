#include "Lexer.hpp"
#include <limits>


inline void utf8Convert(std::string& result, unsigned int hex)
{
	if(hex <= 0x7F) // 1 byte
	{
		result.push_back(hex); // 0b 0xxx xxxx
	}
	else if(hex <= 0x7FF) // 2 byte
	{
		result.push_back((hex>>6) | 0XC0); // 0b 110x xxxx
		result.push_back((hex%0x40) | 0x80); // 0b 10xx xxxx
	}
	else if(hex <= 0xFFFF) // 3 byte
	{
		result.push_back((hex>>12) | 0xE0); // 0b 1110 xxxx
		result.push_back((hex>>6)%0x40 | 0x80); // 0b 10xx xxxx
		result.push_back((hex%0x40) | 0x80); // 0b 10xx xxxx
	}
	else if(hex <= 0x1FFFFF) // 4 byte
	{
		result.push_back((hex>>18) | 0xF0); // 0b 1111 0xxx
		result.push_back((hex>>12)%0x40 | 0x80); // 0b 10xx xxxx
		result.push_back((hex>>6)%0x40 | 0x80); // 0b 10xx xxxx
		result.push_back((hex%0x40) | 0x80); // 0b 10xx xxxx
	}
}

inline bool isHexChar(char c)
{
	// return c>='0' && c<='f' && (c<='F' || c>='a') && (c>='A' || c<='9'); // true(5,5,6) false(1,4,3,2)  => if often false 
	return (c<='9' && c>='0') || (c<='F' && c>='A') || (c>='a' && c<='f'); // true(2,4,6) false(5,4,3,4)  => if often true
}

inline bool isAlnumChar(char c)
{
	return (c<='9' && c>='0') || (c<='Z' && c>='A') || (c>='a' && c<='z');
}

inline bool isBlank(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline bool isFlowChar(char c)
{
	return c == '{' || c == '}' || c == '[' || c == ']' || c == ',';
}

inline bool isUriChar(char c)
{
	/*
	[A-Za-z0-9] | "!#$%&'()*+,-./:;=?@[]_~"
	=> #21 | [#23-#3B] | #3D | [#3F-#5B] |
	   #5D | #5F | [#61-#7A] | #7E
	*/
	return (c>=char(0x21) && c<=char(0x7a) && 
			c!='\"' && c!='<' && c!='>' &&
			c!='\\' && c!='^' && c!='`') ||
		   c=='~';
}

inline bool isTagChar(char c)
{
	/*
	ns-uri-char - "!,[]"
	*/
	return  (c<=char(0x5a) && c>=char(0x23) && c!=',' && c!='<' && c!='>') ||
			(c<=char(0x7a) && c>=char(0x5F) && c!='`') ||
			c=='~';
}

inline bool isAnchorChar(char c)
{
	/*
	=> c-printable - #FEFF
				- "\r" - "\n" - " " - "\t"
				- "," - "[" - "]" - "{" - "}"
	c-printable = #9 | #A | #D | [#20-#7E] | 
				  #85 | [#A0-#D7FF] | [#E000-#FFFD]
				  [#10000-10FFFF]
	ns-char     = [#21-#7E] | #85 | 
				  [#A0-#D7FF] | [#E000-#FFFD] | 
				  [#10000-10FFFF] - #FEFF
	=> ns-char - "," - "[" - "]" - "{" - "}"
	*/
	return (c & 0x80) || (
			c>=char(0x21) && c<=char(0x7e) &&
			c!=',' && c!='[' && c!=']' && c!='{' && c!='}'
		);
}














Lexer::Lexer(std::istream& input) :
	stream(input), /* open in binary mode! */
	column(0),
	prevIndent(0)
{}

std::string Lexer::getValue()
{
	return value;
}

Encoding Lexer::readEncoding()
{
	/*
	UTF32_BE  BOM  =>  00 00 FE FF
	UTF32_BE  ---  =>  00 00 00 ??
	UTF32_LE  BOM  =>  FF FE 00 00
	UTF32_LE  ---  =>  ?? 00 00 00
	UTF16_BE  BOM  =>  FE FF
	UTF16_BE  ---  =>  00 ??
	UTF16_LE  BOM  =>  FF FE
	UTF16_LE  ---  =>  ?? 00
	UTF8      BOM  =>  EF BB BF
	UTF8      ---  =>  other
	*/
	// Not supported
	return UTF8;
}

Lexer::TokenInfo Lexer::next(size_t indentation)
{
	prevIndent = indentation;
	size_t retIndent;
	char c;
	do
	{
		do
		{
			retIndent = column;
			c = getChar();
			if(eof())
				return TokenInfo({ retIndent, END_STREAM });
		} while(isBlank(c));

		// comments
		if( (c == '#' && isBlank(charBuf))  ||
			(c == '/' && peekChar() == '/')
		   )
		{
			skipComments();
			continue;
		}
		if(c == '/' && peekChar() == '*')
		{
			skipComments(true);
			charBuf = ' ';
			continue;
		}
	} while(0);

	// block indicators
	if(c == '?' && isBlank(peekChar()))
		return TokenInfo({ retIndent, BLOCK_MAP_ENTRY });
	if(c == '-' && isBlank(peekChar()))
		return TokenInfo({ retIndent, BLOCK_SEQ_ENTRY });
	if(c == ':')
		return TokenInfo({ retIndent, MAP_KEY_DELIMITER });
	
	// flow indicators
	if(c == '{')
		return TokenInfo({ retIndent, FLOW_MAP_BEGIN });
	if(c == '}')
		return TokenInfo({ retIndent, FLOW_MAP_END });
	if(c == '[')
		return TokenInfo({ retIndent, FLOW_SEQ_BEGIN });
	if(c == ']')
		return TokenInfo({ retIndent, FLOW_SEQ_END });
	if(c == ',')
		return TokenInfo({ retIndent, FLOW_DELIMITER });
	
	// scalars
	if(c == '|')
		return readBlockScalar(false), TokenInfo({ retIndent, SCALAR });
	if(c == '>')
		return readBlockScalar(true), TokenInfo({ retIndent, SCALAR });
	if(c == '"')
		return readQuotedString(c, true), TokenInfo({ retIndent, SCALAR });
	if(c == '\'')
		return readQuotedString(c, false), TokenInfo({ retIndent, SCALAR });
	if(c == '*')
		return readAnchorName(), TokenInfo({ retIndent, ALIAS });
	
	// node properties
	if(c == '&')
		return readAnchorName(), TokenInfo({ retIndent, ANCHOR });
	if(c == '!')
		return readTagName(), TokenInfo({ retIndent, TAG });
	
	// others
	return readPlainScalar(true), TokenInfo({ retIndent, SCALAR }); // TODO : replace true by *is in flow*
}

char Lexer::getChar()
{
	charBuf = stream.get();
	if(charBuf == '\r') {
		charBuf == '\n';
		if(stream.peek() == '\n')
			stream.get();
	}
	if(charBuf == '\n')
		column = 0;
	else if((charBuf & 0xC0) != 0x80) // unicode
		column++;
	return charBuf;
}

void Lexer::ungetChar()
{
	column--;
	stream.unget();
}

char Lexer::peekChar()
{
	return stream.peek();
}

bool Lexer::eof()
{
	return stream.eof();
}

void Lexer::skipComments(bool multiline)
{
	if(multiline) // c style only
	{
		while(!eof())
		{
			char c = getChar();
			if(c == '*' && peekChar() == '/')
			{
				getChar();
				break;
			}
		}
	}
	else
	{
		while(!eof())
		{
			if(getChar() == '\n')
				break;
		}
	}
}

void Lexer::readAnchorName()
{
	std::string& result = value;
	result.clear();
	while(!eof())
	{
		char c = peekChar();
		if(isAnchorChar(c))
		{
			result.push_back(c);
			getChar();
		}
		else break;
	}
}

// error handling:
//     - verbatim must start with "!<" and end with ">"
//     - verbatim must contains only Uri chars
//     - tag handles must be followed by a non-empty suffix
void Lexer::readTagName()
{
	std::string& result = value;
	result.clear();
	char c = peekChar();
	if(c == '<')
	{
		while(!eof())
		{
			c = peekChar();
			if(isUriChar(c))
			{
				getChar();
				result.push_back(c);
			}
			else break;
		}
		if(c == '>')
			getChar();
	}
	else
	{
		bool inHandle = true;
		while(!eof())
		{
			c = peekChar();
			if(isTagChar(c) || (inHandle && c == '!'))
			{
				getChar();
				result.push_back(c);
				inHandle = inHandle && (isAlnumChar(c) || c == '-');
			}
			else break;
		}
	}
}

// error handling:
//     - only header and comment on first line
//     - only one caracter for explicit indentation
//     - no spaces between header caracters (except comment)
//     - less indented lines lead to errors
void Lexer::readBlockScalar(bool folded)
{
	std::string& result = value;
	result.clear();

	// read header
	bool indentIsSet = false;
	size_t indent = prevIndent + 1;
	char chomping = 1; // 0: strip / 1: clip / 2: keep
	while(!eof())
	{
		char c = peekChar();
		if(!indentIsSet && (c <= '9' && c >= '1'))
		{
			indent += c - '1';
			indentIsSet = true;
		}
		else if(chomping == 1 && (c == '+' || c == '-'))
			chomping = c=='+'? 2 : 0;
		else
			break;
		getChar();
	}

	// possible comment + new line
	while(!eof())
	{
		char c = getChar();
		if(c == '\n')
			break;
	}

	// read content text
	bool lastIsFolded = false;
	int nbEndLine = 0;
	while(!eof())
	{
		// read indentation
		char c;
		while(!eof())
		{
			c = peekChar();
			if(c == ' ' || c == '\t')
			{
				getChar();
				if(indentIsSet && column == indent)
					break;
			}
			else
				break;
		}

		c = peekChar();
		if(c == '\n' || '\t')
		{
			getChar();
			nbEndLine++;
			lastIsFolded = false;
			continue;
		}
		else if(eof() || column < indent)
			break;
		else if(c != ' ' && c != '\t')
		{
			if(!indentIsSet)
				indent = column;
			if(folded)
			{
				if(lastIsFolded)
					result.push_back(' ');
				lastIsFolded = true;
				nbEndLine--;
			}
		}
		result.append(nbEndLine, '\n');
		nbEndLine = 0;

		// read content
		c = getChar();
		while(!eof())
		{
			if(c == '\n')
			{
				getChar();
				nbEndLine++;
				break;
			}
			result.push_back(c);
			c = getChar();
		}
	}
	
	switch(chomping)
	{
		case 0: // strip
			break;
		case 1: // clip
			result.push_back('\n');
			break;
		case 2: // keep
			result.append(nbEndLine, '\n');
			break;
		default:// programming error
			break;
	}
}

// error handling:
//     - no comments on multilines
//     - one key on a line (no compact notation)
//     - allways expect a single key or terminate by end of line
void Lexer::readPlainScalar(bool inFlow)
{
	std::string  blankBuffer;
	std::string& result = value;
	result.clear();
	ungetChar();
	int nbEndLine = 0;
	bool lastIsFolded = true;
	while(!eof())
	{
		char c = getChar();
		if(c == '\n')
		{
			blankBuffer.clear();
			while(!eof())
			{
				c = peekChar();
				if(c == ' ' || c == '\t')
					getChar();
				else
					break;
			}

			if(c == '\n' || c == '\r')
			{
				lastIsFolded = false;
				nbEndLine++;
			}
			else if(column <= prevIndent)
				break;
			else if(c == '#')
				break;
			else
			{
				if(lastIsFolded)
					blankBuffer.push_back(' ');
				lastIsFolded = true;
				blankBuffer.append(nbEndLine, '\n');
				nbEndLine = 0;
			}
		}
		else if(c == ' ' || c == '\t')
		{
			if(peekChar() == '#')
				break;
			else
				blankBuffer.push_back(c);
		}
		else if( (c == ':' && isBlank(peekChar())) ||
			     (c == '/' && peekChar() == '/')   ||
			     (inFlow && isFlowChar(c))
			    )
		{
			ungetChar();
			break;
		}
		else
		{
			if(!blankBuffer.empty())
			{
				result.append(blankBuffer);
				blankBuffer.clear();
			}
			result.push_back(c);
		}
	}
}

// error handling:
//     - respect indentation after '\' + '\n'
//     - one key on a line (no compact notation)
//     - allways expect a single key or terminate by end of line
void Lexer::readQuotedString(char endChar, bool escape)
{
	std::string blankBuffer;
	std::string& result = value;
	result.clear();
	int nbEndLine = 0;
	bool lastIsFolded = true;
	while(!eof())
	{
		char c = getChar();
		if(c == '\n')
		{
			blankBuffer.clear();
			while(!eof())
			{
				c = peekChar();
				if(c == ' ' || c == '\t')
					getChar();
				else
					break;
			}

			if(c == '\n' || c == '\r')
			{
				lastIsFolded = false;
				nbEndLine++;
			}
			else if(column <= prevIndent)
				break;
			else
			{
				if(lastIsFolded)
					result.push_back(' ');
				lastIsFolded = true;
				result.append(nbEndLine, '\n');
				nbEndLine = 0;
			}
		}
		else if(c == ' ' || c == '\t')
		{
			blankBuffer.push_back(c);
		}
		else if(escape && c=='\\')
		{
			if(!blankBuffer.empty())
			{
				result.append(blankBuffer);
				blankBuffer.clear();
			}
			if(peekChar() == '\n' || peekChar() == '\r')
			{
				getChar();
				while(!eof())
				{
					c = peekChar();
					if(c == ' ' || c == '\t')
						getChar();
					else
						break;
				}
				if(column <= prevIndent)
					break;
			}
			else
				parseEscape(result);
		}
		else if(c==endChar && !escape && peekChar()==endChar)
		{
			if(!blankBuffer.empty())
			{
				result.append(blankBuffer);
				blankBuffer.clear();
			}
			result.push_back(c);
			getChar();
			c = getChar();
		}
		else if(c != endChar)
		{
			if(!blankBuffer.empty())
			{
				result.append(blankBuffer);
				blankBuffer.clear();
			}
			result.push_back(c);
		}
		else
		{
			// keep it here!
			break;
		}
	}
}

void Lexer::parseEscape(std::string& result)
{
	unsigned int utfNum = 0; // unicodes points
	int nbChar = 2;

	char c = getChar();
	switch(c)
	{
	// C escape sequences
		case '0':   c = 0x00; break; // NUL: '\0'
		case 'a':   c = 0x07; break; // BEL: '\a'
		case 'b':   c = 0x08; break; // BS : '\b'
		case 't':   c = 0x09; break; // TAB: '\t'
		case 'n':   c = 0x0a; break; // LF : '\n'
		case 'v':   c = 0x0b; break; // VT : '\v'
		case 'f':   c = 0x0c; break; // FF : '\f'
		case 'r':   c = 0x0d; break; // CR : '\r'
		case 'e':   c = 0x1b; break; // ESC: '\e' may be not recognized
		case 'U':   nbChar *= 2;     // \Unnnnnnnn (UTF-32)
		case 'u':   nbChar *= 2;     // \unnnn     (UTF-16)
	// YAML escape sequences
		case 'x':                    // \xnn       (UTF-8)
			for(int i=0; i<nbChar; i++) {
				if(!eof() && isHexChar(peekChar())) {
					c = getChar();
					c -= (c<='9')? '0' : (c<='Z')? ('A'-10) : ('a'-10);
					utfNum = (utfNum << 4) | c;
				}
				else break;
			}
			c = 0x00;
			break;
		case 'N':   utfNum = 0x0085; break;
		case '_':   utfNum = 0x00a0; break;
		case 'L':   utfNum = 0x2028; break;
		case 'P':   utfNum = 0x2029; break;
		default:
			break;
	}

	if(!eof())
	{
		if(utfNum > 0)
			utf8Convert(result, utfNum);
		else
			result.push_back(c);
	}
}


