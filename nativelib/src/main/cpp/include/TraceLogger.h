//
// Created by fang on 2024/12/26.
// TraceLogger - QBDI追踪日志记录工具声明
//

#ifndef QBDI_TRACE_TRACELOGGER_H
#define QBDI_TRACE_TRACELOGGER_H

#include <string>
#include <map>
#include <vector>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>

// 日志类型枚举
enum class LogType {
    QBDI_TRACE,     // QBDI指令追踪
    THREAD_TRACE,   // 线程创建追踪
    HOOK_EVENT,     // Hook事件
    GENERAL_EVENT   // 通用事件
};

// SO类型枚举
enum class SoType {
    SYSTEM_SO,      // 系统SO
    APP_SO,         // 应用SO
    CUSTOM_SO,      // 自定义路径SO
    UNKNOWN_SO      // 未知类型SO
};

// Sgmain Hook日志事件类型枚举
enum class SgmainLogType {
    INIT,           // 初始化事件
    HOOK_SETUP,     // Hook安装事件
    FUNCTION_ENTER, // 函数进入事件
    FUNCTION_EXIT,  // 函数退出事件
    PARAMETER,      // 参数分析事件
    RETURN_VALUE,   // 返回值事件
    ERROR,          // 错误事件
    DEBUG           // 调试信息
};

// Sgmain Hook日志级别
enum class SgmainLogLevel {
    INFO,           // 普通信息
    WARNING,        // 警告信息
    ERROR,          // 错误信息
    DEBUG           // 调试信息
};

// 线程信息结构
struct ThreadInfo {
    pthread_t thread_id;             // pthread标识符
    pid_t real_tid;                  // 真实的线程TID
    std::string thread_name;
    std::string creator_so;          // SO名称（保持向后兼容）
    std::string creator_so_path;     // SO完整路径
    SoType creator_so_type;          // SO类型
    uintptr_t creator_address;
    uintptr_t start_routine;
    int creator_tid;
    std::string timestamp;
};

// SO分类统计信息
struct SoClassificationStats {
    std::map<std::string, int> system_so_count;     // 系统SO统计
    std::map<std::string, int> app_so_count;        // 应用SO统计
    std::map<std::string, int> custom_so_count;     // 自定义SO统计
    std::map<std::string, int> unknown_so_count;    // 未知SO统计

    int total_system_threads;
    int total_app_threads;
    int total_custom_threads;
    int total_unknown_threads;

    SoClassificationStats() : total_system_threads(0), total_app_threads(0),
                              total_custom_threads(0), total_unknown_threads(0) {}
};

// 线程统计信息
struct ThreadStatistics {
    std::map<std::string, std::vector<ThreadInfo>> so_thread_map;  // SO -> 线程列表
    std::map<std::string, int> so_thread_count;                    // SO -> 线程数量
    SoClassificationStats classification_stats;                     // SO分类统计
    int total_threads;                                              // 总线程数
    int call_count;                                                 // 调用计数器
    time_t last_report_time;                                        // 上次报告时间
    time_t last_thread_time;                                        // 上次线程创建时间

    ThreadStatistics() : total_threads(0), call_count(0), last_report_time(0),
                         last_thread_time(0) {}
};

// 主要日志记录函数
bool writeTraceLog(LogType type, void* address, const std::string& logContent,
                   const char* additional_info = nullptr);

// 获取日志路径
std::string getLogPath(LogType type,void* address);

// 线程创建记录函数
bool recordThreadCreation(const ThreadInfo& thread_info);

// 获取线程统计信息
ThreadStatistics getThreadStatistics();

// 写入线程统计报告
bool writeThreadStatisticsReport();

// 事件日志记录函数
bool writeEventLog(const char* event_name, const std::string& message,
                   const char* additional_info = nullptr);

// Hook日志记录函数
bool writeHookLog(const char* hook_name, void* target_address, const std::string& message);
// SO分类相关函数
SoType classifySoType(const std::string& so_path);
const char* getSoTypeName(SoType type);
std::string getSoPathFromAddress(void* address);

// 线程管理相关函数
void updateThreadNames();                                    // 更新所有线程的名称
bool updateThreadName(const std::string& so_name, pthread_t thread_id, const std::string& name);
pid_t getThreadTid(pthread_t thread_id);                   // 获取pthread对应的真实TID

// 工具函数
bool getFormattedTimestamp(char* buffer, size_t buffer_size);
bool ensureLogDirectory(const std::string& log_dir);
const char* getLogTypeName(LogType type);
const char* getLogTypePrefix(LogType type);

#endif //QBDI_TRACE_TRACELOGGER_H