#ifndef EVENTTYPE_H
#define EVENTTYPE_H
#include <string>
#include <cstring>

namespace lyserver
{
    enum reactor_type
    {
        main_re,
        sub_re
    };
    enum class serverType
    {
        TCP,
        FTP,
        FTPDATA,
        UDP,
        HTTP
    };
#define EVENT_TYPE(XX, NUM)                                      \
    /*request*/                                                  \
    XX(NUM, ID_REQUEST, ID_REQUEST)                              \
    XX(NUM + 1, LOGON_REQUEST, LOGON_REQUEST)                    \
    XX(NUM + 2, SIGNUP_REQUEST, SIGNUP_REQUEST)                  \
    XX(NUM + 3, CLOSE_ACCOUNT_REQUEST, CLOSE_ACCOUNT_REQUEST)    \
    XX(NUM + 4, LOG_OUT_REQUEST, LOG_OUT_REQUEST)                \
    XX(NUM + 5, TRANSMIT_REQUEST, TRANSMIT_REQUEST)              \
    XX(NUM + 6, RETURN_REQUEST, RETURN_REQUEST)                  \
    /*response*/                                                 \
    XX(NUM + 7, ID_EVENT_RESPOND, ID_EVENT_RESPOND)              \
    XX(NUM + 8, LOGON_RESPOND_NOUSER, LOGON_RESPOND_NOUSER)      \
    XX(NUM + 9, LOGON_RESPOND_PWERROR, LOGON_RESPOND_PWERROR)    \
    XX(NUM + 10, LOGON_RESPOND_PWRIGHT, LOGON_RESPOND_PWRIGHT)   \
    XX(NUM + 11, SIGNUP_RESPOND_EXIST, SIGNUP_RESPOND_EXIST)     \
    XX(NUM + 12, SIGNUP_RESPOND_SUCCESS, SIGNUP_RESPOND_SUCCESS) \
    XX(NUM + 13, QUERY_RESPOND_ERROR, QUERY_RESPOND_ERROR)       \
    XX(NUM + 14, CLOSE_RESPOND_SUCCESS, CLOSE_RESPOND_SUCCESS)   \
    XX(NUM + 15, CLOSE_RESPOND_ERROR, CLOSE_RESPOND_ERROR)       \
    XX(NUM + 16, CLOSE_RESPOND_PWERROR, CLOSE_RESPOND_PWERROR)   \
    XX(NUM + 17, TRANSMIT_RESPOND, TRANSMIT_RESPOND)             \
    /*client state*/                                             \
    XX(NUM + 18, CLIENT_STATE, CLIENT_STATE)                     \
    /*无动作，关闭*/                                       \
    XX(NUM + 19, CLOSE_CONNECTION, CLOSE_CONNECTION)             \
    /*ESP8266相关的*/                                         \
    XX(NUM + 20, RECOGNIZE, RECOGNIZE)                           \
    XX(NUM + 21, IMAGE_LENGTH, IMAGE_LENGTH)                     \
    XX(NUM + 22, READY_READ, READY_READ)                         \
    XX(NUM + 23, FTP_REQUEST, FTP_REQUEST)                       \
    /*停车场客户端命令*/                                 \
    XX(NUM + 24, DETECTION_DATA, DETECTION_DATA)                 \
    /*RF_EVENT*/                                                 \
    XX(NUM + 25, RF_EXECUTE, RF_EXECUTE)                         \
    XX(NUM + 26, RF_RECEIVE, RF_RECEIVE)                         \
    XX(NUM + 27, RF_DATASET_ONE, RF_DATASET_ONE)                 \
    /*response*/                                                 \
    XX(NUM + 28, RF_EXECUTE_RESPONSE, RF_EXECUTE_RESPONSE)

    enum class EventType
    {
#define XX(num, name, string) name = num,
        EVENT_TYPE(XX, 0)
            // RF_EVENT_TYPE(XX, 25)
#undef XX
                INVALID_EVENT
    };

    /**
     * @brief 将字符串方法名转成Event方法枚举
     * @param[in] m Event方法
     * @return Event方法枚举
     */
    EventType StringToEventType(const std::string &m);

    /**
     * @brief 将Event方法枚举转换成字符串
     * @param[in] m Event方法枚举
     * @return 字符串
     */
    const char *EventTypeToString(const EventType &m);
}
#endif
