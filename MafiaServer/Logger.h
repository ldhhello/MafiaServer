#pragma once
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>

using namespace std;

// is_null이 true면 실제 화면에 아무것도 띄우지 않음.
class Logger : public ostream
{
private:
    class LogBuf : public stringbuf
    {
    private:
        // or whatever you need for your application
        //bool is_null;
    public:
        //LogBuf(bool is_null) : is_null(is_null) { }

        ~LogBuf() { pubsync(); }
        int sync();
    };

public:
    Logger() : std::ostream(new LogBuf) {}
    ~Logger() { delete rdbuf(); }
};

extern Logger debug_log;