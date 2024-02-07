//
// Created by juntao on 2021/4/1.
//
#include "include/Adapter/Upd/UpdAdapter.h"
#include "include/Controller/Controller.h"
#include "include/Utils/Com/Com.h"
using namespace bamboo;
Upd_bias::Upd_bias()
{
    tpy = t_UPD_bias;
    m_reset();
}
void Upd_bias::m_reset()
{
    mjd = 0;
    sod = 0.0;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    for (int isat = 0; isat < dly->nprn; ++isat)
    {
        for (int ifreq = 0; ifreq < MAXFREQ; ++ifreq)
        {
            bias[isat][ifreq] = INVALID_UPD;
            xsig[isat][ifreq] = INVALID_UPD;
        }
    }
    for (int isys = 0; isys < MAXSYS; ++isys)
        refsat[isys] = -1;
}
bool Upd_bias::v_checkSatellite(const char *cprn, const char *freq)
{
    int isys, isat, ifreq;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    if (-1 == (isat = pointer_string(dly->nprn, dly->cprn, cprn)))
        return false;
    isys = index_string(SYS, dly->prn_alias[isat][0]);
    if (-1 == (ifreq = bbo_if(isys, freq)))
        return false;
    if (bias[isat][ifreq] == INVALID_UPD)
        return false;
    return true;
}
Upd_comb::Upd_comb()
{
    tpy = t_UPD_comb;
    m_reset();
}
void Upd_comb::m_reset()
{
    mjd = 0;
    sod = 0.0;
    for (int isat = 0; isat < MAXSAT; isat++)
    {
        comb_bias[isat] = INVALID_UPD;
        comb_xsig[isat] = INVALID_UPD;
    }
    for (int isys = 0; isys < MAXSYS; ++isys)
        refsat[isys] = -1;
    memset(ptime, 0, sizeof(ptime));
}
bool Upd_comb::m_validComb(char csys)
{
    int isys = index_string(SYS, csys);
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    for (int isat = 0; isat < MAXSAT; isat++)
    {
        if (dly->prn_alias[isat][0] != csys)
            continue;
        if (v_checkSatellite(dly->cprn[isat].c_str(), ""))
            return true;
    }
    return false;
}
bool Upd_comb::v_checkSatellite(const char *cprn, const char *freq)
{
    int isat;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    if (-1 == (isat = pointer_string(dly->nprn, dly->cprn, cprn)))
        return false;
    return !(comb_bias[isat] == INVALID_UPD);
}
UpdAdapter::UpdAdapter()
{
}
void UpdAdapter::m_inquireBias(int mjd, double sod, Upd_bias &bias)
{
    /* get the corresponding bias from the file */
    double dt, dtmin = 9e9;
    Upd_bias *sel = NULL;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    std::unique_lock<std::mutex> lk(m_lock);
    if (m_updbias.empty())
    {
        return;
    }
    bias = m_updbias.front();
    for (auto &v : m_updbias)
    {
        dt = fabs(timdif(mjd, sod, v.mjd, v.sod));
        if (dt < dtmin)
        {
            dtmin = dt;
            sel = &v;
        }
    }
    if (sel)
        bias = *sel;
    return;
}
void UpdAdapter::m_inquireComb(int mjd, double sod, const char *tp, const char *mode, Upd_comb &comb)
{
    /* get the corresponding comb bias from the memory */
    double dtmin = 9e9, dt;
    Upd_comb *sel = NULL;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    std::unique_lock<std::mutex> lk(m_lock);
    if (m_tp2m.find(tp) == m_tp2m.end())
    {
        return;
    }
    t_combmp &m_updcomb = m_tp2m[tp];
    if (m_updcomb.find(mode) == m_updcomb.end())
    {
        return;
    }
    list<Upd_comb> &comb_list = m_updcomb[mode];
    for (auto &v : comb_list)
    {
        dt = fabs(timdif(mjd, sod, v.mjd, v.sod));
        if (dt < dtmin)
        {
            dtmin = dt;
            sel = &v;
        }
    }
    if (sel)
        comb = *sel;
    return;
}
void UpdAdapter::v_inputBias(Upd_bias &bias)
{
    int isat, ifreq, isys, bvalid;
    double f1, f2, fac[2], gama, lamdn;
    std::unique_lock<std::mutex> lk(m_lock);
    list<Upd_bias>::iterator updItr;
    list<Upd_comb>::iterator combItr;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    /* step 1, remove the overdelay tag */
    for (updItr = m_updbias.begin(); updItr != m_updbias.end();)
    {
        if (timdif(bias.mjd, bias.sod, (*updItr).mjd, (*updItr).sod) > 86400.0 * 3)
        {
            updItr = m_updbias.erase(updItr);
            continue;
        }
        ++updItr;
    }
    m_updbias.push_back(bias);
}
void UpdAdapter::v_inputComb(const char *tp, const char *mode, Upd_comb &comb)
{
    std::unique_lock<std::mutex> lk(m_lock);
    list<Upd_comb>::iterator combItr;
    if (m_tp2m.find(tp) == m_tp2m.end())
        m_tp2m[tp] = t_combmp();
    t_combmp &m_updcomb = m_tp2m[tp];
    if (m_updcomb.find(mode) == m_updcomb.end())
        m_updcomb[mode] = list<Upd_comb>();
    list<Upd_comb> &kv = m_updcomb[mode];
    for (combItr = kv.begin(); combItr != kv.end();)
    {
        if (timdif(comb.mjd, comb.sod, (*combItr).mjd, (*combItr).sod) > 86400 * 3.0 && kv.size() > 1)
        {
            combItr = kv.erase(combItr);
            continue;
        }
        ++combItr;
    }
    kv.push_back(comb);
}