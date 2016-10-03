#ifndef VARIANT_H
#define VARIANT_H

#include <string>
#include <map>
#include <deque>

/*
class _Variant_iterator
{
};
*/



/*! \brief Class which can contain multiple type of variable.
 *
 * This class is a generic handler for all common type of variable, and especially the ones usable in the JSON format.
 * It contains internally a single value of these type that can be read and modified. This class permit to construct
 * many descriptive object and tree based or list based data.
 *
 * Use of a Variant :
 * -------------------
 *
 * To use a Variant there are multiple ways.
 * Many accessors have been created to use it very easly. They are explained bellow.
 *
 * For a primitive type (all except arrays and maps):<br>
 * The data into the object can be set using one of the [constructors](@ref Variant::Variant), the overloded
 * [assignment operator](@ref Variant::operator=) or the function setToNull(). To read the data, you must use one of the
 * conversion methods with the following name template: to<em>type</em>(). Exemple: toBool(), toInt(), toDouble()<br>
 * If you want to know what type contains the object to use the correct function, you can use the getType() function.<br>
 * You can also check if the Variant is set to null with isNull().
 *
 * For a container type (array and map):<br>
 * These type are a bit more complicated. To create a container type, you must use first one of the [create methods](@ref createArray).
 * Then you can use the chosen container in the Variant.<br>
 * To use it, you can get the internal container with one of the [get methods](@ref getArray), but it is recomended
 * to use the pre implemented access methods. The following operations are aviable: [inserting an element](@ref insert),
 * [obtaining the size](@ref size) and the overloaded [access operators](@ref Variant::operator[]).
 *
 * Exemple :
 * ----------
 *
 * Here is an exemple of creating and using a Variant representing complex data.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * try{
 *     Variant v;
 *
 *     // Creation of the Variant v
 *     v.createMap();
 *     v.insert("var1",true);
 *     v.insert("var2",110);
 *     v.insert("var3",3.141592);
 *
 *     v.insert("class").createMap();
 *     v["class"].insert("foo1","bar");
 *
 *     v.insert("array").createArray();
 *     v["array"].insert(1);
 *     v["array"].insert(2);
 *     v["array"].insert("end");
 *
 *
 *     // Getting previously inserted values
 *     if(!v["var1"] || !v["var2"] || !v["var3"])
 *     {
 *         std::cout << "error : empty object" << std::endl;
 *         exit(0);
 *     }
 *     bool   var1 = v["var1"].toBool();
 *     int    var2 = v["var2"].toInt();
 *     double var3 = v["var3"].toDouble();
 *
 *     std::string foo1 = v["class"]["foo1"].toString();
 *
 *     int tab1 = v["array"][1].toInt();
 *     int tab2 = v["array"][2].toInt();
 *     int tab4 = v["array"][4].toString();
 *
 * } catch(std::exception &e)
 * {
 *     std::cout << "Error: " << e.what() << std::endl;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */
class Variant
{
    public:
        /*! \brief Enumeration to specify all the type that a Variant can contains.
         * \see getType()
         */
        enum VariantType {
            BOOL,       //!< Represent a boolean
            CHAR,       //!< Represent an ascii character
            INT,        //!< Represent an integer
            UINT,       //!< Represent an unsigned integer
            LONG,       //!< Represent a long integer (64 bits)
            ULONG,      //!< Represent an unsigned long integer (64 bits)
            FLOAT,      //!< Represent a float
            DOUBLE,     //!< Represent a double
            STRING,     //!< Represent a string

            SEQUENCE,   //!< Represent a sequence of elements
            MAP,        //!< Represent a list of key/value pair

            SCALAR,     //!< Any non-recursive type
            NULLTYPE,   //!< Special type representing a _null_ value.
            UNDEFINED   //!< The node doesn't exist
        };

        typedef std::map<std::string,Variant> MapType;  //!< A typedef for the type of key/value map used
        typedef std::deque<Variant> ArrayType;          //!< A typedef for the type of dynamic array used

        //**********************************************************************************************//
        //********************************  Constructors / Destructors  ********************************//
        //**********************************************************************************************//
        /*! \brief x
         *
         */
        Variant();

