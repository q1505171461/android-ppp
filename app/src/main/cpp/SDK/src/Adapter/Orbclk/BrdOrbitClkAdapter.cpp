//
// Created by juntao, at wuhan university   on 2020/11/2.
//
#include "include/Adapter/Orbclk/BrdOrbitClkAdapter.h"
#include "include/Utils/Com/Com.h"
#include "include/Controller/Controller.h"
#include "include/Utils/Com/Taiutc.h"
using namespace bamboo;
BrdOrbitClkAdapter::BrdOrbitClkAdapter()
{
    this->orbType = ORB_BRD;
    this->clkType = CLK_BRD;
}
int BrdOrbitClkAdapter::v_readClk(const char *cprn, int mjd, double sod, double *sclk)
{
    // using rinex navigation file to acquire satellite clock
    int wk, iode = -1, ret;
    double sow, tgd, dtmin;
    *sclk = 0.0;
    mjd2wksow(mjd, sod, &wk, &sow);
    std::unique_lock<std::mutex> lock(q_mutex);
    if (cprn[0] != 'R')
        if (cprn[0] == 'C' || cprn[0] == 'B')
            ret = this->m_brd2xyz("ynn", cprn, wk, sow - 14.0, NULL, sclk, &dtmin, &tgd, &iode);
        else
            ret = this->m_brd2xyz("ynn", cprn, wk, sow, NULL, sclk, &dtmin, &tgd, &iode);
    else
        ret = this->m_gls2xyz("ynn", cprn, wk, sow, NULL, sclk, &dtmin, &iode);
    return ret;
}
int BrdOrbitClkAdapter::v_readClk(const char *cprn, int mjd, double sod, double *sclk, int *iode)
{
    int wk, ret;
    double sow, tgd, dtmin;
    *sclk = 0.0;
    mjd2wksow(mjd, sod, &wk, &sow);
    std::unique_lock<std::mutex> lock(q_mutex);
    if (cprn[0] != 'R')
        if (cprn[0] == 'C' || cprn[0] == 'B')
            ret = this->m_brd2xyz("ynn", cprn, wk, sow - 14.0, NULL, sclk, &dtmin, &tgd, iode);
        else
            ret = this->m_brd2xyz("ynn", cprn, wk, sow, NULL, sclk, &dtmin, &tgd, iode);
    else
        ret = this->m_gls2xyz("ynn", cprn, wk, sow, NULL, sclk, &dtmin, iode);
    return ret;
}
int BrdOrbitClkAdapter::v_readOrbit(const char *cprn, int mjd, double sod, double *xsat)
{
    // using rinex navigation file to acquire orbit
    int wk, iode = -1, ret;
    double sow, tgd, dtmin, sclk;
    mjd2wksow(mjd, sod, &wk, &sow);
    memset(xsat, 0, sizeof(double) * 6);
    std::unique_lock<std::mutex> lock(q_mutex);
    if (cprn[0] != 'R')
        if (cprn[0] == 'C' || cprn[0] == 'B')
            ret = this->m_brd2xyz("yyy", cprn, wk, sow - 14.0, xsat, &sclk, &dtmin, &tgd, &iode);
        else
            ret = this->m_brd2xyz("yyy", cprn, wk, sow, xsat, &sclk, &dtmin, &tgd, &iode);
    else
        ret = this->m_gls2xyz("yyy", cprn, wk, sow, xsat, &sclk, &dtmin, &iode);

    return ret;
}
int BrdOrbitClkAdapter::v_readOrbit(const char *cprn, int mjd, double sod, double *xsat, int *iode)
{
    // using rinex navigation file to acquire orbit
    int wk, ret;
    double sow, tgd, dtmin, sclk;
    mjd2wksow(mjd, sod, &wk, &sow);
    memset(xsat, 0, sizeof(double) * 6);
    std::unique_lock<std::mutex> lock(q_mutex);
    if (cprn[0] != 'R')
        if (cprn[0] == 'C' || cprn[0] == 'B')
            ret = this->m_brd2xyz("yyy", cprn, wk, sow - 14.0, xsat, &sclk, &dtmin, &tgd, iode);
        else
            ret = this->m_brd2xyz("yyy", cprn, wk, sow, xsat, &sclk, &dtmin, &tgd, iode);
    else
        ret = this->m_gls2xyz("yyy", cprn, wk, sow, xsat, &sclk, &dtmin, iode);

    return ret;
}
int BrdOrbitClkAdapter::v_readClkDrift(const char *cprn, int mjd, double sod, double *vclk)
{
    int iode = -1;
    double sclk, sclk_inc, inc = 0.01;
    if (BrdOrbitClkAdapter::v_readClk(cprn, mjd, sod, &sclk, &iode))
    {
        if (BrdOrbitClkAdapter::v_readClk(cprn, mjd, sod + 0.01, &sclk_inc, &iode))
        {
            *vclk = (sclk_inc - sclk) / 0.01;
            return 1;
        }
    }
    return 0;
}
void BrdOrbitClkAdapter::v_openOutFile(string cmd, double ver)
{
    // 15min interval
    char hours[] = "abcdefghijklmnopqrstuvwx";
    const char *mins[] = {"00", "15", "30", "45"};
    int curmjd, iy, im, id, ih, imi, idoy, imt, outheader = false;
    double cursod, dsec;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    Logtrace::s_defaultlogger.m_closeLog(outFile); /// Close the early file
    curmjd = atoi(cmd.substr(0, index_string(cmd.c_str(), ':')).c_str());
    cursod = atof(cmd.substr(index_string(cmd.c_str(), ':') + 1).c_str());
    mjd2date(curmjd, cursod, &iy, &im, &id, &ih, &imi, &dsec);
    mjd2doy(curmjd, &iy, &idoy);
    imt = imi / 15;
    Logtrace::s_defaultlogger.m_closeLog(outFile);
    sprintf(outFile, "%s/brdm%03d%c%2s.%02dp", dly->outdir, idoy, hours[ih], mins[imt], iy - 2000);
    if (-1 == access(outFile, 0))
    {
        outheader = true;
    }
    Logtrace::s_defaultlogger.m_openLog(outFile, true);
    this->isOutputOpen = true;
    if (ver > 3.0)
        this->outVer = ver;
    else
        cout << "***WARNING(RnxEphStreamAdapter):v_openOutFile version " << ver << " is not support,use default " << outVer << " instead!" << endl;
    if (outheader)
        m_outFileHeader();
}
void BrdOrbitClkAdapter::v_closeOutFile()
{
    this->isOutputOpen = false;
    Logtrace::s_defaultlogger.m_closeLog(outFile);
}
void BrdOrbitClkAdapter::m_outFileHeader()
{
    char tmebuf[256];
    Logtrace::s_defaultlogger.m_wtMsg("@%s      %4.2lf           NAVIGATION DATA     M (Mixed)           RINEX VERSION / TYPE\r\n", outFile, outVer);
    Logtrace::s_defaultlogger.m_wtMsg("@%s BambooHoursMerge         WHU-GRC           %sPGM / RUN BY / DATE\r\n", outFile, runtime(tmebuf));
    Logtrace::s_defaultlogger.m_wtMsg("@%s Merged GPS/GLO/GAL/BDS/ navigation file                     COMMENT\r\n", outFile);
    Logtrace::s_defaultlogger.m_wtMsg("@%s                                                             END OF HEADER\r\n", outFile);
}
map<string, GPSEPH> BrdOrbitClkAdapter::v_getCurrentGpsEph(double ptime)
{
    std::unique_lock<std::mutex> lock(q_mutex);
    map<string, GPSEPH> r = m_getCurrentGpsEph(ptime);
    return r;
}
map<string, GLSEPH> BrdOrbitClkAdapter::v_getCurrentGlsEph(double ptime)
{
    std::unique_lock<std::mutex> lock(q_mutex);
    map<string, GLSEPH> r = m_getCurrentGlsEph(ptime);
    return r;
}
void BrdOrbitClkAdapter::v_outFileEph()
{
    int isys, iy, im, id, ih, imin, mjd;
    double sec, sod, week;
    list<GPSEPH>::iterator gpsItr;
    list<GLSEPH>::iterator glsItr;
    std::unique_lock<std::mutex> lock(q_mutex);
    for (auto &kv : gnssEph)
    {
        for (gpsItr = kv.second.begin(); gpsItr != kv.second.end(); ++gpsItr)
        {
            // Output the broadcast navigation message
            week = (*gpsItr).week;
            isys = index_string(SYS, (*gpsItr).cprn[0]);
            if (SYS[isys] == 'C')
                week = week - 1356;
            mjd2date((*gpsItr).mjd, (*gpsItr).sod, &iy, &im, &id, &ih, &imin, &sec);
            Logtrace::s_defaultlogger.m_wtMsg("@%s %3s %04d %02d %02d %02d %02d %02d%19.12e%19.12e%19.12e\r\n", outFile, (*gpsItr).cprn, iy, im, id, ih, imin, (int)sec, (*gpsItr).a0,
                                              (*gpsItr).a1, (*gpsItr).a2);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (double)(*gpsItr).aode, (*gpsItr).crs, (*gpsItr).dn, (*gpsItr).m0);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*gpsItr).cuc, (*gpsItr).e, (*gpsItr).cus, (*gpsItr).roota);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*gpsItr).toe, (*gpsItr).cic, (*gpsItr).omega0, (*gpsItr).cis);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*gpsItr).i0, (*gpsItr).crc, (*gpsItr).omega, (*gpsItr).omegadot);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*gpsItr).idot, (*gpsItr).resvd0, week, (*gpsItr).resvd1);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*gpsItr).accu, (*gpsItr).hlth, (*gpsItr).tgd, SYS[isys] == 'C' ? (*gpsItr).tgd1 : (*gpsItr).aodc);

            if (SYS[isys] == 'E')
                Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e\r\n", outFile, 9.999e8);
            else
                Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e\r\n", outFile, 0.0, 0.0);
        }
    }
    for (auto &kv : glsEph)
    {
        for (glsItr = kv.second.begin(); glsItr != kv.second.end(); ++glsItr)
        {
            timinc((*glsItr).mjd, (*glsItr).sod, OFF_GPS2TAI - Taiutc::s_getInstance()->m_getTaiutc((*glsItr).mjd),
                   &mjd, &sod);
            mjd2date(mjd, sod, &iy, &im, &id, &ih, &imin, &sec);
            Logtrace::s_defaultlogger.m_wtMsg("@%s %3s %04d %02d %02d %02d %02d %02d%19.12e%19.12e%19.12e\r\n", outFile,
                                              (*glsItr).cprn, iy, im, id, ih, imin, (int)sec, (*glsItr).tau,
                                              (*glsItr).gamma, (*glsItr).tk);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*glsItr).pos[0],
                                              (*glsItr).vel[0],
                                              (*glsItr).acc[0], (*glsItr).health);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*glsItr).pos[1],
                                              (*glsItr).vel[1],
                                              (*glsItr).acc[1], (*glsItr).frenum);
            Logtrace::s_defaultlogger.m_wtMsg("@%s     %19.12e%19.12e%19.12e%19.12e\r\n", outFile, (*glsItr).pos[2],
                                              (*glsItr).vel[2],
                                              (*glsItr).acc[2], (*glsItr).age);
        }
    }
}
// the time check is performed outside this function
void BrdOrbitClkAdapter::v_inputEph(GPSEPH *ephgps, GLSEPH *ephgls)
{
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    /* 去除非整秒星历 */
    if (ephgps && fmod(ephgps->sod, 300.0))
        return;
    if (dly->lpost)
        m_inputEphPost(ephgps, ephgls);
    else
        m_inputEphRealTime(ephgps, ephgls);
}
void BrdOrbitClkAdapter::m_inputEphPost(GPSEPH *ephgps, GLSEPH *ephgls)
{
    int lexist, isys, mjd_now;
    double dt, ptmax = 0.0, sod_now;
    std::unique_lock<std::mutex> lock(q_mutex);
    if (ephgps != NULL)
    {
        // add the list here,
        lexist = false;
        isys = index_string(SYS, ephgps->cprn[0]);
        list<GPSEPH>::iterator gpsItr;
        ptmax = 0.0;
        ephgps->aode = genAode(SYS[isys], ephgps->mjd, ephgps->sod, ephgps->toe, ephgps->aode, ephgps);
        if (gnssEph.find(ephgps->cprn) == gnssEph.end())
            gnssEph[ephgps->cprn] = list<GPSEPH>();
        list<GPSEPH> &eph_list = gnssEph[ephgps->cprn];
        lexist = false;
        for (gpsItr = eph_list.begin(); gpsItr != eph_list.end(); ++gpsItr)
        {
            if (ptmax < (*gpsItr).mjd + (*gpsItr).sod / 86400.0)
                ptmax = (*gpsItr).mjd + (*gpsItr).sod / 86400.0; /*find the max time tag of the ephemeris*/
        }

        for (gpsItr = eph_list.begin(); gpsItr != eph_list.end();)
        {
            if ((ptmax - (*gpsItr).mjd) * 86400.0 - (*gpsItr).sod > 86400.0)
            { /* one day interval */
                gpsItr = eph_list.erase(gpsItr);
                continue;
            }
            if (strncmp((*gpsItr).cprn, ephgps->cprn, 3) == 0)
            {
                dt = ((*gpsItr).mjd - ephgps->mjd) * 86400.0 + (*gpsItr).sod - ephgps->sod;
                if (fabs(dt) < 1e-3 && (*gpsItr).aode == ephgps->aode)
                {
                    lexist = true;
                    break;
                }
            }
            ++gpsItr;
        }
        if (ephgps->cprn[0] == 'E')
        {
            /** galileo Data sources
             * Bit 0 set: I/NAV E1-B
             * Bit 1 set: F/NAV E5a-I
             * Bit 2 set: I/NAV E5b-I
             * **/
            /**galileo system here**/
            int code = (int)ephgps->resvd0;
            if (!(code & (1 << 1)))
            { /**means this is a inav,will continue**/
                lexist = true;
            }
        }
        if (!lexist)
        {
            eph_list.push_front(*ephgps);
        }
    }
    if (ephgls != NULL)
    {
        lexist = false;
        list<GLSEPH>::iterator glsItr;
        isys = index_string(SYS, ephgls->cprn[0]);
        ptmax = 0.0;
        ephgls->aode = genAode(SYS[isys], ephgls->mjd, ephgls->sod, 0.0, ephgls->aode, NULL);
        if (glsEph.find(ephgls->cprn) == glsEph.end())
            glsEph[ephgls->cprn] = list<GLSEPH>();
        list<GLSEPH> &eph_list = glsEph[ephgls->cprn];
        for (glsItr = eph_list.begin(); glsItr != eph_list.end(); ++glsItr)
        {
            if (ptmax < (*glsItr).mjd + (*glsItr).sod / 86400.0)
                ptmax = (*glsItr).mjd + (*glsItr).sod / 86400.0; /*find the max time tag of the ephemeris*/
        }
        for (glsItr = eph_list.begin(); glsItr != eph_list.end();)
        {
            if ((ptmax - (*glsItr).mjd) * 86400.0 - (*glsItr).sod > 86400.0)
            { /* one day interval */
                glsItr = eph_list.erase(glsItr);
                continue;
            }
            if (strncmp((*glsItr).cprn, ephgls->cprn, 3) == 0)
            {
                dt = ((*glsItr).mjd - ephgls->mjd) * 86400.0 + (*glsItr).sod - ephgls->sod;
                if (fabs(dt) < 1e-3)
                {
                    lexist = true;
                    break;
                }
            }
            ++glsItr;
        }
        if (!lexist)
        {
            eph_list.push_back(*ephgls);
        }
    }
}
void BrdOrbitClkAdapter::m_inputEphRealTime(GPSEPH *ephgps, GLSEPH *ephgls)
{
    /// input the real-time ephemeris
    int isys, lexist, curmjd;
    double cursod;
    std::unique_lock<std::mutex> lock(q_mutex);
    if (ephgps != NULL)
    {
        isys = index_string(SYS, ephgps->cprn[0]);
        ephgps->aode = genAode(SYS[isys], ephgps->mjd, ephgps->sod, ephgps->toe, ephgps->aode, ephgps);
        if (gnssEph.find(ephgps->cprn) == gnssEph.end())
            gnssEph[ephgps->cprn] = list<GPSEPH>();
        list<GPSEPH> &eph_list = gnssEph[ephgps->cprn];
        /// check whether exists
        lexist = false;
        for (auto &eph : eph_list)
        {
            if (fabs(timdif(eph.mjd, eph.sod, ephgps->mjd, ephgps->sod)) < MAXWND)
            {
                lexist = true;
                break;
            }
        }
        if (ephgps->cprn[0] == 'E')
        {
            /** galileo Data sources
             * Bit 0 set: I/NAV E1-B
             * Bit 1 set: F/NAV E5a-I
             * Bit 2 set: I/NAV E5b-I
             * **/
            /**galileo system here**/
            int code = (int)ephgps->resvd0;
            if (!(code & (1 << 1)))
            { /**means this is a inav,will continue**/
                lexist = true;
            }
        }
        cur_time(&curmjd, &cursod);
        if (fabs((curmjd - ephgps->mjd) * 86400.0 + cursod - ephgps->sod) > 21600.0)
        { // 3 hours{
            LOGSTATINFO("receving wrong time of ephemeris:%s eph:%d %8.1lf cur:%d %8.1lf", ephgps->cprn, ephgps->mjd, ephgps->sod, curmjd, cursod);
            lexist = true;
        }
        if (!lexist)
        {
            if (eph_list.empty())
            {
                eph_list.push_back(*ephgps);
            }
            else if (eph_list.size() == 1)
            {
                /// only one ephemeris
                GPSEPH &ft = eph_list.front();
                if (timdif(ephgps->mjd, ephgps->sod, ft.mjd, ft.sod) > MAXWND)
                {
                    eph_list.push_front(*ephgps);
                }
                else if (timdif(ephgps->mjd, ephgps->sod, ft.mjd, ft.sod) < -MAXWND)
                {
                    eph_list.push_back(*ephgps);
                }
            }
            else
            {
                /// there is two ephemeris in the memory, just compare the time
                GPSEPH &ft = eph_list.front();
                GPSEPH &fb = eph_list.back();
                double dt_t = timdif(ephgps->mjd, ephgps->sod, ft.mjd, ft.sod);
                double dt_b = timdif(ephgps->mjd, ephgps->sod, fb.mjd, fb.sod);
                if (dt_t > MAXWND)
                {
                    eph_list.pop_back();
                    eph_list.push_front(*ephgps);
                }
                else if (dt_t < -MAXWND && dt_b > MAXWND)
                {
                    eph_list.pop_back();
                    eph_list.push_back(*ephgps);
                }
            }
        }
    }
    if (ephgls != NULL)
    {
        isys = index_string(SYS, ephgls->cprn[0]);
        ephgls->aode = genAode(SYS[isys], ephgls->mjd, ephgls->sod, 0.0, ephgls->aode, NULL);
        if (glsEph.find(ephgls->cprn) == glsEph.end())
            glsEph[ephgls->cprn] = list<GLSEPH>();
        list<GLSEPH> &eph_list = glsEph[ephgls->cprn];
        lexist = false;
        for (auto &eph : eph_list)
        {
            if (fabs(timdif(eph.mjd, eph.sod, ephgls->mjd, ephgls->sod)) < MAXWND)
            {
                lexist = true;
                break;
            }
        }
        cur_time(&curmjd, &cursod);
        if (fabs((curmjd - ephgls->mjd) * 86400.0 + cursod - ephgls->sod) > 21600.0)
        { // 3 hours
            LOGSTATINFO("receving wrong time of ephemeris:%s eph:%d %8.1lf cur:%d %8.1lf", ephgls->cprn, ephgls->mjd, ephgls->sod, curmjd, cursod);
            lexist = true;
        }
        if (!lexist)
        {
            if (eph_list.empty())
            {
                eph_list.push_back(*ephgls);
            }
            else if (eph_list.size() == 1)
            {
                /// only one ephemeris
                GLSEPH &ft = eph_list.front();
                if (timdif(ephgls->mjd, ephgls->sod, ft.mjd, ft.sod) > MAXWND)
                {
                    eph_list.push_front(*ephgls);
                }
                else if (timdif(ephgls->mjd, ephgls->sod, ft.mjd, ft.sod) < -MAXWND)
                {
                    eph_list.push_back(*ephgls);
                }
            }
            else
            {
                /// there is two ephemeris in the memory, just compare the time
                GLSEPH &ft = eph_list.front();
                GLSEPH &fb = eph_list.back();
                double dt_t = timdif(ephgls->mjd, ephgls->sod, ft.mjd, ft.sod);
                double dt_b = timdif(ephgls->mjd, ephgls->sod, fb.mjd, fb.sod);
                if (dt_t > MAXWND)
                {
                    eph_list.pop_back();
                    eph_list.push_front(*ephgls);
                }
                else if (dt_t < -MAXWND && dt_b > MAXWND)
                {
                    eph_list.pop_back();
                    eph_list.push_back(*ephgls);
                }
            }
        }
    }
}