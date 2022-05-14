#include "log.h"
#include <memory>
#include <stdarg.h> // va_list
#include <sys/time.h>
#include <sys/stat.h> // mkdir

Log::Log()
{
    isOpen_ = false;
    isAsync_ = false;
    fp_ = nullptr;
    que_ = nullptr;
    writeThread_ = nullptr;
}

Log::~Log()
{
    if (writeThread_ != nullptr && writeThread_->joinable()) {
        while (!que_->Empty()) {
            que_->Flush();
        }
        que_->Close();
        writeThread_->join();    
    }    
    if (fp_ != nullptr) {
        std::lock_guard<std::mutex> locker(mtx_);
        Flush();
        fclose(fp_);
    }
}

void Log::Init(int level, std::string path, std::string suffix, size_t queCapacity)
{
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    lineCnt_ = 0;
    if (queCapacity > 0) {
        isAsync_ = true;
        que_ = std::make_unique<BlockQueue<std::string>>(queCapacity);
        assert(que_);
        writeThread_ = std::make_unique<std::thread>(FlushLogThread);
        assert(writeThread_);
    } else {
        isAsync_ = false;
    }

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    // localtime函数不可重入，需要拷贝下来
    struct tm t = *sysTime;
    today_ = t.tm_mday;

    char fileName[256];
    snprintf(fileName, sizeof(fileName) - 1, "%s/%04d_%02d_%02d%s", path_.c_str(),
        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_.c_str());
    {
        std::lock_guard<std::mutex> locker(mtx_);
        buffer_.RetrieveAll();
        if (fp_) {
            Flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr) {
            mkdir(path_.c_str(), 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

void Log::Write(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;

    std::lock_guard<std::mutex> locker(mtx_);
    if ((today_ != t.tm_mday) || (lineCnt_ > 0 && lineCnt_ % MAX_LINES == 0)) {
        char fileName[256];
        if (today_ != t.tm_mday) {
            snprintf(fileName, sizeof(fileName) - 1, "%s/%04d_%02d_%02d%s", path_.c_str(),
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_.c_str());
            lineCnt_ = 0;
            today_ = t.tm_mday;
        } else {
            snprintf(fileName, sizeof(fileName) - 1, "%s/%04d_%02d_%02d_%d%s", path_.c_str(),
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, lineCnt_ / MAX_LINES, suffix_.c_str());
        }
        Flush();
        fclose(fp_);
        fp_ = fopen(fileName, "a");
        assert(fp_ != nullptr);
    }
    ++lineCnt_;
    int n = snprintf(buffer_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                     t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                     t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
    buffer_.HasWritten(n);

    AppendLogTitle(level);

    va_list vaList;
    va_start(vaList, format);
    int m = vsnprintf(buffer_.BeginWrite(), buffer_.WritableBytes(), format, vaList);
    va_end(vaList);
    buffer_.HasWritten(m);
    buffer_.Append("\n\0", 2); // "\n\0"的len为1

    if (isAsync_ && que_ != nullptr && !que_->Full()) {
        que_->PushBack(buffer_.RetrieveAllToStr());
    } else {
        fputs(buffer_.BeginRead(), fp_);
    }
    buffer_.RetrieveAll();
}

void Log::Flush()
{
    if (isAsync_) {
        que_->Flush();
    }
    fflush(fp_);
}

void Log::AppendLogTitle(int level)
{
    switch (level)
    {
    case 0:
        buffer_.Append("[DEBUG] : ");
        break;
    case 1:
        buffer_.Append("[INFO]  : ");
        break;
    case 2:
        buffer_.Append("[WARN]  : ");
        break;
    case 3:
        buffer_.Append("[ERROR] : ");
        break;
    default:
        buffer_.Append("[INFO]  : ");
        break;
    }
}

void Log::AsyncWrite()
{
    std::string str;
    while (que_->Pop(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}
