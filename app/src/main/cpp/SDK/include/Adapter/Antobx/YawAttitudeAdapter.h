#ifndef GNSSCORRECTION_ANT_ATTITUDE_H_
#define GNSSCORRECTION_ANT_ATTITUDE_H_
#include <map>
#include <mutex>
#include "include/Controller/Const.h"
#ifdef USEEIGEN
#include </usr/local/include/USEEIGEN3/USEEIGEN/Core>
#include </usr/local/include/USEEIGEN3/USEEIGEN/Geometry>
#endif
using namespace std;
namespace bamboo
{
#ifdef USEEIGEN
    class t_QuaterAttitude
    {
    public:
        t_QuaterAttitude(double w, double x, double y, double z) : q(w, x, y, z)
        {
            mjd = 0;
            sod = 0.0;
        }
        int mjd;
        double sod;
        ::USEEIGEN::Quaterniond q;
    };
#endif
    class YawAttitudeAdapter
    {
    public:
        /// quaterniond method
        bool m_quater2rot(int mjd, double sod, const char *cprn, double *xscf, double *yscf, double *zscf);
        //// traditional model part
#ifdef USEEIGEN
        void v_inputData(const char *cprn, t_QuaterAttitude &t);
        void m_quaterniond2EulerAngle(const USEEIGEN::Quaterniond &q, double &roll, double &pitch, double &yaw);
        t_QuaterAttitude m_search_att(int mjd, double sod, const char *cprn);
#endif
    protected:
        /// data here
#ifdef USEEIGEN
        map<string, vector<t_QuaterAttitude>> q_Data;
#endif
        std::mutex q_lock;
    };

} // namespace bamboo

#endif