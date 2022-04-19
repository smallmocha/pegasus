#ifndef HTTP_REQUST_H
#define HTTP_REQUST_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include "buffer/buffer.h"

enum ParseState {
    PARSE_REQUEST_LINE = 0,
    PARSE_HEADER,
    PARSE_BODY,
    PARSE_FINISH,
};

class HttpRequst {
public:
    HttpRequst() 
    {
        Init();
    }
    ~HttpRequst() = default;

    void Init() 
    {
        state_ = PARSE_REQUEST_LINE;
        method_ = path_ = version_ = body_ = "";
    }

    bool Parse(Buffer &Buffer);

    std::string Method() const 
    {
        return method_;
    }

    std::string Path() const 
    {
        return path_;
    }

    std::string Version() const 
    {
        return version_;
    }

    std::string Body() const 
    {
        return body_;
    }

private:
    bool ParseRequstLine(const std::string &line);
    void ParseHeader(const std::string &line);
    void ParseBody(const std::string &line);

private:
    ParseState state_;
    std::string method_, path_, version_, body_;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
};

#endif // !HTTP_REQUST_H
