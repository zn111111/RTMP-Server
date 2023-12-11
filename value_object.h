#ifndef __VALUE_OBJECT_H__
#define __VALUE_OBJECT_H__

#include <cstdint>
#include <string>

struct Value_Object
{
    Value_Object();
    virtual ~Value_Object();
};

struct Value_String : Value_Object
{
    Value_String(const char *p, size_t len);
    ~Value_String();

    std::string value;
};

struct Value_Num : Value_Object
{
    Value_Num(double value_);
    ~Value_Num();

    double value;
};

struct Value_Len : Value_Object
{
    Value_Len(int32_t value_);
    ~Value_Len();

    int32_t value;
};

struct Value_0x01 : Value_Object
{
    Value_0x01(char value_);
    ~Value_0x01();
    char value;
};

#endif