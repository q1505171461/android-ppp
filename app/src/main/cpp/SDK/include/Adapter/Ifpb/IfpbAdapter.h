#ifndef GNSSALGO_GNSSCORRECTION_IFPB_IFPBADAPTER_INCLUDE_H_
#define GNSSALGO_GNSSCORRECTION_IFPB_IFPBADAPTER_INCLUDE_H_
#include <mutex>
#include "t_ifpb.h"
namespace bamboo
{
    class IfpbAdapter {
    public:
        IfpbAdapter();
        ~IfpbAdapter();
        t_ifpb m_getCorrection(string cprn) {
            std::unique_lock<std::mutex> lk(m_lock);
            return m_ifpbs.find(cprn) == m_ifpbs.end() ? t_ifpb() : m_ifpbs[cprn];
        }
        void m_inputIfpb(string cprn, t_ifpb&);
    protected:
        std::mutex m_lock;
        map<string, t_ifpb> m_ifpbs;
    };
}
#endif