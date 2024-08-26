#include "Logger.h"

#if defined DEBUG
bool is_null = false;
#else
bool is_null = true;
#endif

int Logger::LogBuf::sync()
{
    if (is_null)
    {
        str("");
        return 0;
    }

    //std::cout << m_marker << ": " << str() << endl;
    cout.write(str().c_str(), str().size());
    //cout << str();
    str("");

    return std::cout ? 0 : -1;
}

//#if defined DEBUG
//Logger debug_log(false);
//#elif defined NDEBUG
//Logger debug_log(true);
//#endif

Logger debug_log;