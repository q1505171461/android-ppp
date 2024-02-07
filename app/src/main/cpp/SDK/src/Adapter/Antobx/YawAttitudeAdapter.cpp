#include "include/Adapter/Antobx/YawAttitudeAdapter.h"
#include "include/Controller/Controller.h"
using namespace bamboo;
using namespace std;

bool YawAttitudeAdapter::m_quater2rot(int mjd, double sod, const char *cprn, double *xscf, double *yscf, double *zscf)
{
/// acquire the corresponding Quaterniond here
#ifdef USEEIGEN
    if (q_Data.find(cprn) == q_Data.end())
        return false;
    t_QuaterAttitude t_q = m_search_att(mjd, sod, cprn);
    if (fabs((mjd - t_q.mjd) + sod - t_q.sod) > 300)
        return false;
    ::USEEIGEN::Matrix3d rx = t_q.q.toRotationMatrix();
    /// copy back into the x,y,z
    xscf[0] = rx(0, 0);
    xscf[1] = rx(0, 1);
    xscf[2] = rx(0, 2);

    yscf[0] = rx(1, 0);
    yscf[1] = rx(1, 1);
    yscf[2] = rx(1, 2);

    zscf[0] = rx(2, 0);
    zscf[1] = rx(2, 1);
    zscf[2] = rx(2, 2);

    return true;
#else
    return false;
#endif
}
#ifdef USEEIGEN
void YawAttitudeAdapter::m_quaterniond2EulerAngle(const USEEIGEN::Quaterniond &q, double &roll, double &pitch, double &yaw)
{
    // roll (x-axis rotation)
    double sinr_cosp = +2.0 * (q.w() * q.x() + q.y() * q.z());
    double cosr_cosp = +1.0 - 2.0 * (q.x() * q.x() + q.y() * q.y());
    roll = atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = +2.0 * (q.w() * q.y() - q.z() * q.x());
    if (fabs(sinp) >= 1)
        pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    double siny_cosp = +2.0 * (q.w() * q.z() + q.x() * q.y());
    double cosy_cosp = +1.0 - 2.0 * (q.y() * q.y() + q.z() * q.z());
    yaw = atan2(siny_cosp, cosy_cosp);
}
t_QuaterAttitude YawAttitudeAdapter::m_search_att(int mjd, double sod, const char *cprn)
{
    int i, j, k, ncount;
    t_QuaterAttitude q(0, 0, 0, 0);
    if (q_Data.find(cprn) == q_Data.end())
        return q;
    vector<t_QuaterAttitude> &v_att = q_Data[cprn];
    ncount = v_att.size();
    // binary search here
    for (i = 0, j = ncount - 1; i < j;)
    {
        k = (i + j) / 2;
        if ((v_att[k].mjd - mjd) * 86400.0 + v_att[k].sod - sod < -bamboo::MAXWND)
            i = k + 1;
        else
            j = k;
    }
    // i is the exact position here
    return v_att[i];
}
void YawAttitudeAdapter::v_inputData(const char *cprn, t_QuaterAttitude &t)
{
    int mjd;
    double sod;
    std::unique_lock<std::mutex> l_k(q_lock);
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    if (q_Data.find(cprn) == q_Data.end())
        q_Data[cprn] = vector<t_QuaterAttitude>();
    q_Data[cprn].push_back(t);
    vector<t_QuaterAttitude> &series = q_Data[cprn];
    if (!dly->lpost)
    {
        vector<t_QuaterAttitude>::iterator itr;
        for (itr = series.begin(); itr != series.end();)
        {
            if ((mjd - itr->mjd) * 86400.0 + sod - itr->sod > 7200.0)
            {
                itr = series.erase(itr);
                continue;
            }
            ++itr;
        }
    }
}
#endif