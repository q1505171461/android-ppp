//
// Created by juntao on 2021/4/5.
//
#include "include/Adapter/CodeBias/CodeBiasAdapter.h"
#include "include/Controller/Controller.h"
#include "include/Controller/Const.h"
#include "include/Utils/Com/Com.h"
#include <android/log.h>
#define LOG_TAG "YourAppTag"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace bamboo;
CodeBiasAdapter::CodeBiasAdapter()
{
}
CodeBiasAdapter::~CodeBiasAdapter()
{
}

void CodeBiasAdapter::v_inputCodeBias(const char *cprn, const char *mode, double dval)
{
    std::unique_lock<std::mutex> lk(m_lock);
    if (m_mem.find(cprn) == m_mem.end())
        m_mem[cprn] = t_codebias();
    m_mem[cprn][mode] = dval;
}
bool CodeBiasAdapter::m_inquireCodeBias(bool bcommon, const char *cprn, const char *mode, double *dval)
{
    if (m_mem.find(cprn) == m_mem.end())
        return false;
    if (m_mem[cprn].find(mode) != m_mem[cprn].end())
    {
        if (dval)
            *dval = m_mem[cprn][mode];
        return true;
    }
    if (!bcommon)
        return false;
    /* try common correction */
    switch (cprn[0])
    {
    case 'G':
        if (strstr(mode, "1C"))
            return false;
        if (mode[0] == '1')
            return strstr(mode, "1W") ? false : m_inquireCodeBias(bcommon, cprn, "1W", dval);
        if (mode[0] == '2')
            return strstr(mode, "2W") ? false : m_inquireCodeBias(bcommon, cprn, "2W", dval);
        if (mode[0] == '5')
            return strstr(mode, "5X") ? false : m_inquireCodeBias(bcommon, cprn, "5X", dval);
        break;
    case 'R':
        if (mode[0] == '1')
            return strstr(mode, "1P") ? false : m_inquireCodeBias(bcommon, cprn, "1P", dval);
        if (mode[0] == '2')
            return strstr(mode, "2P") ? false : m_inquireCodeBias(bcommon, cprn, "2P", dval);
        break;
    case 'E':
        if (mode[0] == '1')
            return strstr(mode, "1C") ? false : m_inquireCodeBias(bcommon, cprn, "1C", dval);
        if (mode[0] == '5')
            return strstr(mode, "5Q") ? false : m_inquireCodeBias(bcommon, cprn, "5Q", dval);
        if (mode[0] == '6')
            return strstr(mode, "6C") ? false : m_inquireCodeBias(bcommon, cprn, "6C", dval);
        if (mode[0] == '7')
            return strstr(mode, "7Q") ? false : m_inquireCodeBias(bcommon, cprn, "7Q", dval);
        if (mode[0] == '8')
            return strstr(mode, "8Q") ? false : m_inquireCodeBias(bcommon, cprn, "8Q", dval);
        break;
    case 'C':
        if (mode[0] == '2')
            return strstr(mode, "2I") ? false : m_inquireCodeBias(bcommon, cprn, "2I", dval);
        if (mode[0] == '5')
            return strstr(mode, "5P") ? false : m_inquireCodeBias(bcommon, cprn, "5P", dval);
        if (mode[0] == '6')
            return strstr(mode, "6I") ? false : m_inquireCodeBias(bcommon, cprn, "6I", dval);
        if (mode[0] == '7')
            return strstr(mode, "7I") ? false : m_inquireCodeBias(bcommon, cprn, "7I", dval);
        break;
    case 'J':
        if (mode[0] == '1')
            return strstr(mode, "1X") ? false : m_inquireCodeBias(bcommon, cprn, "1X", dval);
        if (mode[0] == '2')
            return strstr(mode, "2X") ? false : m_inquireCodeBias(bcommon, cprn, "2X", dval);
        if (mode[0] == '5')
            return strstr(mode, "5X") ? false : m_inquireCodeBias(bcommon, cprn, "5X", dval);
    default:
        LOGPRINT("unknown gnss system %c", cprn[0]);
        exit(1);
    }
    return false;
}
void CodeBiasAdapter::m_corrCodeBias(int mjd, double sod, Rnxobs *sit)
{
    std::unique_lock<std::mutex> lk(m_lock);
    int isat, isys, ifreq;
    double biass;
    char type[256];
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    for (isat = 0; isat < dly->nprn; isat++)
    {
        // isys = index_string(SYS, dly->cprn[isat][0]);
        isys = index_string(SYS, dly->prn_alias[isat][0]);
        for (ifreq = 0; ifreq < dly->nfreq_obs[isys]; ifreq++)
        {
            if (sit->obs[isat][MAXFREQ + ifreq] != 0.0)
            {
                if (!m_inquireCodeBias(true, dly->cprn[isat].c_str(), sit->fob[isat][MAXFREQ + ifreq] + 1, &biass))
                {
                    LOGPRINT("cprn = %s, obs-type = %s no correction", dly->cprn[isat].c_str(), sit->fob[isat][MAXFREQ + ifreq] + 1);
                    LOGD("cprn = %s, obs-type = %s no correction", dly->cprn[isat].c_str(), sit->fob[isat][MAXFREQ + ifreq] + 1);
                    memset(sit->obs[isat], 0, sizeof(double) * 2 * MAXFREQ);
                    for (ifreq = 0; ifreq < MAXFREQ; ++ifreq)
                        sit->obsstat[isat][ifreq] = DEL_LACK_DCB;
                    continue;
                }
                double lam = VEL_LIGHT / freqbytp(isat, dly->freq_obs[isys][ifreq]); // c/f
                sit->obs[isat][MAXFREQ + ifreq] += biass;
            }
        }
    }
}
int CodeBiasAdapter::inquireDCB(int mjd, double sod, int isat, const char *obt1, const char *obt2, double &bias)
{
    std::unique_lock<std::mutex> lk(m_lock);
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    double dval1, dval2;
    if (m_inquireCodeBias(false, dly->cprn[isat].c_str(), obt1 + 1, &dval1) && m_inquireCodeBias(false, dly->cprn[isat].c_str(), obt2 + 1, &dval2))
    {
        bias = (dval2 - dval1) / VEL_LIGHT * 1e9; // change it into ns
        return true;
    }
    return false;
}
