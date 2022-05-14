#include "http_requst.h"
#include <regex>
#include <log/log.h>

const std::unordered_set<std::string> HttpRequst::DEFAULT_HTML {
    "/index", "/register", "/login", "/welcome", "/video", "/picture", 
};

const std::unordered_map<std::string, int> HttpRequst::DEFAULT_HTML_TAG {
    {"/register.html", 0}, {"/login.html", 1},
};

bool HttpRequst::Parse(Buffer &buffer)
{
    if (buffer.ReadableBytes() <= 0) {
        return false;
    }
    const char *CRLF = "\r\n";
    while (buffer.ReadableBytes() > 0 && state_ != PARSE_FINISH) {
        const char *end = std::search(buffer.BeginRead(), buffer.BeginWriteConst(), CRLF, CRLF + 2);
        std::string line(buffer.BeginRead(), end);
        switch (state_) {
        case PARSE_REQUEST_LINE:
            if (!ParseRequstLine(line)) {
                return false;
            }
            break;
        case PARSE_HEADER:
            ParseHeader(line);
            if(buffer.ReadableBytes() <= 2) {
                state_ = PARSE_FINISH;
            }
            break;
        case PARSE_BODY:
            ParseBody(line);
            break;
        default:
            break;
        }
        if (buffer.BeginWrite() == end) {
            break;
        }
        buffer.RetrieveUtil(end + 2);
    }
    LOG_INFO("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

bool HttpRequst::ParseRequstLine(const std::string &line)
{
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if (std::regex_match(line, subMatch, patten)) {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = PARSE_HEADER;
        if (path_ == "/") {
            path_ += "index.html";
        } else {
            for(auto &item : HttpRequst::DEFAULT_HTML) {
                if (item == path_) {
                    path_ += ".html";
                    break;
                }
            }
        }

        return true;
    }
    return false;
}

void HttpRequst::ParseHeader(const std::string &line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(!regex_match(line, subMatch, patten)) {
        state_ = PARSE_BODY;
    } else {
        
    }
}

void HttpRequst::ParseBody(const std::string &line)
{
    body_ = line;
    state_ = PARSE_FINISH;
}
