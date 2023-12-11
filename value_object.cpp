#include "value_object.h"

Value_Object::Value_Object() {}
Value_Object::~Value_Object() {}

Value_String::Value_String(const char *p, size_t len)
{
    value.append(p, len);
}
Value_String::~Value_String() {}

Value_Num::Value_Num(double value_) : value(value_) {}
Value_Num::~Value_Num() {}

Value_Len::Value_Len(int32_t value_) : value(value_) {}
Value_Len::~Value_Len() {}

Value_0x01::Value_0x01(char value_) : value(value_) {}
Value_0x01::~Value_0x01() {}