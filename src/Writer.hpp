#ifndef WRITER_H
#define WRITER_H

#include "Variant.hpp"
#include <ostream>
#include <string>

/*! \brief Class providing an interface to write a Variant object into a JSON syntax.
 *
 * The content of a Variant object is wrote in an output stream with a JSON like syntax.
 * \see Variant, Writer
 */
class Writer
{
    public:
        //**********************************************************************************************//
        //**************************************  Static members  **************************************//
        //**********************************************************************************************//
		/*! \brief Write in a file the data of a Variant object.
         *
         * Write all the data of _object_ in the specified file in a JSON like syntax.
         * \param object A Variant object containing all the data.
         * \param file The JSON file name.
         * \throw If the the file cannot be opened, an std::invalid_argument exception is thown.
         */
        static void writeInFile(Variant &object, std::string file);

        /*! \brief Write in a string the data of a Variant object.
         *
         * Write all the data of _object_ in a string in a JSON like syntax.
         * \param object A Variant object containing all the data.
         * \return The resulting string in a JSON syntax.
         */
        static std::string writeInString(Variant &object);


        //**********************************************************************************************//
        //**************************************  Public methods  **************************************//
        //**********************************************************************************************//
        /*! \brief Construct a Writer object with the specified output stream.
         *
         * The Writer object will use the output stream to write JSON data.
         * The operation can be performed using the write method.
         * \note The stream is supposed to be ready to write. Otherwise, an exception can be thrown.
         * \param output The output stream to write in.
         * \throw An std::logic_error is thrown if the stream is not good.
         */
        Writer(std::ostream* output);

        /*! \brief Modify the internal output stream.
         *
         * The Writer object will use the output stream to write JSON data
         * insteed of the one set in the constructor.
         * \param output The output stream to write in.
         * \throw An std::logic_error is thrown if the stream is not good.
         */
        void setStream(std::ostream* output);

        /*! \brief Write in the internal output stream the specified data.
         *
         * All the data to write must be placed in the Variant object argument.
         * \param object A Variant object containing all the data.
         * \throw An std::logic_error is thrown if the stream is not good.
         */
        void write(Variant &object);




    private:
        std::ostream* ostr; //!< The output stream to write in.
        bool json;          //!< Use a JSON syntax or a more personnal one
        char indent;        //!< The size of the indentation in number of space.


        /*! \brief Write the content of the variant in the output stream.
         *
         * \param var A Variant object containing the data to write.
         * \param decal The size of the last indentation in number of space.
         */
        void writeVariant(Variant& var,int decal=0) const;
};

#endif // WRITER_H
