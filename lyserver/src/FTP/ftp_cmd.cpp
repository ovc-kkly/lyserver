#include "ftp_cmd.h"

FTPRequestCommand StringToFTPRequestCmd(const std::string &m)
{
#define XX(num, name, param)            \
    if (strcmp(#name, m.c_str()) == 0)  \
    {                                   \
        return FTPRequestCommand::name; \
    }

    FTP_REQUEST_COMMAND(XX)
#undef XX
    return FTPRequestCommand::INVALID_CMD;
}

const char *FTPRequestCmdToString(const FTPRequestCommand &m)
{
    switch (m)
    {
#define XX(code, name, param)     \
    case FTPRequestCommand::name: \
        return #name;

        FTP_REQUEST_COMMAND(XX)
#undef XX
    default:
        return "UNKNOWN_CMD";
    }
}

FTPResponseCommand StringToFTPResponseCmd(const std::string &m)
{
#define XX(name, code)                   \
    if (strcmp(#name, m.c_str()) == 0)   \
    {                                    \
        return FTPResponseCommand::name; \
    }

    FTP_RESPONSE_COMMAND(XX)
#undef XX
    return FTPResponseCommand::INVALID_REQ;
}

/**
 * @brief
 *
 * @param m
 * @return const char*
 */
const char *FTPResponseCmdToString(const FTPResponseCommand &m)
{
    switch (m)
    {
#define XX(name, code)             \
    case FTPResponseCommand::name: \
        return #name;

        FTP_RESPONSE_COMMAND(XX)
#undef XX
    default:
        return "UNKNOWN_CMD";
    }
}