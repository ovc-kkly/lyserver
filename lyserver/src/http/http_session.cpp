#include "http_session.h"
#include "http_parser.h"

namespace lyserver
{
    namespace http
    {

        HttpSession::HttpSession(Socket::ptr sock, bool owner)
            : SocketStream(sock, owner)
        {
        }

        HttpRequest::ptr HttpSession::recvRequest(int& ret)
        {
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
            // uint64_t buff_size = 100;
            std::shared_ptr<char> buffer(
                new char[buff_size], [](char *ptr)
                { delete[] ptr; });
            char *data = buffer.get();
            int offset = 0;//未解析数据的长度，就是请求体
            //读http请求内容，解析请求行和请求头保存在parser的HttpRequest对象
            do
            {
                int len = read(data + offset, buff_size - offset);
                ret = len;
                if (len <= 0)
                {
                    // close();
                    
                    return nullptr;
                }
                len += offset;
                size_t nparse = parser->execute(data, len);//解析请求行，请求头，剩余请求体
                if (parser->hasError())
                {
                    // close();
                    ret = -1;
                    return nullptr;
                }
                offset = len - nparse;
                if (offset == (int)buff_size)
                {
                    // close();
                    ret = -1;
                    return nullptr;
                }
                if (parser->isFinished())
                {
                    break;
                }
            } while (true);
            //把请求体拷贝到body,然后设置到parser的HttpRequest对象
            int64_t length = parser->getContentLength();
            if (length > 0)
            {
                std::string body;
                body.resize(length);

                int len = 0;
                if (length >= offset)
                {
                    memcpy(&body[0], data, offset);
                    len = offset;
                }
                else
                {
                    memcpy(&body[0], data, length);
                    len = length;
                }
                length -= offset;
                if (length > 0)
                {
                    ret = readFixSize(&body[len], length);
                    if (ret <= 0)
                    {
                        // close();
                        ret = -1;
                        return nullptr;
                    }
                }
                parser->getData()->setBody(body);
            }

            parser->getData()->init();
            return parser->getData();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp)
        {
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }

    }
}
