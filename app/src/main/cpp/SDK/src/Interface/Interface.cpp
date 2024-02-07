#include "include/Adapter/Ifpb/t_ifpb.h"
#include "include/Controller/Rnxobs.h"
#include "include/Adapter/Orbclk/OrbitClkAdapter.h"
#include "include/Adapter/Upd/UpdAdapter.h"
#include "include/Adapter/CodeBias/CodeBiasAdapter.h"
#include "include/Controller/Controller.h"
#include "include/Rtklib/rtklib_fun.h"
#include "include/Utils/Com/Com.h"
#include "include/Interface/Interface.h"
#include "include/Interface/LoggerSsr.h"
#include <cstdlib>



int KPL_isInitialize = false;
const char * KPL_config_path = "./";
static void s_setObsData(KPL_IO::gtime_t tm, KPL_IO::obs_t *obs, bamboo::Rnxobs *pack)
{
    char cprn[256] = {0}, obcode[256] = {0}, cfq;
    int isat, jsat, n = obs->n, week, ifreq, jfreq, isys;
    double sow;
    sow = KPL_IO::time2gpst(tm, &week);
    bamboo::wksow2mjd(week, sow, &pack->mjd, &pack->sod);
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    for (isat = 0; isat < n; isat++)
    {
        KPL_IO::satno2id(obs->data[isat].sat, cprn);
        if (-1 == (jsat = bamboo::pointer_string(dly->nprn, dly->cprn, cprn)))
            continue;
        isys = bamboo::index_string(bamboo::SYS, dly->prn_alias[jsat][0]);
        for (ifreq = 0; ifreq < NFREQ; ifreq++)
        {
            strcpy(obcode, KPL_IO::code2obs(obs->data[isat].code[ifreq]));
            for (jfreq = 0; jfreq < dly->nfreq_obs[isys]; jfreq++)
            {
                cfq = dly->freq_obs[isys][jfreq][1];
                if (cfq == obcode[0])
                {
                    pack->obs[jsat][jfreq] = obs->data[isat].L[ifreq];
                    pack->obs[jsat][bamboo::MAXFREQ + jfreq] = obs->data[isat].P[ifreq];
                    pack->snr[jsat][jfreq] = obs->data[isat].SNR[ifreq] * SNR_UNIT;
                    pack->dop[jsat][jfreq] = -obs->data[isat].D[ifreq];
                    sprintf(pack->fob[jsat][jfreq], "L%s", obcode);
                    sprintf(pack->fob[jsat][bamboo::MAXFREQ + jfreq], "C%s", obcode);
                    break;
                }
            }
        }
    }
}
int flog() {
    // 打开一个文件以写入内容
    std::ofstream outputFile("example.txt");

    // 检查文件是否成功打开
    if (outputFile.is_open()) {
        // 写入文本内容
        outputFile << "Hello, World!\n";
        outputFile << "This is a C++ file write example.\n";

        // 关闭文件
        outputFile.close();
        std::cout << "File write successful.\n";
    } else {
        std::cerr << "Unable to open the file.\n";
    }

    return 0;
}
static void s_setEphData(KPL_IO::nav_t *nav, int psat, int offset)
{
    int isat, isys, mjd, week;
    char prn[256] = {0};
    double sow, sod;
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    KPL_IO::satno2id(psat, prn);
    if (!(strlen(prn)) || -1 == (isys = bamboo::index_string(bamboo::SYS, prn[0])) || -1 == (isat = bamboo::pointer_string(dly->nprn, dly->cprn, string(prn))))
    {
        return;
    }
    if (bamboo::SYS[isys] != 'R')
    {
        bamboo::GPSEPH ephg;
        psat = psat + offset;
        if (bamboo::SYS[isys] == 'C')
        {
            nav->eph[psat - 1].toc.time -= 14; /*change it into bdt*/
            nav->eph[psat - 1].toe.time -= 14;
        }
        sow = KPL_IO::time2gpst(nav->eph[psat - 1].toc, &week); /***Time in GPST****/
        bamboo::wksow2mjd(week, sow, &mjd, &sod);
        strcpy(ephg.cprn, prn);
        ephg.mjd = mjd; // Time in GPS
        ephg.sod = sod;
        ephg.a0 = nav->eph[psat - 1].f0;
        ephg.a1 = nav->eph[psat - 1].f1;
        ephg.a2 = nav->eph[psat - 1].f2;
        ephg.aode = nav->eph[psat - 1].iode;
        ephg.crs = nav->eph[psat - 1].crs;
        ephg.dn = nav->eph[psat - 1].deln;
        ephg.m0 = nav->eph[psat - 1].M0;
        ephg.cuc = nav->eph[psat - 1].cuc;
        ephg.e = nav->eph[psat - 1].e;
        ephg.cus = nav->eph[psat - 1].cus;
        ephg.roota = sqrt(nav->eph[psat - 1].A);
        ephg.cic = nav->eph[psat - 1].cic;
        ephg.omega0 = nav->eph[psat - 1].OMG0;
        ephg.cis = nav->eph[psat - 1].cis;
        ephg.i0 = nav->eph[psat - 1].i0;
        ephg.crc = nav->eph[psat - 1].crc;
        ephg.omega = nav->eph[psat - 1].omg;
        ephg.omegadot = nav->eph[psat - 1].OMGd;
        ephg.idot = nav->eph[psat - 1].idot;
        ephg.resvd0 = nav->eph[psat - 1].code;
        ephg.resvd1 = nav->eph[psat - 1].flag;
        ephg.accu = nav->eph[psat - 1].sva;
        ephg.hlth = nav->eph[psat - 1].svh;
        ephg.aodc = nav->eph[psat - 1].iodc;
        ephg.tgd = nav->eph[psat - 1].tgd[0];
        ephg.tgd1 = nav->eph[psat - 1].tgd[1];
        /*bdtime toe & week*/
        ephg.toe = time2gpst(nav->eph[psat - 1].toe, &week);
        ephg.week = week;
        //	memcpy(ephg.tgd,nav->eph[isat-1].tgd,sizeof(double)*4);
        ephg.aode = genAode(bamboo::SYS[isys], ephg.mjd, ephg.sod, ephg.toe, ephg.aode, &ephg);

        bamboo::Controller::s_getInstance()->m_getOacAdapter()->v_inputEph(&ephg, NULL);
    }
    else
    {
        bamboo::GLSEPH ephr;
        int week;
        psat = atoi(prn + 1);
        strcpy(ephr.cprn, prn);
        memcpy(ephr.pos, nav->geph[psat - 1].pos, sizeof(double) * 3);
        memcpy(ephr.vel, nav->geph[psat - 1].vel, sizeof(double) * 3);
        memcpy(ephr.acc, nav->geph[psat - 1].acc, sizeof(double) * 3);
        for (int i = 0; i < 3; i++)
        {
            ephr.pos[i] /= 1e3;
            ephr.vel[i] /= 1e3;
            ephr.acc[i] /= 1e3;
        }
        ephr.frenum = nav->geph[psat - 1].frq;
        ephr.gamma = nav->geph[psat - 1].gamn;
        ephr.tau = -nav->geph[psat - 1].taun;
        ephr.health = nav->geph[psat - 1].svh;
        ephr.age = nav->geph[psat - 1].age;
        KPL_IO::gtime_t tk = gpst2utc(nav->geph[psat - 1].tof); /*change it back into utc*/
        ephr.tk = KPL_IO::time2gpst(tk, &week);
        double sow = KPL_IO::time2gpst(nav->geph[psat - 1].toe, &week); /*GPST*/
        bamboo::wksow2mjd(week, sow, &ephr.mjd, &ephr.sod);
        ephr.aode = nav->geph[psat - 1].iode;
        ephr.aode = bamboo::genAode(bamboo::SYS[isys], ephr.mjd, ephr.sod, 0.0, ephr.aode, NULL);
        bamboo::Controller::s_getInstance()->m_getOacAdapter()->v_inputEph(NULL, &ephr);
    }
}
int s_setSsrData(KPL_IO::ssr_t *ssr)
{
    double sow;
    int isat, psat, ueph = false, uclk = false, ret = 0;
    char cprn[256] = {0};
    bamboo::SSRORB orb;
    bamboo::SSRCLK clk;
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    /*only support orbit & clock */
    for (isat = 0; isat < MAXRTKSAT; isat++)
    {
        if (ssr[isat].update)
        {
            ssr[isat].update = 0;
            KPL_IO::satno2id(isat + 1, cprn);
            if (-1 == (psat = bamboo::pointer_string(dly->nprn, dly->cprn, cprn)))
                continue;
            if (ssr[isat].iod[0] != -999)
            {
                ueph = true;
                ssr[isat].iod[0] = -999;
                /*fill the value*/
                orb.sow = time2gpst(ssr[isat].t0[0], &orb.wk);
                orb.iod[psat] = cprn[0] == 'C' ? ssr[isat].iodcrc : ssr[isat].iode;
                memcpy(orb.dx[psat], ssr[isat].deph, sizeof(double) * 3);
                memcpy(orb.dx[psat] + 3, ssr[isat].ddeph, sizeof(double) * 3);
            }
            if (ssr[isat].iod[1] != -999)
            {
                uclk = true;
                ssr[isat].iod[1] = -999;
                /*fill the value*/
                clk.sow = time2gpst(ssr[isat].t0[1], &clk.wk);
                clk.iod[psat] = cprn[0] == 'C' ? ssr[isat].iodcrc : ssr[isat].iode;
                memcpy(clk.c[psat], ssr[isat].dclk, sizeof(double) * 3);
            }
        }
    }
    bamboo::OrbitClkAdapter *orbc = bamboo::Controller::s_getInstance()->m_getOacAdapter();
    if (ueph && uclk)
    {
        orbc->v_inputSsr(&orb, &clk);
    }
    else if (ueph)
    {
        orbc->v_inputSsr(&orb, NULL);
    }
    else if (uclk)
    {
        orbc->v_inputSsr(NULL, &clk);
    }
    return ret;
}
static map<string, double> s_bias2upd(int isat, double *bias)
{
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    bamboo::Upd_ambconfig &ewl_config = dly->ewl_config;
    bamboo::Upd_ambconfig &wl_config = dly->wl_config;
    bamboo::Upd_ambconfig &nl_config = dly->nl_config;

    int isys = bamboo::index_string(bamboo::SYS, dly->prn_alias[isat][0]);

    int iq = nl_config.iq_comb[isys][0];
    int jq = nl_config.iq_comb[isys][1];

    double fq0 = bamboo::freqbytp(isat, dly->freq[isys][iq]);
    double fq1 = bamboo::freqbytp(isat, dly->freq[isys][jq]);
    for (int i = 0; i < bamboo::MAXFREQ; ++i)
    {
        if (bias[i] != bamboo::INVALID_UPD)
            bias[i] = bias[i] / 1e9 * bamboo::VEL_LIGHT;
    }
    map<string, double> c2upd;
    if (bias[iq] != bamboo::INVALID_UPD && bias[jq] != bamboo::INVALID_UPD && iq != -1 && jq != -1)
    {
        double nl_upd = -1 * (_IF_0(fq0, fq1) * bias[iq] + _IF_1(fq0, fq1) * bias[jq]); /* in ns */
        double wl_upd = -1 * (_WL_0(fq0, fq1) * bias[iq] + _WL_1(fq0, fq1) * bias[jq]); /* in ns */
        double lam_NL = bamboo::VEL_LIGHT / (fq0 + fq1), lam_WL = bamboo::VEL_LIGHT / (fq0 - fq1);
        nl_upd = nl_upd / lam_NL;
        wl_upd = wl_upd / lam_WL;
        c2upd["wl"] = wl_upd;
        c2upd["n1"] = nl_upd;
    }

    iq = ewl_config.iq_comb[isys][0];
    jq = ewl_config.iq_comb[isys][1];
    if (bias[iq] != bamboo::INVALID_UPD && bias[jq] != bamboo::INVALID_UPD && iq != -1 && jq != -1)
    {

        fq0 = bamboo::freqbytp(isat, dly->freq[isys][iq]);
        fq1 = bamboo::freqbytp(isat, dly->freq[isys][jq]);
        double ewl_upd = -1 * (_WL_0(fq0, fq1) * bias[iq] + _WL_1(fq0, fq1) * bias[jq]);
        double lam_ewl = bamboo::VEL_LIGHT / (fq0 - fq1);
        ewl_upd = ewl_upd / lam_ewl;
        c2upd["ewl"] = ewl_upd;
    }
    return c2upd;
}
static void s_setBiasData(KPL_IO::ssr_t *ssr)
{
    bool b_out = false;
    double sow, sod;
    int isat, psat, isys, iq, wk, mjd;
    char cprn[256] = {0}, obstype[256] = {0};
    bamboo::Controller *s_con = bamboo::Controller::s_getInstance();
    bamboo::Deploy *dly = s_con->m_getConfigure();
    bamboo::Upd_comb comb_ewl, comb_wl, comb_nl;
    /*only support orbit & clock */
    for (isat = 0; isat < MAXRTKSAT; isat++)
    {
        KPL_IO::satno2id(isat + 1, cprn);
        if (-1 == (psat = bamboo::pointer_string(dly->nprn, dly->cprn, cprn)))
            continue;
        isys = bamboo::index_string(bamboo::SYS, dly->prn_alias[psat][0]);
        if (ssr[isat].iod[5] != -999)
        {
            /* update the phase-bias here */
            ssr[isat].iod[5] = -999;
            sow = KPL_IO::time2gpst(ssr[isat].t0[5], &wk);
            bamboo::wksow2mjd(wk, sow, &mjd, &sod);
            char p_obsstr[1024] = {0};
            double bias[bamboo::MAXFREQ] = {0};
            for (int i = 0; i < bamboo::MAXFREQ; ++i)
                bias[i] = bamboo::INVALID_UPD;
            for (int i = 0; i < MAXCODE; ++i)
            {
                strcpy(obstype, KPL_IO::code2obs(i + 1));
                if (!strlen(obstype))
                    continue;
                if (-1 == (iq = bamboo::bbo_if(isys, (string("L") + obstype[0]).c_str())))
                    continue;
                double f1 = bamboo::freqbytp(psat, (string("L") + obstype[0]).c_str());
                if (ssr[isat].stdpb[i] != 0.0)
                {
                    bias[iq] = ssr[isat].pbias[i] / bamboo::VEL_LIGHT * 1e9; /* in nanoseconds */
                    sprintf(p_obsstr + strlen(p_obsstr), "%s %9.3lf ", (string("L") + obstype).c_str(), ssr[isat].pbias[i]);
                }
            }
            map<string, double> c2b = s_bias2upd(psat, bias);
            if (c2b.find("ewl") != c2b.end())
            {
                comb_ewl.mjd = mjd;
                comb_ewl.sod = sod;
                comb_ewl.ptime[psat] = mjd + sod / 86400.0;
                comb_ewl.comb_bias[psat] = c2b["ewl"];
            }
            if (c2b.find("wl") != c2b.end())
            {
                comb_wl.mjd = mjd;
                comb_wl.sod = sod;
                comb_wl.ptime[psat] = mjd + sod / 86400.0;
                comb_wl.comb_bias[psat] = c2b["wl"];
            }
            if (c2b.find("n1") != c2b.end())
            {
                comb_nl.mjd = mjd;
                comb_nl.sod = sod;
                comb_nl.ptime[psat] = mjd + sod / 86400.0;
                comb_nl.comb_bias[psat] = c2b["n1"];
            }

            char buff[1024];
            std::stringstream  ss;
//            sprintf(buff, "%04d %13.3lf %s pbias: %-100s", wk, sow, dly->cprn[psat].c_str(),p_obsstr);
            sprintf(buff, "%-50s", p_obsstr);
            ss << buff;
            LoggerSsr::pbais[dly->cprn[psat].c_str()] = ss.str();

            if (strlen(p_obsstr) && b_out)
                printf("[PHASEBIAS] %04d %13.3lf %s pbias: %-100s yangle: %9.3lf yrate:%9.4lf \n", wk, sow, dly->cprn[psat].c_str(),
                       p_obsstr, ssr[isat].yaw_ang > 180 ? ssr[isat].yaw_ang - 360 : ssr[isat].yaw_ang, ssr[isat].yaw_rate);
        }
        if (ssr[isat].iod[4] != -999)
        {
            ssr[isat].iod[4] = -999;
            sow = time2gpst(ssr[isat].t0[4], &wk);
            char p_obsstr[1024] = {0};
            for (int i = 0; i < MAXCODE; ++i)
            {
                strcpy(obstype, KPL_IO::code2obs(i + 1));
                if (!strlen(obstype))
                    continue;
                if (ssr[isat].cbias[i] != 0)
                {
                    s_con->m_getCodeBiasAdapter()->v_inputCodeBias(cprn, obstype, ssr[isat].cbias[i]);
                    sprintf(p_obsstr + strlen(p_obsstr), "%s %9.3lf ", (string("L") + obstype).c_str(), ssr[isat].cbias[i]);
                }
            }
            if (b_out)
                printf("[ CODEBIAS] %04d %13.3lf %s cbias: %-100s\n", wk, sow, dly->cprn[psat].c_str(), p_obsstr);
        }

        if (ssr[isat].iod[6] != -999)
        {
            int wk_t0, mjd;
            double sow_t0, sod;
            ssr[isat].iod[6] = -999;
            sow = time2gpst(ssr[isat].t0[6], &wk);
            sow_t0 = time2gpst(ssr[isat].ifpb_t0, &wk_t0);
            bamboo::wksow2mjd(wk, sow, &mjd, &sod);
            bamboo::t_ifpb ifpb;

            double lam3 = dly->SAT[psat].mlam[dly->freq[isys][2]];
            double lam1 = dly->SAT[psat].mlam[dly->freq[isys][0]];
            double g = pow(lam3 / lam1, 2);
            /// get correction for this moment
            ifpb.m_tc = ssr[isat].t0[6].time;
            ifpb.m_t0 = ssr[isat].ifpb_t0.time;
            ifpb.m_sum = ssr[isat].ifpb;
            ifpb.m_iset = true;
            if (b_out)
                printf("[     IFPB] %04d %13.3lf %s ifpb: %9.3lf t0:%13.3lf\n", mjd, sod, dly->cprn[psat].c_str(), ssr[isat].ifpb, sow_t0);
            s_con->m_getIfpbAdapter()->m_inputIfpb(dly->cprn[psat], ifpb);

        }
    }
    /* input into UPD bias */
    if (comb_ewl.mjd != 0)
        s_con->m_getUpdAdapter()->v_inputComb("comb", "ewl", comb_ewl);
    if (comb_wl.mjd != 0)
        s_con->m_getUpdAdapter()->v_inputComb("comb", "wl", comb_wl);
    if (comb_nl.mjd != 0)
        s_con->m_getUpdAdapter()->v_inputComb("comb", "n1", comb_nl);
}
void KPL_setintv(double intv)
{
    bamboo::Controller::s_getInstance()->m_setIntv(intv);
}
void KPL_initialize(const char *mode, KPL_IO::sta_t *sta, double cut, double intv)
{
    if (!KPL_isInitialize)
    {
        strcpy(bamboo::configPath,KPL_config_path);
        strcat(bamboo::configPath,"/");
        bamboo::Controller::s_getInstance()->_initilize(mode, sta, cut, intv);
        KPL_isInitialize = true;
    }
}
void KPL_finilize()
{
    if (KPL_isInitialize)
    {
        KPL_isInitialize = false;
        bamboo::Controller::s_getInstance()->_finalize();
    }
}
void KPL_inputObs(KPL_IO::gtime_t tm, KPL_IO::obs_t *obs)
{
    if (!KPL_isInitialize)
        return;
    bamboo::Rnxobs ob;
    s_setObsData(tm, obs, &ob);
    bamboo::Controller *s_con = bamboo::Controller::s_getInstance();
    bamboo::Deploy *dly = s_con->m_getConfigure();
    /* step 1, check observation */
    for (int isat = 0; isat < dly->nprn; ++isat)
        bamboo::Rnxobs::s_checkObs(isat, ob.obs[isat], ob.dop[isat], ob.snr[isat], ob.obsstat[isat]);
    /* step 2, update current time */
    dly->mjd = ob.mjd;
    dly->sod = ob.sod;
    /* step 3, correct with DCB */
    s_con->m_corrdcb(&ob);
    s_con->m_makeupOrbclkCorrs(ob.mjd, ob.sod);
    /* step 4, input into filter */
    s_con->v_inputObs(&ob);
}
void KPL_inputEph(KPL_IO::nav_t *nav, int psat, int offset)
{
    if (!KPL_isInitialize)
        return;
    s_setEphData(nav, psat, offset);
}
void KPL_inputSsr(KPL_IO::ssr_t *ssr)
{
    if (!KPL_isInitialize)
        return;
    s_setSsrData(ssr);
}
void KPL_inputSsrBias(KPL_IO::ssr_t *ssr)
{
    if (!KPL_isInitialize)
        return;
    s_setBiasData(ssr);
}
void KPL_restart()
{
    if (!KPL_isInitialize)
        return;
    bamboo::Controller *s_con = bamboo::Controller::s_getInstance();
}

void KPL_retrieve(const char *type, char *buff, int len){
    LoggerSsr *logSsr = LoggerSsr::s_getInstance();
    if (strcmp(type, "SSR-ORBCLK")==0){
        sprintf(buff, "%s", logSsr->getLastSsrInfo("orbclk").c_str());
    }else if (strcmp(type, "SSR-IFPB")==0){
        sprintf(buff, "%s", logSsr->getLastSsrInfo("ifpb").c_str());
    }else if (strcmp(type, "SSR-CODEBIAS")==0){
        sprintf(buff, "%s", logSsr->getLastSsrInfo("codebias").c_str());
    }else if (strcmp(type, "SSR-ALL")==0){
        sprintf(buff, "%s", logSsr->getLastSsrInfo("all").c_str());
    }
}