#ifndef BUFFER_H
#define BUFFER_H
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <string>
#include <cstdlib>
#include <memory>

namespace lyserver
{
    class Buffer
    {
    private:
        std::string buf;

    public:
        typedef std::shared_ptr<Buffer> ptr;
        Buffer();
        ~Buffer();
        void append(const char *_str, int _size);
        ssize_t size();
        const char *c_str();
        std::string get_str();
        // string c4_str();
        void clear();
        void getline();
    };
}
#endif