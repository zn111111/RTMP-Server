#ifndef __IO_BUFFER_H__
#define __IO_BUFFER_H__

#include <string>

class IO_Buffer
{
public:
    IO_Buffer();
    ~IO_Buffer();

public:
    const char *get_data();
    void push_back(const char *data, size_t len);
    void clear();
    size_t size() const;
    void erase(size_t pos, size_t len);
    size_t find(char target, size_t pos);
    void print_buffer() const;

private:
    std::string buffer;
};

#endif