#include "httprequest.h"

using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML{
    "/index",
    "/register",
    "/login",
    "/welcome",
    "/video",
    "/picture",
};

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG{
    {"/register.html", 0},
    {"/login.html", 1},
};

void HttpRequest::Init()
{
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
}

bool HttpRequest::IsKeepAlive() const
{
    if (header_.count("Connection") == 1)
    {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::parse(Buffer &buff)
{
    const char CRLF[] = "\r\n";
    if (buff.ReadableBytes() <= 0)
    {
        return false;
    }
    while (buff.ReadableBytes() && state_ != FINISH)
    {
        // 找到 \r\n 的位置
        const char *lineEnd = search(buff.curReadPtr(), buff.curWritePtrConst(), CRLF, CRLF + 2);
        std::string line(buff.curReadPtr(), lineEnd);
        switch (state_)
        {
        case REQUEST_LINE:
            if (!ParseRequestLine_(line))
            {
                return false;
            }
            ParsePath_();
            break;
        case HEADERS:
            ParseHeader_(line);
            /*无数据可读，结束*/
            if (buff.ReadableBytes() <= 2)
            {
                state_ = FINISH;
            }
            break;
        case BODY:
            ParseBody_(line);
            break;
        default:
            break;
        }
        if (lineEnd == buff.curWritePtr())
        {
            break;
        }
        buff.RetrieveUntil(lineEnd + 2);    //定位下一行
    }

    printf("[%s], [%s], [%s]\n", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

//解析请求路径
void HttpRequest::ParsePath_()
{
    if (path_ == "/")
    {
        path_ = "/index.html";
    }
    else
    {
        for (auto &item : DEFAULT_HTML)
        {
            if (item == path_)
            {
                path_ += ".html";
                break;
            }
        }
    }
}

bool HttpRequest::ParseRequestLine_(const string &line)
{
    /* 通配符：()分组 [^ ]:除空格之外的所有字符 *：重复多次*/
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten))
    {
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];

        printf("[%s], [%s], [%s]\n", method_.c_str(), path_.c_str(), version_.c_str());

        state_ = HEADERS;
        return true;
    }

    printf("RequestLine Error\n");
    return false;
}

void HttpRequest::ParseHeader_(const string &line)
{
    /* . 表示匹配除换行符" \n "以外的任意字符
    ** ? 表示零次或一次匹配前面的字符或子表达式
    */
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if (regex_match(line, subMatch, patten))
    {
        /*subMatch[1]:头部字段名，subMatch[2]：值*/
        header_[subMatch[1]] = subMatch[2];

        printf("%s : %s\r\n", std::string(subMatch[1]).c_str(), std::string(subMatch[2]).c_str());
    }
    else
        state_ = BODY;
}

void HttpRequest::ParseBody_(const string &line)
{
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    printf("Body:%s, len:%ld\n", line.c_str(), line.size());
}

void HttpRequest::ParsePost_()
{
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded")
    {
        ParseFromUrlencoded_();
        if (DEFAULT_HTML_TAG.count(path_))
        {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            printf("Tag:%d\n", true);
            if (tag == 0 || tag == 1)
            {
                //bool isLogin = (tag == 1);
                // if(UserVerify(post_["username"]))
                path_ = "/welcome.html";
            }
        }
    }
}

void HttpRequest::ParseFromUrlencoded_()
{
    if (body_.size() == 0)
        return;

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for (; i < n; i++)
    {
        char ch = body_[i];
        switch (ch)
        {
        case '=':
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body_[i] = ' ';
            break;
        case '%':
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            printf("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }      
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

int HttpRequest::ConverHex(char ch)
{
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'F')
        return ch - 'a' + 10;
    return ch;
}

std::string HttpRequest::path() const
{
    return path_;
}

std::string &HttpRequest::path()
{
    return path_;
}

std::string HttpRequest::method() const
{
    return method_;
}

std::string HttpRequest::version() const
{
    return version_;
}

std::string HttpRequest::GetPost(const std::string &key) const
{
    assert(key != "");
    if (post_.count(key) == 1)
    {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char *key) const
{
    assert(key != nullptr);
    if (post_.count(key) == 1)
    {
        return post_.find(key)->second;
    }
    return "";
}