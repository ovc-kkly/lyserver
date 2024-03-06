/**
 * @file macro.h
 * @brief 常用宏的封装
 * @author lyserver.yin
 * @email 564628276@qq.com
 * @date 2019-06-01
 * @copyright Copyright (c) 2019年 lyserver.yin All rights reserved (www.lyserver.top)
 */
#ifndef MACRO_H
#define MACRO_H

#include <string.h>
#include <assert.h>
#include "log.h"
#include "myutil.h"

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define LY_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define LY_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LY_LIKELY(x) (x)
#define LY_UNLIKELY(x) (x)
#endif

/// 断言宏封装
#define LY_ASSERT(x)                                                                \
    if (LY_UNLIKELY(!(x)))                                                          \
    {                                                                                  \
        LY_LOG_ERROR(LY_LOG_ROOT()) << "ASSERTION: " #x                          \
                                          << "\nbacktrace:\n"                          \
                                          << lyserver::BacktraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

/// 断言宏封装
#define LY_ASSERT2(x, w)                                                            \
    if (LY_UNLIKELY(!(x)))                                                          \
    {                                                                                  \
        LY_LOG_ERROR(LY_LOG_ROOT()) << "ASSERTION: " #x                          \
                                          << "\n"                                      \
                                          << w                                         \
                                          << "\nbacktrace:\n"                          \
                                          << lyserver::BacktraceToString(100, 2, "    "); \
        assert(x);                                                                     \
    }

#endif
