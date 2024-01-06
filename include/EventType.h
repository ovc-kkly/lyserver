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

#define EVENT_TYPE(XX)                                     \
    /*request*/                                            \
    XX(0, ID_REQUEST, ID_REQUEST)                          \
    XX(1, LOGON_REQUEST, LOGON_REQUEST)                    \
    XX(2, SIGNUP_REQUEST, SIGNUP_REQUEST)                  \
    XX(3, CLOSE_ACCOUNT_REQUEST, CLOSE_ACCOUNT_REQUEST)    \
    XX(4, LOG_OUT_REQUEST, LOG_OUT_REQUEST)                \
    XX(5, TRANSMIT_REQUEST, TRANSMIT_REQUEST)              \
    /*response*/                                           \
    XX(6, ID_EVENT_RESPOND, ID_EVENT_RESPOND)              \
    XX(7, LOGON_RESPOND_NOUSER, LOGON_RESPOND_NOUSER)      \
    XX(8, LOGON_RESPOND_PWERROR, LOGON_RESPOND_PWERROR)    \
    XX(9, LOGON_RESPOND_PWRIGHT, LOGON_RESPOND_PWRIGHT)    \
    XX(10, SIGNUP_RESPOND_EXIST, SIGNUP_RESPOND_EXIST)     \
    XX(11, SIGNUP_RESPOND_SUCCESS, SIGNUP_RESPOND_SUCCESS) \
    XX(12, QUERY_RESPOND_ERROR, QUERY_RESPOND_ERROR)       \
    XX(13, CLOSE_RESPOND_SUCCESS, CLOSE_RESPOND_SUCCESS)   \
    XX(14, CLOSE_RESPOND_ERROR, CLOSE_RESPOND_ERROR)       \
    XX(15, CLOSE_RESPOND_PWERROR, CLOSE_RESPOND_PWERROR)   \
    XX(16, TRANSMIT_RESPOND, TRANSMIT_RESPOND)             \
    /*client state*/                                       \
    XX(17, CLIENT_STATE, CLIENT_STATE)                     \
    /*无动作，关闭*/                                 \
    XX(18, CLOSE_CONNECTION, CLOSE_CONNECTION)             \
    /*ESP8266相关的*/                                   \
    XX(19, RECOGNIZE, RECOGNIZE)                           \
    XX(20, IMAGE_LENGTH, IMAGE_LENGTH)                     \
    XX(21, READY_READ, READY_READ)

    enum class EventType
    {
#define XX(num, name, string) name = num,
        EVENT_TYPE(XX)
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
