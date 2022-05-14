#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <buffer/buffer.h>

class HttpResponse {
public:
    HttpResponse() : code_(-1), mmFile_(nullptr)
    {
        srcDir_ = "";
        path_ = "";
        bzero(&mmFileStat_, sizeof(mmFileStat_));
    }

    ~HttpResponse() {
        UnmapFile();
    }

    void Init(const std::string &srcDir, const std::string &path, int code = -1)
    {
        srcDir_ = srcDir;
        path_ = path;
        code_ = code;
        if (mmFile_) {
            UnmapFile();
        }
        mmFile_ = nullptr;
        bzero(&mmFileStat_, sizeof(mmFileStat_));
    }

    void MakeResponse(Buffer &buffer);

    char *File() const
    {
        return mmFile_;
    }

    size_t FileLen() const
    {
        return mmFileStat_.st_size;
    }

    void UnmapFile()
    {
        if (mmFile_) {
            munmap(mmFile_, mmFileStat_.st_size);
            mmFile_ = nullptr;
        }
    }

private:
    void AppendStateLine(Buffer &buffer);
    void AppendHeader(Buffer &buffer);
    void AppendContent(Buffer &buffer);
    void ErrorHtml();
    void ErrorContent(Buffer &buff, const std::string &message);

    std::string GetFileType();

private:
    int code_;
    std::string path_;
    std::string srcDir_;
    char *mmFile_;
    struct stat mmFileStat_;
};

#endif // !HTTP_RESPONSE_H
