#ifndef MYUTIL_H
#define MYUTIL_H

#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include "ocr_car.h"
#include "ThreadPool.h"
namespace lyserver
{
    using CHECK_CB = std::function<int()>; // 包装器
    void errif(bool, const char *);
    /**
     * @brief 获取当前时间的毫秒
     */
    uint64_t GetCurrentMS();

    /**
     * @brief 获取当前时间的微秒
     */
    uint64_t GetCurrentUS();

}

#endif