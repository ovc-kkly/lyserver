#include "EventType.h"
namespace lyserver
{
    EventType StringToEventType(const std::string &m)
    {
#define XX(num, name, string)            \
    if (strcmp(#string, m.c_str()) == 0) \
    {                                    \
        return EventType::name;          \
    }
        EVENT_TYPE(XX, 0);
        // RF_EVENT_TYPE(XX, 25);
#undef XX
        return EventType::INVALID_EVENT;
    }

    static const char *s_Type_string[] = {
#define XX(num, name, string) #string,
        EVENT_TYPE(XX, 0)
        // RF_EVENT_TYPE(XX, 25)
#undef XX
    };

    const char *EventTypeToString(const EventType &m)
    {
        uint32_t idx = (uint32_t)m;
        if (idx >= (sizeof(s_Type_string) / sizeof(s_Type_string[0])))
        {
            return "<unknown>";
        }
        return s_Type_string[idx];
    }
}
