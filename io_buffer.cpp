#include "io_buffer.h"

IO_Buffer::IO_Buffer() {}
IO_Buffer::~IO_Buffer() {}

const char *IO_Buffer::get_data()
{
    return buffer.data();
}

void IO_Buffer::push_back(const char *data, size_t len)
{
    buffer.append(data, len);
}

void IO_Buffer::clear()
{
    buffer.clear();
}

size_t IO_Buffer::size() const
{
    return buffer.size();
}

void IO_Buffer::erase(size_t pos, size_t len)
{
    buffer.erase(pos, len);
}

size_t IO_Buffer::find(char target, size_t pos)
{
    return buffer.find(target, pos);
}

void IO_Buffer::print_buffer() const
{
    // printf("buffer length: %ld\n", buffer.size());
    for (char c : buffer)
    {
        printf("%x ", (unsigned char)c);
    }
    printf("\n");
}