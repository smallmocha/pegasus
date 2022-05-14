#ifndef LOG_H
#define LOG_H

#include <string>
#include <thread>
#include <mutex>
#include <string.h>
#include "block_queue.h"
#include "buffer/buffer.h"

class Log {
public:
    void Init(int level, std::string path = "./log", std::string suffix = ".log", size_t queCapacity = 1024);

    static Log *Instance()
    {
        static Log log;
        return &log;
    }

    static void FlushLogThread()
    {
        Log::Instance()->AsyncWrite();
    }

    bool IsOpen()
    {
        return isOpen_;
    }

    int GetLevel()
    {
        return level_;
    }

    void SetLevel(int level)
    {
        level_ = level;
    }

    void Write(int level, const char *format, ...);
    void Flush();

private:
    Log();
    ~Log();

    void AppendLogTitle(int level);
    void AsyncWrite();

private:
    static constexpr uint32_t MAX_LINES = 10000;

    bool isOpen_;
    bool isAsync_;
    int level_;
    int lineCnt_;
    int today_;
    std::string path_;
    std::string suffix_;
    FILE *fp_;
    std::mutex mtx_;
    Buffer buffer_;
    std::unique_ptr<BlockQueue<std::string>> que_;
    std::unique_ptr<std::thread> writeThread_;
};

#define LOG_BASE(level, format, ...) \
    do { \
        Log *log = Log::Instance(); \
        if (log->IsOpen() && level >= log->GetLevel()) { \
            log->Write(level, format, ##__VA_ARGS__); \
            log->Flush(); \
        } \
    } while (0); \

#define LOG_DEBUG(format, ...) do { LOG_BASE(0, format, ##__VA_ARGS__) } while(0)
#define LOG_INFO(format, ...) do { LOG_BASE(1, format, ##__VA_ARGS__) } while(0)
#define LOG_WARNING(format, ...) do { LOG_BASE(2, format, ##__VA_ARGS__) } while(0)
#define LOG_ERROR(format, ...) do { LOG_BASE(3, format, ##__VA_ARGS__) } while(0)

#endif // !LOG_H
