#include "Reader.hpp"
#include <stdexcept>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <limits>

void Reader::parseFile(Variant &result, std::string file)
{
    std::ifstream strm(file.c_str());
    if(!strm.is_open())
		throw std::invalid_argument("Reader::parseFile : Cannot open file");
    Reader reader(&strm);
    reader.parse(result);
}

void Reader::parseString(Variant &result, std::string text)
{
    std::istringstream strm(text);
    Reader reader(&strm);
    reader.parse(result);
}


//******************************** Constructors *******************************//
Reader::Reader(std::istream* input)
{
    comment = '\0';
    ifs = 0;
    if(!input->good())
        throw std::logic_error("Reader::Reader : stream error");
    ifs = input;
}


//****************************** Public functions *******************************//
void Reader::setStream(std::istream* input)
{
    ifs = 0;
    if(!input->good())
        throw std::logic_error("Reader::Reader : stream error");
    ifs = input;
}

void Reader::parse(Variant &result)
{
    if(ifs == 0 || ifs->fail())
        throw std::logic_error("Reader::Reader : stream error");
    *ifs >> std::noskipws;

    result.createMap();
    std::string key;
    for(nextChar(); !ifs->eof(); nextChar())
    {
        readKey(key);
        if(charBuf==',' || charBuf==';' || charBuf=='}')
        {
            nextChar();
            continue;
        }
        readValue(&(result.insert(key,0)));
    }
}


//****************************** Private functions *******************************//
char Reader::nextChar()
{
    charBuf = ifs->get();
    if(ifs->eof())
    {
        charBuf = ' ';
    }
    else if(std::isspace(charBuf))
    {
        *ifs >> std::ws;
        charBuf = ' ';
    }
    else if(charBuf == '/')
    {
        switch(ifs->peek())
        {
            case '/':
                ifs->ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                charBuf = ' ';
                break;
            case '*':
                ifs->get();
                do
                {
                    ifs->ignore(std::numeric_limits<std::streamsize>::max(),'*');
                } while(!ifs->eof() && ifs->peek()!='/');
                ifs->get();
                charBuf = ' ';
                break;
            default:
                break;
        }
    }
    else if(comment != '\0' && charBuf == comment)
    {
        ifs->ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        charBuf = ' ';
    }
    return charBuf;
}

bool Reader::goodChar(char c) const
{
    return std::isalnum(c) || c == '_' || c=='.' || c=='+' || c=='-';
}

void Reader::readMap(Variant* vmap)
{
    std::string key;
    Variant* v = 0;
    nextChar();
    vmap->createMap();
    do
    {
        readKey(key);
        if(charBuf=='}' || ifs->eof())
        {
            nextChar();
            break;
        }
        if(charBuf==',' || charBuf==';')
        {
            nextChar();
            v = 0;
            continue;
        }
        v = &(vmap->insert(key,0));
    } while(readValue(v)); // on sort avec le caractere apres : ]}
}

void Reader::readArray(Variant* varray)
{
    Variant* v = 0;
    nextChar();
    varray->createArray();
    do
    {
        v = &(varray->insert(0));
    } while(readValue(v)); // on sort avec le caractere apres : ]}
}

// on sort avec le caractere apres : =:
// ou avec le caractere chelou si exception
void Reader::readKey(std::string& key)
{
    key.clear();
    for( ;charBuf!=':' && charBuf!='=' &&
          charBuf!=',' && charBuf!=';' &&
          charBuf!='[' && charBuf!='{' &&
          charBuf!='}' && !ifs->eof(); nextChar())
    {
        if(charBuf=='\"' || charBuf=='\'')
            key = readString(charBuf,charBuf=='\"');
        else if(goodChar(charBuf))
            key.push_back(charBuf);
        else if(charBuf!=' ')
            nbErrors++;
    }

    if(charBuf==':' || charBuf=='=')
        nextChar();
    if(ifs->eof())
        key = "";
}

bool Reader::readValue(Variant* exp)
{
    if(exp==0)
        return true;
    exp->setToNull();
    std::string str;
    bool isString = false;

	if(charBuf == '[')
	{
		readArray(exp);
		return true;
	}
	if(charBuf == '{')
	{
		readMap(exp);
		return true;
	}

    for( ;charBuf!=',' && charBuf!=';' &&
          charBuf!='}' && charBuf!=']' &&
          !ifs->eof(); nextChar())
    {
        if(goodChar(charBuf))
            str.push_back(charBuf);
        else switch(charBuf)
        {
            case '\"':
            case '\'':
                str.append(readString(charBuf,charBuf=='\"'));
                isString = true;
				break;
            default:
                if(charBuf!=' ')
                    nbErrors++;
        }
    }

    // TODO:
    // traiter la string
    if(!isString && (isdigit(str[0]) || str[0]=='+' || str[0]=='-' || str[0]=='.'))
        readNumber(exp,str);
    else
    {
        if(str.empty() || str == "null")
            exp->setToNull();
        else if(str == "true")
            *exp = true;
        else if(str == "false")
            *exp = false;
        else
            *exp = str;
    }

    if(charBuf==']' || charBuf=='}')
    {
        nextChar();
        return false;
    }
    else
    {
        nextChar();
        return true;
    }
}

std::string utf8Convert(unsigned int hex)
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
std::string Reader::readString(char endChar, bool escape) const
{
    std::string result;
    for(char c = ifs->get(); c!=endChar && !ifs->eof(); c = ifs->get())
    {
        if(escape && c=='\\')
        {
            std::string hex; // nombres
            int nbChar = 0;

            c = ifs->get();
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
                        c = ifs->get();
                        if(!ifs->eof() && isxdigit(c))
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
        if(!ifs->eof())
            result.push_back(c);
    }
    return result;
}

void sconvert(Variant* var, std::string& value, int base=10, bool isFloat=false)
{
    if(isFloat)
    {
        double num = atof(value.c_str());
		if(num>=std::numeric_limits<float>::min() && num<=std::numeric_limits<float>::max())
            *var = static_cast<float>(num);
        else
			*var = num;
    }
    else
    {
        long long num = strtoll(value.c_str(),0,base);
        if(num>=std::numeric_limits<int>::min() && num<=std::numeric_limits<int>::max())
            *var = static_cast<int>(num);
        else
            *var = num;
    }
}

void Reader::readNumber(Variant* num, std::string& str) const
{
    if(str.size()>3 && str[0]=='0' && str[1]=='x') // hexadecimal
    {
        str.erase(0,2);
        sconvert(num,str,16);
    }
    else if(str.size()>3 && str[0]=='0' && str[1]=='b') // binary
    {
        str.erase(0,2);
        sconvert(num,str,2);
    }
    else // else a decimal number
    {
        unsigned int i = 0;
        bool isFloat = false;
        if(str[0]=='.')
            isFloat = true;
        for(i++; (std::isdigit(str[i]) || str[i]=='.' || str[i]=='e' || str[i]=='E')
            && i<str.size() && !isFloat; i++)
        {
            if(str[i]=='e' || str[i]=='E' || str[i]=='.')
                isFloat = true;
        }
        if(isFloat || (i<str.size() && str[i]=='f'))
        {
            if(str[i]=='f')
                *num = static_cast<float>(atof(str.c_str()));
            sconvert(num,str,10,true);
        }
        else
        {
            sconvert(num,str);
        }
    }
}

