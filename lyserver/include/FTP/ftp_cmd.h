#ifndef FTP_CMD_H
#define FTP_CMD_H
#include <string>
#include <cstring>

#define FTP_REQUEST_COMMAND(XX)    \
    XX(0, USER, PARAMETER)         \
    XX(1, PASS, PARAMETER)         \
    XX(2, QUIT, NO_PARAMETER)      \
    XX(3, CWD, DIRECTORY)          \
    XX(4, CDUP, NO_PARAMETER)      \
    XX(5, PWD, NO_PARAMETER)       \
    XX(6, LIST, PATHNAME)          \
    XX(7, RETR, PARAMETER)         \
    XX(8, RETRCON, PARAMETER)      \
    XX(9, STOR, PARAMETER)         \
    XX(10, DELE, PARAMETER)        \
    XX(11, MKD, PARAMETER)         \
    XX(12, RMD, PARAMETER)         \
    XX(13, RNFR, PARAMETER)        \
    XX(14, RNTO, PARAMETER)        \
    XX(15, PASV, NO_PARAMETER)     \
    XX(16, PORT, HOST_PORT)        \
    XX(17, TYPE, DATA_TYPE)        \
    XX(18, MODE, PARAMETER)        \
    XX(19, STRU, PARAMETER)        \
    XX(20, SITE, PARAMETER)        \
    XX(21, ALLO, PARAMETER)        \
    XX(22, SYST, NO_PARAMETER)     \
    XX(23, NOOP, NO_PARAMETER)     \
    XX(24, REIN, NO_PARAMETER)     \
    XX(25, ABOR, NO_PARAMETER)     \
    XX(26, HELP, PARAMETER)        \
    XX(27, STAT, PARAMETER)        \
    XX(28, AUTH, PARAMETER)        \
    XX(29, ADAT, PARAMETER)        \
    XX(30, MLSD, PATHNAME)         \
    XX(31, MLST, PATHNAME)         \
    XX(32, INIT, NO_PARAMETER)     \
    XX(33, STOR_ING, NO_PARAMETER) \
    XX(34, MSG_SUCCESSED, NO_PARAMETER)

#define FTP_RESPONSE_COMMAND(XX) \
    XX(INIT, 119)                \
                                 \
    XX(Code120, 120)             \
    XX(Code125, 125)             \
    XX(Code150, 150)             \
    XX(Code151, 151)             \
                                 \
    XX(Code200, 200)             \
    XX(Code202, 202)             \
    XX(Code211, 211)             \
    XX(Code212, 212)             \
    XX(Code213, 213)             \
    XX(Code214, 214)             \
    XX(Code215, 215)             \
    XX(Code220, 220)             \
    XX(Code221, 221)             \
    XX(Code225, 225)             \
    XX(Code226, 226)             \
    XX(Code227, 227)             \
    XX(Code230, 230)             \
    XX(Code250, 250)             \
    XX(Code257, 257)             \
                                 \
    XX(Code331, 331)             \
    XX(Code332, 332)             \
                                 \
    XX(Code425, 425)             \
    XX(Code350, 350)             \
    XX(Code426, 426)             \
    XX(Code450, 450)             \
    XX(Code451, 451)             \
    XX(Code452, 452)             \
                                 \
    XX(Code500, 500)             \
    XX(Code501, 501)             \
    XX(Code502, 502)             \
    XX(Code503, 503)             \
    XX(Code504, 504)             \
    XX(Code530, 530)             \
    XX(Code532, 532)             \
    XX(Code550, 550)             \
    XX(Code551, 551)             \
    XX(Code552, 552)             \
    XX(Code553, 553)

enum class FTPRequestCommand
{
#define XX(num, name, param) name = num,
    FTP_REQUEST_COMMAND(XX)
#undef XX
    INVALID_CMD
};
// 枚举FTP响应命令的标识符
enum class FTPResponseCommand
{
#define XX(name, code) name = code,
    FTP_RESPONSE_COMMAND(XX)
#undef XX
    INVALID_REQ
};
/**
 * @brief
 *
 * @param m
 * @return FTPRequestCommand
 */
