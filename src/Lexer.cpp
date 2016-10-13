#include "Lexer.hpp"
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <limits>



Lexer::Lexer(std::istream& input) :
	stream(input)
{}

Token Lexer::next(Context context)
{
	token = NO_TOKEN;
	value.erase();

    do {
    	charBuf = stream.get();
    	if(stream.eof())
	    {
	        token = END_STREAM;
	        return token;
	    }
    } while(skipComments());

    if(token != NO_TOKEN)
    	return token;

    switch(charBuf)
    {
    	case '{':
    		token = MAP_FLOW_BEGIN;
    		break;
    	case '[':
    		token = SEQ_FLOW_BEGIN;
    		break;
    	case '}':
    		token = MAP_FLOW_END;
    		break;
    	case ']':
    		token = SEQ_FLOW_END;
    		break;
    	case '?':
    		if(stream.peek() == ' ')
    			token = MAP_BLOCK_BEGIN;
    		break;
    	case '-':
    		if(stream.peek() == ' ')
    			token = SEQ_BLOCK_BEGIN;
    		break;
    	case ':':
    		if(stream.peek() == ' ')
    			token = MAP_VALUE_BEGIN;
    		break;
    	default:
    		token = ERROR;
    		break;
    }

	return token;
}

Token Lexer::token()
{
	return token;
}

std::string Lexer::value()
{
	return std::string(1,charBuf);
}

bool Lexer::skipComments()
{
	if(charBuf == '/') // c style comments
    {
        switch(stream.peek())
        {
            case '/':
                stream.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                charBuf = '\n';
                token = NEW_LINE;
                return false;
            case '*':
                stream.get();
                do
                {
                    stream.ignore(std::numeric_limits<std::streamsize>::max(),'*');
                } while(!stream.eof() && stream.peek()!='/');
                stream.get();
                return true;
            default:
                return false;
        }
    }
    else if(charBuf == ' ' && stream.peek() == '#') // yaml style comments
    {
        stream.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        charBuf = '\n';
        token = NEW_LINE;
        return false;
    }
}

inline std::string utf8Convert(unsigned int hex)
{
    std::string result;
    if(hex <= 0x7F) // 1 byte
    {
        result.push_back(hex); // 0b 0xxx xxxx
    }
    else if(hex <= 0x7FF) // 2 byte
    {
        result.push_back((hex>>6) + 192); // 0b 110x xxxx
        result.push_back((hex%0x40) + 128); // 0b 10xx xxxx
    }
    else if(hex <= 0xFFFF) // 3 byte
    {
        result.push_back((hex>>12) + 224); // 0b 1110 xxxx
        result.push_back((hex>>6)%0x40 + 128); // 0b 10xx xxxx
        result.push_back((hex%0x40) + 128); // 0b 10xx xxxx
    }
    else if(hex <= 0x1FFFFF) // 4 byte
    {
        result.push_back((hex>>18) + 240); // 0b 1111 0xxx
        result.push_back((hex>>12)%0x40 + 128); // 0b 10xx xxxx
        result.push_back((hex>>6)%0x40 + 128); // 0b 10xx xxxx
        result.push_back((hex%0x40) + 128); // 0b 10xx xxxx
    }
    return result;
}

// on entre apres : "'
// on sort avec : "'
std::string Lexer::readString(char endChar, bool escape)
{
    std::string result;
    for(char c = stream.get(); c!=endChar && !stream.eof(); c = stream.get())
    {
        if(escape && c=='\\')
        {
            std::string hex; // nombres
            int nbChar = 0;

            c = stream.get();
            switch(c)
            {
            // C escape sequences
                case '0':   c = '\0'; break;
                case 'a':   c = '\a'; break;
                case 'b':   c = '\b'; break;
                case 'f':   c = '\f'; break;
                case 'n':   c = '\n'; break;
                case 'r':   c = '\r'; break;
                case 't':   c = '\t'; break;
                case 'v':   c = '\v'; break;
                case 'U':   nbChar = 8; // \Unnnnnnnn (UTF-32)
                case 'u':   nbChar = 4; // \unnnn     (UTF-16)
            // YAML escape sequences
                case 'x':   nbChar = 2; // \xnn       (UTF-8)
                    for(int i=0; i<nbChar; i++)
                    {
                        c = stream.get();
                        if(!stream.eof() && isxdigit(c))
                            hex.push_back(c);
                        else
                        {
                            result.append(hex);
                            break;
                        }
                    }
                    goto l_convert;
				case 'N':   hex = "0085"; goto l_convert;
				case '_':   hex = "00a0"; goto l_convert;
				case 'L':   hex = "2028"; goto l_convert;
				case 'P':   hex = "2029"; goto l_convert;
                l_convert:
                    hex = utf8Convert(static_cast<unsigned int>(strtoul(hex.c_str(),0,16)));
                    break;
                case 'e':   c = 0x1b; break;
                default:
                    // TODO octal
                    break;
            }
        }
        if(!stream.eof())
            result.push_back(c);
    }
    return result;
}

