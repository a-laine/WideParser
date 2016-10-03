#ifndef READER_H
#define READER_H

#include "Variant.hpp"
#include <istream>
#include <string>

/*! \brief Class providing an interface to read a JSON input.
 *
 * Parsing methods construct a Variant object containing the data of the stream in a similar structure.
 * \see Variant, Writer
 */
class Reader
{
    public:
        //**********************************************************************************************//
        //**************************************  Static members  **************************************//
        //**********************************************************************************************//
        /*! \brief Read a JSON file and extract data.
         *
         * All the elements of the file are placed in a Variant objet.
         * \param result A Variant object containing all the data.
         * \param file The JSON file name.
         * \throw std::invalid_argument is thown if the the file cannot be opened.
         */
        static void parseFile(Variant &result, std::string file);

        /*! \brief Read a string with a JSON structure and extract data.
         *
         * All the elements of the string are placed in a Variant objet.
         * \param result A Variant object containing all the data.
         * \param text The input string containig the data.
         */
        static void parseString(Variant &result, std::string text);


        //**********************************************************************************************//
        //**************************************  Public methods  **************************************//
        //**********************************************************************************************//
        /*! \brief Construct a Reader object with the specified input stream.
         *
         * The Reader object will use the input stream to extract JSON data.
         * The parsing operation can be performed using the parse method.
         * \note The stream is supposed to be ready to read. Otherwise, an exception can be thrown.
         * \param input The input stream to read.
         * \throw std::logic_error is thrown if the stream is not good.
         */
        Reader(std::istream* input);

        /*! \brief Modify the internal input stream.
         *
         * The Reader object will use the input stream to extract JSON data
         * insteed of the one set in the constructor.
         * \param input The input stream to read.
         * \throw std::logic_error is thrown if the stream is not good.
         */
        void setStream(std::istream* input);

        /*! \brief Read the internal input stream and extract data.
         *
         * All the elements of the stream are placed in a Variant objet.
         * \param result A Variant object containing all the data.
         * \throw std::logic_error is thrown if the stream is not good.
         */
        void parse(Variant &result);




    private:
        std::istream* ifs;  //!< The input stream to read.
        int nbErrors;       //!< Number of syntax errors found. Not used yet.
        char charBuf;       //!< A buffer containing the character read.
        char comment;       //!< The comment caracter.


        /*! Read the next character from the stream.
         *  It is placed in the buffer charBuf and returned.
         */
        char nextChar();

        /*! Check if the character is a valid character for a non literal string.
         *  Valid character are: <pre> _ + . - </pre> and all alphanumerical character.
         */
        bool goodChar(char c) const;

        /*! Read the next key sequence of a map from the stream.
         *  Ends the read after the symbol '=' or ':' or any other in case of an invalid syntax.
         */
        void readKey(std::string& key);

        /*! Read the next expression from the stream (a map, an array, or another value).
         *  Ends the read after one of theses symbols: <pre> , ; ] } </pre>.
         */
        bool readValue(Variant* exp);

        /*! Read the differents values of a map from the stream.
         *  Ends the read after one of theses symbols: <pre> ] } </pre>.
         */
        void readMap(Variant* vmap);

        /*! Read the differents values of an array from the stream.
         *  Ends the read after one of theses symbols: <pre> ] } </pre>.
         */
        void readArray(Variant* varray);

        /*! Read a string literal from the stream.
         *  Ends the read after one the character _endChar_.
         *  If escape is set to true, escape sequence are converted to their spacial meaning.
         */
        std::string readString(char endChar, bool escape) const;

        /*! Convert the string _str_ in a numerical value placed in _num_ with the best type.
         */
        void readNumber(Variant* num, std::string& str) const;
};

#endif // READER_H
