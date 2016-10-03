#include "Variant.hpp"
#include <stdexcept>

//********************************************----------------------------*********************************************//
//******************************************** constructors / destructors *********************************************//
//********************************************----------------------------*********************************************//
Variant::~Variant()
{
    setToNull();
}

Variant::Variant() : type(Variant::NULLTYPE) {
    value.Long=0; }

Variant::Variant(const Variant &v)
{
    switch(type)
    {
        case Variant::STRING:
            value.String = new std::string(*v.value.String);
            break;
        case Variant::SEQUENCE:
            value.Array = new ArrayType(*v.value.Array);
            break;
        case Variant::MAP:
            value.Map = new MapType(*v.value.Map);
            break;
        default:
            value = v.value;
    }
    type = v.getType();
}

Variant::Variant(Variant &&v)
{
    type = v.getType();
    value = v.value;

    v.type = Variant::NULLTYPE;
    v.value.Long = 0;
}

Variant::Variant(const bool var) : type(Variant::BOOL) {
    value.Bool=var; }

Variant::Variant(const char var) : type(Variant::CHAR) {
    value.Int=var; }

Variant::Variant(const int var) : type(Variant::INT) {
    value.Int=var; }

Variant::Variant(const long long var) : type(Variant::LONG) {
    value.Long=var; }

Variant::Variant(const float var) : type(Variant::FLOAT) {
    value.Float=var; }

Variant::Variant(const double var) : type(Variant::DOUBLE) {
    value.Double=var; }

Variant::Variant(const std::string var) : type(Variant::STRING) {
    value.String = new std::string(var); }

Variant::Variant(const char* var) : type(Variant::STRING) {
    value.String = new std::string(var); }

Variant::Variant(const ArrayType var) : type(Variant::SEQUENCE) {
    value.Array = new ArrayType(var); }

Variant::Variant(const MapType var) : type(Variant::MAP) {
    value.Map = new MapType(var); }



//********************************************----------------*********************************************//
//******************************************** type accessors *********************************************//
//********************************************----------------*********************************************//

Variant::VariantType Variant::getType() const {
    return type; }

bool Variant::isNull() const {
    return type==Variant::NULLTYPE; }

bool Variant::defined() const {
    return type!=Variant::UNDEFINED; }

bool Variant::operator!() const {
    return type==Variant::UNDEFINED; }


//********************************************----------------*********************************************//
//******************************************** data accessors *********************************************//
//********************************************----------------*********************************************//
bool Variant::toBool() const {
    return value.Bool; }

char Variant::toChar() const {
    return toInt(); }

long long Variant::toLong() const {
    return value.Long; }

int Variant::toInt() const {
    return value.Int; }

double Variant::toDouble() const {
    return value.Double; }

float Variant::toFloat() const {
    return value.Float; }

std::string Variant::toString() const
{
    if(type!=Variant::STRING)
		throw std::logic_error("Variant::toString : wrong type");
    return *value.String;
}


Variant& Variant::operator[] (const size_t key)
{
    if(type!=Variant::SEQUENCE)
        throw std::logic_error("Variant::operator[](size_t) : wrong type");
    return value.Array->at(key);
}

const Variant& Variant::operator[] (const size_t key) const
{
    Variant v();
    if(type!=Variant::SEQUENCE)
        throw std::logic_error("Variant::operator[](size_t) : wrong type");
    return value.Array->at(key);
}

Variant& Variant::operator[] (const std::string key)
{
    Variant v();
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::operator[](std::string) : wrong type");
    return value.Map->at(key);
}

const Variant& Variant::operator[] (const std::string key) const
{
    Variant v();
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::operator[](std::string) : wrong type");
    return value.Map->at(key);
}


Variant::ArrayType& Variant::getArray() const
{
	if(type!=Variant::SEQUENCE)
		throw std::logic_error("Variant::getArray : wrong type");
	return *value.Array;
}

