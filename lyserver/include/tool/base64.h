/*
 * @Author: ly ly1285620755@163.com
 * @Date: 2024-02-09 00:36:23
 * @LastEditors: ly ly1285620755@163.com
 * @LastEditTime: 2024-02-09 00:40:56
 * @FilePath: /lyserver_master/lyserver/include/tool/base64.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef BASE64_H
#define BASE64_H
#include <string>
namespace lyserver
{
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    static inline bool is_base64(const char c)
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    std::string base64_encode(const char *bytes_to_encode, unsigned int in_len);

    std::string base64_decode(std::string const &encoded_string);
}

#endif // BASE64_H