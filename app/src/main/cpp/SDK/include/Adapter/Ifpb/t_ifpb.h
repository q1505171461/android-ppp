#ifndef GNSSCORRECTION_DCB_IFPB_T_IFPB_INCLUDE_H_
#define GNSSCORRECTION_DCB_IFPB_T_IFPB_INCLUDE_H_
#include "include/Controller/Const.h"
#include <map>
using namespace std;
namespace bamboo
{
    class t_ifpb
    {
    public:
        void m_reset()

        {
            ic_r = 0;
            m_iset = 0;
            m_t0 = m_tc = 0;
            m_sum = angle = 0.0;
        }
        t_ifpb() { m_reset(); }
        bool m_iset;
        int ic_r; /* reset count */
        double m_sum, angle;
        time_t m_t0, m_tc;
    };
}
#endif