        /*! \brief x
         *
         */
        Variant(const Variant &v);

        /*! \brief x
         *
         */
        Variant(Variant &&v);

        /*! \brief x
         *
         */
        Variant(const bool var);

        /*! \brief x
         *
         */
        Variant(const char var);

        /*! \brief x
         *
         */
        Variant(const int var);

        /*! \brief x
         *
         */
        Variant(const long long var);

        /*! \brief x
         *
         */
        Variant(const float var);

        /*! \brief x
         *
         */
        Variant(const double var);

        /*! \brief x
         *
         */
        Variant(const std::string var);

        /*! \brief x
         *
         */
        Variant(const char* var);

        /*! \brief x
         *
         */
        Variant(const ArrayType var);

        /*! \brief x
         *
         */
        Variant(const MapType var);

        /*! \brief x
         *
         */
        virtual ~Variant();

        //**********************************************************************************************//
        //**************************************  Type Accessors  **************************************//
        //**********************************************************************************************//
        /*! \brief x
         *
         */
        VariantType getType() const;

        /*! \brief x
         *
         */
        bool isNull() const;

        /*! \brief x
         *
         */
        bool defined() const;

        /*! \brief x
         *
         */
        bool operator!() const;

        //**********************************************************************************************//
        //**************************************  Data Accessors  **************************************//
        //**********************************************************************************************//
        /*! \brief x
         *
         */
        bool toBool() const;

        /*! \brief x
         *
         */
        char toChar() const;

        /*! \brief x
         *
         */
        long long toLong() const;

        /*! \brief x
         *
         */
        int toInt() const;

        /*! \brief x
         *
         */
        double toDouble() const;

        /*! \brief x
         *
         */
        float toFloat() const;

        /*! \brief x
         *
         */
        std::string toString() const;


        /*! \brief x
         *
         */
        Variant& operator[] (const size_t key);

        /*! \brief x
         *
         */
        const Variant& operator[] (const size_t key) const;

        /*! \brief x
         *
         */
        Variant& operator[] (const std::string key);

        /*! \brief x
         *
         */
        const Variant& operator[] (const std::string key) const;

        /*! \brief x
         *
         */
        ArrayType& getArray() const;

        /*! \brief x
         *
         */
        MapType& getMap() const;


        /*! \brief x
         *
         */
        size_t size() const;




        //**********************************************************************************************//
        //****************************************  Modifiers  *****************************************//
        //**********************************************************************************************//
        /*! \brief x
         *
         */
        void setToNull();


        /*! \brief x
         *
         */
        Variant& operator= (const Variant &v);

        /*! \brief x
         *
         */
        Variant& operator= (Variant &&v);

        /*! \brief x
         *
         */
        Variant& operator= (const bool var);

        /*! \brief x
         *
         */
        Variant& operator= (const char var);

        /*! \brief x
         *
         */
        Variant& operator= (const int var);

        /*! \brief x
         *
         */
        Variant& operator= (const long long var);

        /*! \brief x
         *
         */
        Variant& operator= (const float var);

        /*! \brief x
         *
         */
        Variant& operator= (const double var);

        /*! \brief x
         *
         */
        Variant& operator= (const std::string var);

        /*! \brief x
         *
         */
        Variant& operator= (const ArrayType var);

        /*! \brief x
         *
         */
        Variant& operator= (const MapType var);


        /*! \brief x
         *
         */
        Variant& createArray();

        /*! \brief x
         *
         */
        Variant& createMap();


        /*! \brief x
         *
         */
        Variant& insert(const Variant val);

        /*! \brief x
         *
         */
        Variant& insert(const std::string key, const Variant val);




    private:
        /*! The value of the object.
         *  The Variant can hold just one value at the same time.
         */
        typedef union
        {
            bool Bool;
            int Int;
            long long Long;
            float Float;
            double Double;
            std::string* String;
            ArrayType* Array;
            MapType* Map;
        } Var;

        Var value;
		VariantType type;   //!< The type of the value saved.
};

#endif // VARIANT_H
