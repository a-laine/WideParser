#include <iostream>
#include "Reader.hpp"
#include "Writer.hpp"
#include <stdexcept>

using namespace std;

/*
// test arborescence
int main(int argc, char** argv)
{
    std::cout << "---------------------" << std::endl;
    try{
        Variant v;
        Reader::parseFile(v,"tests/fic.txt");
        std::cout << "bla2 = " << v["bla2"].toString() << std::endl;
        std::cout << "class.bla1 = " << v["class"]["bla1"].toString() << std::endl;
        std::cout << "class.bla2 = " << v["class"]["bla2"].toString() << std::endl;
        std::cout << "class.bla3 = " << v["class"]["bla3"].toString() << std::endl;
        std::cout << "class.class.bla1 = " << v["class"]["class"]["bla1"].toString() << std::endl;
        std::cout << "tab[0] = " << v["tab"][0].toString() << std::endl;
        std::cout << "tab[1] = " << v["tab"][1].toString() << std::endl;
        std::cout << "tab[2] = " << v["tab"][2].toString() << std::endl;
        std::cout << "tab[3] = " << v["tab"][3].toString() << std::endl;
    } catch(std::exception &e)
    {
        std::cout << "---------------------" << std::endl;
        std::cout << "Exception lancee:" << std::endl;
        std::cout << e.what() << std::endl;
    }
    std::cout << "---------------------" << std::endl;
    return 0;
}*/

/*
// test types
int main(int argc, char** argv)
{
    std::cout << "---------------------" << std::endl;
    try{
        Variant v;
        Reader::parseFile(v,"tests/fic2.txt");
        std::cout << "num1 = " << v["num1"].toInt() << std::endl;
        std::cout << "num2 = " << v["num2"].toInt() << std::endl;
        std::cout << "num3 = " << v["num3"].toLong() << std::endl;
        std::cout << "num4 = " << v["num4"].toLong() << std::endl;
        std::cout << "num5 = " << v["num5"].toInt() << std::endl;
        std::cout << "num6 = " << v["num6"].toInt() << std::endl;
        std::cout << "num7 = " << v["num7"].toDouble() << std::endl;
        std::cout << "num8 = " << v["num8"].toDouble() << std::endl;
        std::cout << "num9 = " << v["num9"].toDouble() << std::endl;
        std::cout << "num10 = " << v["num10"].toDouble() << std::endl;
        std::cout << "num11 = " << v["num11"].toDouble() << std::endl;
        std::cout << "num12 = " << v["num12"].toDouble() << std::endl;
        std::cout << "num13 = " << v["num13"].toDouble() << std::endl;
        std::cout << "num14 = " << v["num14"].toDouble() << std::endl;
        std::cout << "num15 = " << v["num15"].toDouble() << std::endl;
        std::cout << "var1 = " << v["var1"].toBool() << std::endl;
        std::cout << "var2 = " << v["var2"].toBool() << std::endl;
        std::cout << "var3 = " << v["var3"].isNull() << std::endl;
        std::cout << "var4 = " << v["var4"].toString() << std::endl;
    } catch(std::exception &e)
    {
        std::cout << "---------------------" << std::endl;
        std::cout << "Exception lancee:" << std::endl;
        std::cout << e.what() << std::endl;
    }
    std::cout << "---------------------" << std::endl;
    return 0;
}*/

/*
// test Writer
int main(int argc, char** argv)
{
    std::cout << "---------------------" << std::endl;
    try{
        Variant v;
        v.createMap();
        v.insert("var1",true);
        v.insert("var2");
        v.insert("var3",110);
        v.insert("var4",3.141592);
        v.insert("class").createMap();
        v["class"].insert("bla1","bla");
        v["class"].insert("class").createMap();
        v["class"]["class"].insert("bla","bla");
        v["class"].insert("bla2","bla");
        v.insert("tab").createArray();
        v["tab"].insert(1);
        v["tab"].insert(2);
        v["tab"].insert(3);
        v["tab"].insert(4);
        v["tab"].insert("fin");
        std::cout << Writer::writeInString(v) << std::endl;
    } catch(std::exception &e)
    {
        std::cout << "---------------------" << std::endl;
        std::cout << "Exception lancee:" << std::endl;
        std::cout << e.what() << std::endl;
    }
    std::cout << "---------------------" << std::endl;
    return 0;
}
*/



#include <iostream>
#include <fstream>
#include <stack>
#include "Lexer.hpp"


// test Lexer
int main(int argc, char const *argv[])
{
	for(int i = 1; i < argc; i++)
	{
		std::ifstream ifs(argv[i]);
		if(!ifs)
			continue;

		Lexer lex(ifs);
		bool loop = true;
		size_t indent = 0;
		std::stack<int> indentStack;
		indentStack.push(0);
		Lexer::TokenInfo t = Lexer::TokenInfo({ 0, END_STREAM });
		Lexer::TokenInfo prevT;
		while(loop)
		{
			prevT = t;
            t = lex.next(indent);
			switch(t.token)
			{
				case END_STREAM:
					loop = false;
					break;
				case BLOCK_MAP_ENTRY:
					std::cout << "BLOCK_MAP_ENTRY : " << t.indent << std::endl;
					indentStack.push(t.indent + 1);
					break;
				case BLOCK_SEQ_ENTRY:
					std::cout << "BLOCK_SEQ_ENTRY : " << t.indent << std::endl;
					indentStack.push(t.indent + 1);
					break;
				case MAP_KEY_DELIMITER:
					std::cout << "MAP_KEY_DELIMITER : " << t.indent << std::endl;
					indentStack.push(prevT.indent + 1);
					break;
				case FLOW_MAP_BEGIN:
					std::cout << "FLOW_MAP_BEGIN : " << t.indent << std::endl;
					indentStack.push(t.indent + 1);
					break;
				case FLOW_MAP_END:
					std::cout << "FLOW_MAP_END : " << t.indent << std::endl;
					indentStack.pop();
					break;
				case FLOW_SEQ_BEGIN:
					std::cout << "FLOW_SEQ_BEGIN : " << t.indent << std::endl;
					indentStack.push(t.indent + 1);
					break;
				case FLOW_SEQ_END:
					std::cout << "FLOW_SEQ_END : " << t.indent << std::endl;
					indentStack.pop();
					break;
				case FLOW_DELIMITER:
					std::cout << "FLOW_DELIMITER : " << t.indent << std::endl;
					break;
				case SCALAR:
					std::cout << "SCALAR : " << t.indent << std::endl;
					std::cout << "    " << indentStack.top() << " - " << lex.getValue() << std::endl;
					if(prevT.token != SCALAR && prevT.token != ALIAS && prevT.token != END_STREAM)
					{
						indentStack.pop();
					}
					break;
				case ALIAS:
					std::cout << "ALIAS : " << t.indent << std::endl;
					std::cout << "    " << indentStack.top() << " - " << lex.getValue() << std::endl;
					break;
				case ANCHOR:
					std::cout << "ANCHOR : " << t.indent << std::endl;
					std::cout << "    " << indentStack.top() << " - " << lex.getValue() << std::endl;
					break;
				case TAG:
					std::cout << "TAG" << t.indent << std::endl;
					std::cout << "    " << indentStack.top() << " - " << lex.getValue() << std::endl;
					break;
				default:
					std::cout << "error" << std::endl;
					break;
			}
		}
	}
	return 0;
}

