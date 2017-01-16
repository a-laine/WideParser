#include "Lexer.hpp"
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <limits>




Lexer::Lexer(std::istream& input) :
	stream(input),
    column(0)
{}

char Lexer::getChar()
{
    char c;
    do
    {
        c = stream.get();
        if(c == '\r') { // OSX & Windows EOL
            c = '\n';
            if(stream.peek() == '\n')
                stream.get();
        }
        else if(c == ' ' || c == '\t') {
            c = ' ';
            charBuf = c;
        }
        if(c == '\n')
            column = 0;
        else if((c & 0xC0) != 0x80) // unicode
            column++;
    } while(c == ' ');
    return c;
}

Token Lexer::next()
{
    char c;
_begin:
    if(stream.eof())
        return END_STREAM;
    c = getChar();

    // comments
    if(c == '#' && (charBuf==' ' || column==0))
        return skipComments(), NEW_LINE;
    if(c == '/' && stream.peek() == '/')
        return skipComments(), NEW_LINE;
    if(c == '/' && stream.peek() == '*') {
        skipComments(true);
        goto _begin;
    }
    
    charBuf = c;

    // new line
    if(c == '\n' || c == '\r')
        return NEW_LINE;

    // block indicators
    if(c == '?') // TODO  next = blank
        return BLOCK_MAP_ENTRY;
    if(c == '-') // TODO  next = blank
        return BLOCK_SEQ_ENTRY;
    if(c == ':') // TODO  next = blank
        return MAP_KEY_DELIMITER;
    
    // flow indicators
    if(c == '{')
        return FLOW_MAP_BEGIN;
    if(c == '}')
        return FLOW_MAP_END;
    if(c == '[')
        return FLOW_SEQ_BEGIN;
    if(c == ']')
        return FLOW_SEQ_END;
    if(c == ',')
        return FLOW_DELIMITER;
    
    // scalars
    if(c == '|')
        return value = readBlockScalar(), SCALAR;
    if(c == '>')
        return value = readBlockScalar(), SCALAR;
    if(c == '"')
        return value = readQuotedString(c, true), SCALAR;
    if(c == '\'')
        return value = readQuotedString(c, false), SCALAR;
    if(c == '*')
        return value = readAnchorName(), ALIAS;
    
    // node properties
    if(c == '&')
        return value = readAnchorName(), ANCHOR;
    if(c == '!')
        return value = readTagName(), TAG;
    
    // others
    return value = readPlainScalar(), SCALAR;
}

Token Lexer::getToken()
{
	return token;
}

std::string Lexer::getValue()
{
	return value;
}

int Lexer::getIndentation()
{
    return column;
}

void Lexer::skipComments(bool multiline)
{
    if(multiline) // c style only
    {
        stream.get();
        do
        {
            stream.ignore(std::numeric_limits<std::streamsize>::max(),'*');
        } while(!stream.eof() && stream.peek()!='/');
        stream.get();
        // TODO : process new lines
    }
	else
    {
        stream.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
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

// error handling:
//     - only header and comment on first line
std::string Lexer::readBlockScalar()
{
    // TODO
    // read header
    // possible comment + new line
    // possible error handling: allow content on first line
    // read content text
    return std::string();
}

// on entre apres : endChar
// on sort avec : endChar
std::string Lexer::readQuotedString(char endChar, bool escape)
{
    std::string hex; // special (utf8 & hexa)
    std::string result;
    for(char c = stream.get(); c!=endChar && !stream.eof();)
    {
        if(escape && c=='\\')
        {
            unsigned int utfNum = 0; // unicodes points
            int nbChar = 2;

            c = stream.get();
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
                case 'U':   nbChar *= 2; // \Unnnnnnnn (UTF-32)
                case 'u':   nbChar *= 2; // \unnnn     (UTF-16)
            // YAML escape sequences
                case 'x':                // \xnn       (UTF-8)
                    for(int i=0; i<nbChar; i++) {
                        if(!stream.eof() && isxdigit(stream.peek())) {
                            c = stream.get();
                            c -= (c<='9')? '0' : (c<='Z')? ('A'-10) : ('a'-10);
                            utfNum = (utfNum << 4) | c;
                        }
                        else break;
                    }
                    goto l_convert;
				case 'N':   utfNum = 0x0085; goto l_convert;
				case '_':   utfNum = 0x00a0; goto l_convert;
				case 'L':   utfNum = 0x2028; goto l_convert;
				case 'P':   utfNum = 0x2029; goto l_convert;
                l_convert:
                    hex = utf8Convert(utfNum);
                    break;
            // line folding
                case '\r':
                    c = '\n';
                    if(stream.peek()=='\n')
                        stream.get();
                case '\n':
                    // TODO: consume indentation
                default:
                    break;
            }
        }

        // line folding & indentation
        if(c == '\r') {
            c = '\n';
            if(stream.peek() == '\n')
                stream.get();
        }
        if(c == '\n')
            ; // TODO

        // append the char
        if(hex.size()) {
            result.append(hex);
            hex.clear();
        }
        else if(!stream.eof())
            result.push_back(c);
        
        c = stream.get();
        // single quoted escape
        if(c==endChar && !escape && stream.peek()==endChar)
        {
            result.push_back(c);
            stream.get();
            c = ' ';
        }
    }
    return result;
}

std::string Lexer::readAnchorName()
{
    // TODO
    // break with space, '\n', ',', '[', ']', '{', '}'
    return std::string();
}

// error handling:
//     - no comments on multilines
//     - one key on a line (no compact notation)
//     - allways expect a single key or terminate by end of line
std::string Lexer::readPlainScalar()
{
    // TODO
    // break with ": ", " #" or unindent
    return std::string();
}

std::string Lexer::readTagName()
{
    // TODO
    return std::string();
}


