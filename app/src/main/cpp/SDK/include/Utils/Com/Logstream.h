#ifndef CONFIG_UTILS_LOGSTREAM_H_
#define CONFIG_UTILS_LOGSTREAM_H_
#include "include/Controller/Const.h"
namespace bamboo
{
    class Logstream
    {
    public:
        Logstream() {}
        Logstream(string url) {}
        bool b_iset() { return binit; }
        void m_logPrint(int level, const char *fmt_msg, ...){}; /* log print into logger */

    protected:
        bool binit;
        list<string> m_msgs; /* msg to be sent */
    };
}
#endif