
#ifndef ENDIAN_H
#define ENDIAN_H

#define lyserver_LITTLE_ENDIAN 1
#define lyserver_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

namespace lyserver
{

    // /**
    //  * @brief 8字节类型的字节序转化
    //  */
    // template <class T>
    // typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type
    // byteswap(T value)
    // {
    //     return (T)bswap_64((uint64_t)value);
    // }

    // /**
    //  * @brief 4字节类型的字节序转化
    //  */
    // template <class T>
    // typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type
    // byteswap(T value)
    // {
    //     return (T)bswap_32((uint32_t)value);
    // }

    // /**
    //  * @brief 2字节类型的字节序转化
    //  */
    // template <class T>
    // typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type
    // byteswap(T value)
    // {
    //     return (T)bswap_16((uint16_t)value);
    // }
    template <class T>
    T byteswap(T value)
    {
        static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) == sizeof(uint32_t) || sizeof(T) == sizeof(uint16_t),
                      "Unsupported type for byteswap");

        if constexpr (sizeof(T) == sizeof(uint64_t))
        {
            return (T)bswap_64((uint64_t)value);
        }
        else if constexpr (sizeof(T) == sizeof(uint32_t))
        {
            return (T)bswap_32((uint32_t)value);
        }
        else if constexpr (sizeof(T) == sizeof(uint16_t))
        {
            return (T)bswap_16((uint16_t)value);
        }
        else
        {
            // Handle unsupported type (if needed)
            static_assert(sizeof(T) == sizeof(uint64_t) || sizeof(T) == sizeof(uint32_t) || sizeof(T) == sizeof(uint16_t),
                          "Unsupported type for byteswap");
            return value;
        }
    }

#if BYTE_ORDER == BIG_ENDIAN
#define lyserver_BYTE_ORDER lyserver_BIG_ENDIAN
#else
#define lyserver_BYTE_ORDER lyserver_LITTLE_ENDIAN
#endif

#if lyserver_BYTE_ORDER == lyserver_BIG_ENDIAN

    /**
     * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
     */
    template <class T>
    T byteswapOnLittleEndian(T t)
    {
        return t;
    }

    /**
     * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
     */
    template <class T>
    T byteswapOnBigEndian(T t)
    {
        return byteswap(t);
    }
#else

    /**
     * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
     */
    template <class T>
    T byteswapOnLittleEndian(T t)
    {
        return byteswap(t);
    }

    /**
     * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
     */
    template <class T>
    T byteswapOnBigEndian(T t)
    {
        return t;
    }
#endif

}

#endif