FTPRequestCommand StringToFTPRequestCmd(const std::string &m);

/**
 * @brief
 *
 * @param m
 * @return const char*
 */
const char *FTPRequestCmdToString(const FTPRequestCommand &m);

FTPResponseCommand StringToFTPResponseCmd(const std::string &m);

/**
 * @brief
 *
 * @param m
 * @return const char*
 */
const char *FTPResponseCmdToString(const FTPResponseCommand &m);

/*

#define FTP_RESPONSE_COMMAND_LIST \
    XX(120, DATACONN)  \                                  // 服务不久即将就绪
XX(125, NOOPOK)                \                          // 数据连接打开；数据传输不久即将开始
    XX(150, TYPEOK)               \                       // 文件状态是OK
    XX(200, COMMAND_OK)           \                       // 命令OK
    XX(202, ALLOOK)                \                      // 命令未执行，站点参数完成
    XX(211, FEAT)                 \                       // 系统状态或求助回答
    XX(212, DIRECTORY_STATUS)     \                       // 目录状态
    XX(213, SIZEOK)               \                       // 文件状态
    XX(214, SITEHELP)             \                       // 求助报文
    XX(215, SYSTOK)               \                       // 命名系统类型（操作系统）
    XX(220, GREET)                \                       // 服务就绪
    XX(221, GOODBYE)              \                       // 服务关闭
    XX(225, ABOR_NOCONN)          \                       // 不能打开数据连接
    XX(226, TRANSFEROK)           \                       // 关闭数据连接
    XX(227, PASVOK)               \                       // 进入被动方式，服务器发送IP地址和端口号
    XX(229, EPSVOK)               \                       // 进入扩展被动方式
    XX(230, LOGINOK)              \                       // 用户登录OK
    XX(234, AUTHOK)               \                       // 认证OK
    XX(250, CWDOK)                \                       // 请求文件动作OK
    XX(257, MKDIROK)              \                       // 创建目录OK
    XX(331, GIVEPWORD)            \                       // 用户名OK：需要口令
    XX(332, NEED_LOGIN_ACCOUNT)   \                       // 需要登录账号
    XX(350, RESTOK)               \                       // 文件动作在进行中：需要更多的信息
    XX(421, IDLE_TIMEOUT)         \                       // 连接超时
    XX(425, BADSENDCONN)          \                       // 不能打开数据连接
    XX(426, BADSENDNET)           \                       // 连接关闭：不能识别的命令
    XX(450, FILE_ACTION_NOT_TAKEN_FILE_UNAVAILABLE) \     // 未采取文件动作：文件不可用
    XX(451, ACTION_ABORTED_LOCAL_ERROR) \                 // 动作异常终止：本地差错
    XX(452, ACTION_ABORTED_INSUFFICIENT_STORAGE) \        // 动作异常终止：存储器不足
    XX(500, SYNTAX_ERROR_COMMAND_UNRECOGNIZED) \          // 语法差错：不能识别的命令
    XX(501, SYNTAX_ERROR_IN_PARAMETERS_OR_ARGUMENTS) \    // 参数或变量的语法差错
    XX(502, COMMAND_NOT_IMPLEMENTED) \                    // 命令未实现
    XX(503, BAD_SEQUENCE_OF_COMMANDS) \                   // 不良命令序列
    XX(504, COMMAND_PARAMETER_NOT_IMPLEMENTED) \          // 命令参数未实现
    XX(530, NOT_LOGGED_IN) \                              // 用户未登录
    XX(532, NEED_ACCOUNT_FOR_STORING_FILES) \             // 存储文件需要账号
    XX(550, ACTION_NOT_TAKEN_FILE_UNAVAILABLE) \          // 动作未完成：文件不可用
    XX(552, ACTION_ABORTED_EXCEEDED_STORAGE_ALLOCATION) \ // 请求的动作异常终止：超过分配的存储器空间
    XX(553, ACTION_NOT_TAKEN_FILE_NAME_NOT_ALLOWED) \     // 未采取请求动作：文件名不允许
*/
#endif