Variant::MapType& Variant::getMap() const
{
	if(type!=Variant::MAP)
		throw std::logic_error("Variant::getMap : wrong type");
	return *value.Map;
}

size_t Variant::size() const
{
    switch(type)
    {
        case Variant::SEQUENCE:
            return value.Array->size();
            break;
        case Variant::MAP:
            return value.Map->size();
            break;
        default:
            return 0;
    }
}



//********************************************----------------*********************************************//
//******************************************** data modifiers *********************************************//
//********************************************----------------*********************************************//
void Variant::setToNull()
{
    switch(type)
    {
        case Variant::STRING:
            delete value.String;
            break;
        case Variant::SEQUENCE:
            delete value.Array;
            break;
        case Variant::MAP:
            delete value.Map;
            break;
        default:
            break;
    }
    type = Variant::NULLTYPE;
    value.Long = 0;
}

Variant& Variant::operator= (const Variant &v)
{
    setToNull();
    type = v.getType();
    switch(type)
    {
        case Variant::STRING:
            value.String = new std::string(*v.value.String);
            break;
        case Variant::SEQUENCE:
            value.Array = new ArrayType(*v.value.Array);
            break;
        case Variant::MAP:
            value.Map = new MapType(*v.value.Map);
            break;
        default:
            value = v.value;
    }
    return *this;
}

Variant& Variant::operator= (Variant &&v)
{
    type = v.getType();
    value = v.value;

    v.type = Variant::NULLTYPE;
    v.value.Long = 0;
    return *this;
}

Variant& Variant::operator= (const bool var)
{
    setToNull();
    type=Variant::BOOL; value.Bool=var;
    return *this;
}

Variant& Variant::operator= (const char var)
{
    setToNull();
    type=Variant::CHAR; value.Int=var;
    return *this;
}

Variant& Variant::operator= (const int var)
{
    setToNull();
    type=Variant::INT; value.Int=var;
    return *this;
}

Variant& Variant::operator= (const long long var)
{
    setToNull();
    type=Variant::LONG; value.Long=var;
    return *this;
}

Variant& Variant::operator= (const float var)
{
    setToNull();
    type=Variant::FLOAT; value.Float=var;
    return *this;
}

Variant& Variant::operator= (const double var)
{
    setToNull();
    type=Variant::DOUBLE; value.Double=var;
    return *this;
}

Variant& Variant::operator= (const std::string var)
{
    setToNull();
    type=Variant::STRING; value.String = new std::string(var);
    return *this;
}

Variant& Variant::operator= (const ArrayType var)
{
    setToNull();
    type=Variant::SEQUENCE; value.Array = new ArrayType(var);
    return *this;
}

Variant& Variant::operator= (const MapType var)
{
    setToNull();
    type=Variant::MAP; value.Map = new MapType(var);
    return *this;
}


Variant& Variant::createArray()
{
    setToNull();
    type=Variant::SEQUENCE;
    value.Array = new ArrayType();
    return *this;
}

Variant& Variant::createMap()
{
    setToNull();
    type=Variant::MAP;
    value.Map = new MapType();
    return *this;
}


Variant& Variant::insert(const Variant val)
{
    if(val.getType()==Variant::STRING && type==Variant::MAP)
    {
        (*value.Map)[val.toString()];
        return (*value.Map)[val.toString()];
    }
    else
    {
        if(type!=Variant::SEQUENCE)
            throw std::logic_error("Variant::insert(Variant&) : wrong type");
        value.Array->push_back(Variant()); // Don't remove this!!!!
        value.Array->back() = val; // Don't remove this!!!!
        return value.Array->back();
    }
}

Variant& Variant::insert(const std::string key, const Variant val)
{
    if(type!=Variant::MAP)
        throw std::logic_error("Variant::insert(string,Variant&) : wrong type");
    if(val.getType()==Variant::SEQUENCE || val.getType()==Variant::MAP)
    {
        (*value.Map)[key];
        (*value.Map)[key] = val;
    }
    else
        (*value.Map)[key] = val;
    return (*value.Map)[key];
}

