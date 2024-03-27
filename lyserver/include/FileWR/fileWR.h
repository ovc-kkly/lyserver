#ifndef FILEWR_H
#define FILEWR_H

#include <fstream>

#include <iostream>
#include <sstream>
#include <string>
#include <memory>
namespace lyserver
{

    class FileReader
    {
    public:
        typedef std::shared_ptr<FileReader> ptr;
        // 构造函数，接受文件名作为参数
        explicit FileReader(const std::string &fileName) : fileName_(fileName) {}

        // 读取文件内容
        bool readFile();

        // 获取文件内容
        const std::string &getContent() const
        {
            return fileContent_;
        }

    private:
        std::string fileName_;    // 文件名
        std::string fileContent_; // 文件内容
    };
}
#endif