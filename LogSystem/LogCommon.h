#pragma once
#include <iostream>
#include <memory>
#include <unordered_map>
#include <assert.h>

#define LOG_ROOT "LogsDir"
/************************************************************************************\
*   (未知)5 未知错误                                                                 *
*   (错误)4 数组越界、数据库连接失败、访问空指针                                     *
*   (警告)3 除零、内存泄漏、文件读写错误、文件找不到(某个操作可能存在问题，但不致命) *
*   (信息)2 系统启动或初始化时的日志，如系统参数                                     *
*   (调试)1 调试时能追踪到的信息，比如指针的值、变量的值、函数的调用                 *
\************************************************************************************/

enum LogLevel : uint8_t
{
    LOG_LEVEL_DEBUG = 1, // 调试日志
    LOG_LEVEL_INFO = 3, // 信息日志
    LOG_LEVEL_WARN = 4, // 警告日志
    LOG_LEVEL_ERROR = 5, // 错误日志
    LOG_LEVEL_INVALID = 0xFF // SKIP
};

// 日志输出类型 供用户使用
enum AppenderType : uint8_t
{
    APPENDER_NONE = 0x00, // 不设置输出
    APPENDER_CONSOLE = 0x01, // 控制台输出
    APPENDER_FILE = 0x02, // 文件输出
    APPENDER_BOTH = APPENDER_CONSOLE | APPENDER_FILE, // 控制台和文件输出
    APPENDER_INVALID = 0xFF // 无效标志
};