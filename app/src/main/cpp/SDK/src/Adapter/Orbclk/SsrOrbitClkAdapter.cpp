//
// Created by juntao, at wuhan university   on 2020/11/3.
//
#include "include/Adapter/Orbclk/SsrOrbitClkAdapter.h"
#include "include/Controller/Controller.h"
#include "include/Utils/Com/Com.h"
using namespace bamboo;
SsrOrbitClkAdapter::SsrOrbitClkAdapter()
{
    ptime_chk = 0.0;
    this->orbType = ORB_SSR;
    this->clkType = CLK_SSR;
}
int SsrOrbitClkAdapter::v_readOrbit(const char *cprn, int mjd, double sod, double *xsat)
{
    int jsat, gwk, ifind = 0, i, bsearch = false;
    double dx[6], c[3], dt, gsow, dtref = 120.0, xsat_out[6];
    double A[3], R[3], Cr[3], length, dxx[6];
    list<Ssr_orbclk>::iterator orbItr;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();

    if (fabs(NINT((mjd - ptime_chk) * 86400.0) + NINT(sod)) > MAXWND)
    {
        bsearch = true;
    }
    memset(xsat, 0, sizeof(double) * 6);
    if (bsearch)
    {
        std::unique_lock<std::mutex> _lock(q_mutex);
        mjd2wksow(mjd, sod, &gwk, &gsow);
        for (orbItr = m_orbclks.begin(); orbItr != m_orbclks.end(); ++orbItr)
        {
            dt = fabs((gwk - (*orbItr).wk) * 7.0 * 86400.0 + gsow - (*orbItr).sow);
            if (dt < dtref)
            {
                dtref = dt;
                /*** update the ssr here ***/
                m_curSsr = *orbItr;
                ifind = 1;
                ptime_chk = mjd + NINT(sod) / 86400.0;
            }
        }
        if (ifind)
        {
            int mjd_tag;
            double sod_tag;
            wksow2mjd(m_curSsr.wk, m_curSsr.sow, &mjd_tag, &sod_tag);
            LOGINFO_TAG_EX("DEBUG SSR", "[%d %9.1lf] acquire ssr for clock [%d %9.1lf]", mjd, sod, mjd_tag, sod_tag);
        }
    }
    jsat = pointer_string(dly->nprn, dly->cprn, string(cprn));
    if (jsat == -1 || m_curSsr.iod_orb[jsat] == -1 || (bsearch && ifind == 0))
    {
        return 0;
    }
    if (!BrdOrbitClkAdapter::v_readOrbit(cprn, mjd, sod, xsat_out, &m_curSsr.iod_orb[jsat]))
        return 0;
    memcpy(dx, m_curSsr.dx[jsat], 6 * sizeof(double));

    unit_vector(3, xsat_out + 3, A, &length);
    unit_vector(3, xsat_out, Cr, &length);
    cross(Cr, A, Cr);
    unit_vector(3, Cr, Cr, &length);
    cross(A, Cr, R);
    unit_vector(3, R, R, &length);
    memset(dxx, 0, sizeof(dxx));
    dxx[0] = R[0] * dx[0] + A[0] * dx[1] + Cr[0] * dx[2];
    dxx[1] = R[1] * dx[0] + A[1] * dx[1] + Cr[1] * dx[2];
    dxx[2] = R[2] * dx[0] + A[2] * dx[1] + Cr[2] * dx[2];

    // velocity
    dxx[3] = R[0] * dx[3] + A[0] * dx[4] + Cr[0] * dx[5];
    dxx[4] = R[1] * dx[3] + A[1] * dx[4] + Cr[1] * dx[5];
    dxx[5] = R[2] * dx[3] + A[2] * dx[4] + Cr[2] * dx[5];

    for (i = 0; i < 6; i++)
    {
        xsat[i] = xsat_out[i] - dxx[i];
    }
    return 1;
}
int SsrOrbitClkAdapter::v_readOrbit(const char *cprn, int mjd, double sod, double *xsat, int *iode)
{
    return v_readOrbit(cprn, mjd, sod, xsat);
}
int SsrOrbitClkAdapter::v_readClk(const char *cprn, int mjd, double sod, double *xclk)
{
    int jsat, gwk, ifind = 0, bsearch = false;
    double dx[6], c[3], dt, gsow, dtref = 120.0, xclk_out, dclk;
    list<Ssr_orbclk>::iterator orbItr;
    Ssr_orbclk &clk = m_curSsr;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    *xclk = 0.0;
    if (fabs(NINT((mjd - ptime_chk) * 86400.0) + NINT(sod)) > MAXWND)
    {
        bsearch = true;
    }
    if (bsearch)
    {
        std::unique_lock<std::mutex> _lock(q_mutex);
        mjd2wksow(mjd, sod, &gwk, &gsow);
        for (orbItr = m_orbclks.begin(); orbItr != m_orbclks.end(); ++orbItr)
        {
            dt = fabs((gwk - (*orbItr).wk) * 7.0 * 86400.0 + gsow - (*orbItr).sow);
            if (dt < dtref)
            {
                dtref = dt;
                /*** update the ssr here ***/
                m_curSsr = *orbItr;
                ifind = 1;

                ptime_chk = mjd + NINT(sod) / 86400.0;
            }
        }
        if (ifind)
        {
            int mjd_tag;
            double sod_tag;
            wksow2mjd(m_curSsr.wk, m_curSsr.sow, &mjd_tag, &sod_tag);
            LOGINFO_TAG_EX("DEBUG SSR", "[%d %9.1lf] acquire ssr for clock [%d %9.1lf]", mjd, sod, mjd_tag, sod_tag);
        }
    }
    jsat = pointer_string(dly->nprn, dly->cprn, string(cprn));
    if (jsat == -1 || clk.iod_clk[jsat] == -1 || (bsearch && ifind == 0))
    {
        return 0;
    }
    if (!BrdOrbitClkAdapter::v_readClk(cprn, mjd, sod, &xclk_out, &clk.iod_clk[jsat]))
        return 0;
    memcpy(c, clk.c[jsat], 3 * sizeof(double));
    dclk = c[0] + dt * (c[1] + dt * c[2]);
    *xclk = xclk_out + dclk / VEL_LIGHT;

    // LOGPRINT("[%d %9.1lf] %s reading satellite clock %9.3lf", mjd, sod, dly->cprn[jsat].c_str(), *xclk * VEL_LIGHT);
    return 1;
}
int SsrOrbitClkAdapter::v_readClk(const char *cprn, int mjd, double sod, double *sclk, int *iode)
{
    return v_readClk(cprn, mjd, sod, sclk);
}
void SsrOrbitClkAdapter::v_inputSsr(SSRORB *orb, SSRCLK *clk)
{
    int mjd;
    double sod;
    list<Ssr_orbclk>::iterator itr;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    if (orb != NULL || clk != NULL)
    {
        std::unique_lock<std::mutex> _lock(q_mutex);
        if (orb)
        {
            wksow2mjd(orb->wk, orb->sow, &mjd, &sod);
            LOGPRINT_EX("input ssr orb for %d %lf", mjd, sod);
            Ssr_orbclk *orb_mem = NULL;
            for (itr = m_orbclks.begin(); itr != m_orbclks.end(); ++itr)
            {
                if (fabs(((*itr).wk - orb->wk) * 86400 * 7 + (*itr).sow - orb->sow) < MAXWND)
                {
                    orb_mem = &(*itr);
                    break;
                }
            }
            if (orb_mem != NULL)
            {
                /*** fill the orbit part,should not copy,but fill */
                orb_mem->wk = orb->wk;
                orb_mem->sow = orb->sow;
                for (int isat = 0; isat < dly->nprn; isat++)
                {
                    if (orb->iod[isat] != -1)
                    {
                        orb_mem->iod_orb[isat] = orb->iod[isat];
                        memcpy(orb_mem->dx[isat], orb->dx[isat], sizeof(double) * 6);
                    }
                }
            }
            else
            {
                Ssr_orbclk Ssr_orbclk;
                Ssr_orbclk.wk = orb->wk;
                Ssr_orbclk.sow = orb->sow;
                memcpy(Ssr_orbclk.dx, orb->dx, sizeof(orb->dx));
                memcpy(Ssr_orbclk.iod_orb, orb->iod, sizeof(orb->iod));
                m_orbclks.push_back(Ssr_orbclk);
            }
        }
        if (clk)
        {
            wksow2mjd(clk->wk, clk->sow, &mjd, &sod);
            LOGPRINT("input ssr clk for %d %lf", mjd, sod);

            Ssr_orbclk *clk_mem = NULL;
            for (itr = m_orbclks.begin(); itr != m_orbclks.end(); ++itr)
            {
                if (fabs(((*itr).wk - clk->wk) * 86400 * 7 + (*itr).sow - clk->sow) < MAXWND)
                {
                    clk_mem = &(*itr);
                    break;
                }
            }
            if (clk_mem)
            {
                /*** fill the clock part **/
                clk_mem->wk = clk->wk;
                clk_mem->sow = clk->sow;
                for (int isat = 0; isat < dly->nprn; isat++)
                {
                    if (clk->iod[isat] != -1)
                    {
                        clk_mem->iod_clk[isat] = clk->iod[isat];
                        clk_mem->yaw[isat] = clk->yaw[isat];
                        clk_mem->yrate[isat] = clk->yrate[isat];
                        memcpy(clk_mem->c[isat], clk->c[isat], sizeof(double) * 3);
                    }
#ifdef USEEIGEN
                    if (clk_mem->c2Q.find(dly->cprn[isat]) == clk_mem->c2Q.end() && clk->c2Q.find(dly->cprn[isat]) != clk->c2Q.end())
                    {
                        clk_mem->c2Q[dly->cprn[isat]] = clk->c2Q[dly->cprn[isat]];
                    }
#endif
                }
            }
            else
            {
                Ssr_orbclk Ssr_orbclk;
                Ssr_orbclk.wk = clk->wk;
                Ssr_orbclk.sow = clk->sow;
                memcpy(Ssr_orbclk.c, clk->c, sizeof(clk->c));
                memcpy(Ssr_orbclk.iod_clk, clk->iod, sizeof(clk->iod));
                memcpy(Ssr_orbclk.yaw, clk->yaw, sizeof(clk->yaw));
                memcpy(Ssr_orbclk.yrate, clk->yrate, sizeof(clk->yrate));
#ifdef USEEIGEN
                Ssr_orbclk.c2Q = clk->c2Q;
#endif
                m_orbclks.push_back(Ssr_orbclk);
            }
        }
        if (m_orbclks.size() > MAXOC)
        {
            itr = m_orbclks.begin();
            while (m_orbclks.size() > MAXOC)
            {
                itr = m_orbclks.erase(itr);
            }
        }
    }
}
void SsrOrbitClkAdapter::v_openOutFile(string cmd, double ver)
{
    int curmjd, iyear, imon, iday, ih, imin, iwk, idow;
    double cursod, dsec;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    memset(f_out, 0, sizeof(f_out));
    BrdOrbitClkAdapter::v_openOutFile(cmd, ver);
    curmjd = atoi(cmd.substr(0, index_string(cmd.c_str(), ':')).c_str());
    cursod = atof(cmd.substr(index_string(cmd.c_str(), ':') + 1).c_str());
    mjd2date(curmjd, cursod, &iyear, &imon, &iday, &ih, &imin, &dsec);
    gpsweek(iyear, imon, iday, &iwk, &idow);
    sprintf(f_out, "%s/ssr%04d%01d.ssr", dly->outdir, iwk, idow);
    Logtrace::s_defaultlogger.m_openLog(f_out);
}
void SsrOrbitClkAdapter::v_outFileEph()
{
    int iyear, imon, iday, ih, imin, mjd, nsav, isat;
    double dsec, sod;
    BrdOrbitClkAdapter::v_outFileEph();
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    for (const auto &v : m_orbclks)
    {
        wksow2mjd(v.wk, v.sow, &mjd, &sod);
        mjd2date(mjd, sod, &iyear, &imon, &iday, &ih, &imin, &dsec);
        for (nsav = 0, isat = 0; isat < dly->nprn; isat++)
        {
            if (v.iod_orb[isat] != -1)
                ++nsav;
        }
        Logtrace::s_defaultlogger.m_wtMsg("@%s > ORBIT %04d %02d %02d %02d %02d %4.1lf %d SSR\n", f_out, iyear, imon, iday, ih, imin, dsec, nsav);
        for (isat = 0; isat < dly->nprn; isat++)
        {
            if (v.iod_orb[isat] == -1)
                continue;
            Logtrace::s_defaultlogger.m_wtMsg("@%s %s  %10d%11.4lf%11.4lf%11.4lf%11.4lf%11.4lf%11.4lf\n", f_out, dly->cprn[isat].c_str(),
                                              v.iod_orb[isat], v.dx[isat][0], v.dx[isat][1], v.dx[isat][2], v.dx[isat][3], v.dx[isat][4], v.dx[isat][5]);
        }
        for (nsav = 0, isat = 0; isat < dly->nprn; isat++)
        {
            if (v.iod_clk[isat] != -1)
                ++nsav;
        }
        Logtrace::s_defaultlogger.m_wtMsg("@%s > CLOCK %04d %02d %02d %02d %02d %4.1lf %d SSR\n", f_out, iyear, imon, iday, ih, imin, dsec, nsav);
        for (isat = 0; isat < dly->nprn; isat++)
        {
            if (v.iod_orb[isat] == -1)
                continue;
            Logtrace::s_defaultlogger.m_wtMsg("@%s %s  %10d%11.4lf%11.4lf%11.4lf\n", f_out, dly->cprn[isat].c_str(),
                                              v.iod_clk[isat], v.c[isat][0], v.c[isat][1], v.c[isat][2]);
        }
    }
}
void SsrOrbitClkAdapter::v_closeOutFile()
{
    BrdOrbitClkAdapter::v_closeOutFile();
    Logtrace::s_defaultlogger.m_closeLog(f_out);
}
