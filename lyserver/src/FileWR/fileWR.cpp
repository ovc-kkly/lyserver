#include "fileWR.h"
#include "log.h"
#include <system_error>
namespace lyserver
{
     static Logger::ptr g_logger = LY_LOG_NAME("system");
    bool FileReader::readFile()
    {
        std::ifstream file(fileName_);
        if (!file.is_open()) {
            // 文件打开失败
            // std::error_code ec(file.rdstate());
            LY_LOG_ERROR(g_logger) << "Failed to open file: " << fileName_ << " : " << strerror(errno);

            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf(); // 读取文件内容到stringstream
        fileContent_ = buffer.str(); // 将stringstream的内容转换为string

        file.close(); // 关闭文件
        return true;
    }
    
}