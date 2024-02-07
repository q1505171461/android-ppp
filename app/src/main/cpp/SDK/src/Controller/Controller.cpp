//
// Created by juntao on 2020/11/4.
//

#include <signal.h>
#include <algorithm>
#include <map>
#include "include/Utils/Com/CMat.h"
#include "include/Utils/Com/Com.h"
#include "include/Adapter/Upd/UpdAdapter.h"
#include "include/Adapter/Orbclk/BrdOrbitClkAdapter.h"
#include "include/Adapter/Orbclk/SsrOrbitClkAdapter.h"
#include "include/Adapter/CodeBias/CodeBiasAdapter.h"
#include "include/Controller/Controller.h"
using namespace bamboo;
Controller *Controller::sInstance = NULL;
Controller::Controller()
{
    mOacs = NULL;
    time(&t_startepoch);
}
Controller::~Controller()
{
    if (mOacs)
        delete mOacs;
    mOacs = NULL;
}
/* bugs here, since do not know when to save the ifpb into memory */
void Controller::m_corrdcb(Rnxobs *ob)
{
    /** code bias including: tgd/c1-p1bias/station dcb ***/
    mCodebias.m_corrCodeBias(ob->mjd, ob->sod, ob); /* codebias corrections */
    m_corrosb(ob);                                  /* osb correction */
}
void Controller::m_corrosb(Rnxobs *ob)
{
    /* correct osb here */
    if (dly.b_osbswitch)
    {
        Upd_bias bias;
        mUpdAdapter.m_inquireBias(dly.mjd, dly.sod, bias);
        for (int i = 0; i < dly.nprn; ++i)
        {
            int is = index_string(SYS, dly.prn_alias[i][0]);
            for (int iq = 0; iq < dly.nfreq[is]; ++iq)
            {
                if (ob->obs[i][iq] == 0.0)
                    continue;
                if (bias.v_checkSatellite(dly.cprn[i].c_str(), dly.freq[is][iq]))
                {
                    double lam = VEL_LIGHT / freqbytp(i, dly.freq[is][iq]);
                    ob->obs[i][iq] += bias.bias[i][iq] / lam;
                }
                else
                {
                    ob->obs[i][iq] = ob->obs[i][iq + MAXFREQ] = 0.0;
                    Rnxobs::s_checkObs(i, ob->obs[i], ob->dop[i], ob->snr[i], ob->obsstat[i]);
                }
            }
        }
    }
}
void Controller::m_setStartEpoch(time_t in)
{
    t_startepoch = in;
}
void Controller::m_makeupOrbclkCorrs(int mjd, double sod)
{
    /* make up the clock and the satpos */
    for (int isat = 0; isat < dly.nprn; ++isat)
    {
        /* clean clock */
        dly.SAT[isat].xclk = 0.0;
        /* clean satpos */
        memset(dly.SAT[isat].satpos, 0, sizeof(dly.SAT[isat].satpos));
        /* read sat orbit */
        if (!mOacs->v_readOrbit(dly.cprn[isat].c_str(), mjd, sod, dly.SAT[isat].satpos))
        {
            ; // LOGPRINT("[%d %8.1lf] cprn = %s, no orbit read!", mjd, sod, dly.cprn[isat].c_str());
        }
        /* read sat clock */
        if (!mOacs->v_readClk(dly.cprn[isat].c_str(), mjd, sod, &dly.SAT[isat].xclk))
        {
            ; // LOGPRINT("[%d %8.1lf] cprn = %s, no clock read!", mjd, sod, dly.cprn[isat].c_str());
        }
    }
}
void Controller::m_setIntv(double intv)
{
    dly.dintv = intv;
    dly.obsintv = intv;
}
Station Controller::m_initStation(KPL_IO::sta_t *t_n, double cut, char skd)
{
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    int curmjd, isys;
    double cursod, sigp = 0.003, sigr = 0.3;
    char key[256] = {0};
    /* generate from station */
    Station sta("STAN");
    /* fill body */
    strcpy(sta.clk, "CLK");
    sta.skd[0] = skd;
    sta.anttyp = t_n->antdes;
    sta.rectyp = t_n->rectype;
    sta.cutoff = cut * D2R;
    sta.dion0 = 3.0;
    sta.qion = 3.0;
    sta.dztd0 = 0.02;
    sta.qztd = 4e-4;
    sta.dx0[0] = 0.05;
    sta.dx0[1] = 0.05;
    sta.dx0[2] = 0.05;
    memcpy(sta.x, t_n->pos, sizeof(double) * 3);
    memcpy(sta.refx, t_n->pos, sizeof(double) * 3);
    memcpy(sta.enu0, t_n->del, sizeof(double) * 3);
    for (isys = 0; isys < MAXSYS; isys++)
    {
        sta.sigr[isys] = sigr;
        sta.sigp[isys] = sigp;
    }
    /* inner calculation*/
    if (sta.x[0] * sta.x[1] * sta.x[2] != 0.0)
    {
        xyzblh(sta.x, 1.0, 0, 0, 0, 0, 0, sta.geod);
        rot_enu2xyz(sta.geod[0], sta.geod[1], sta.rot_l2f);
    }
    return sta;
}
Controller *Controller::s_getInstance()
{
    if (sInstance == NULL)
        sInstance = new Controller;
    return sInstance;
}
void Controller::_initilize(const char *mode, KPL_IO::sta_t *sta, double cutoff, double dintv)
{
    int j;
    char skd = 'K';
    char ppp_json_path[1024] = "";
    strcat(ppp_json_path, bamboo::configPath);
    strcat(ppp_json_path, "ppp.json");
    const char *args[] = {"-mode", "ppp", ppp_json_path, "-itrs", "-dia", "-iar"};
    strcpy(dly.configPath, bamboo::configPath);
    dly.m_readJsonFile(sizeof(args) / sizeof(char *), (const char **)args);
    dly.dintv = dly.obsintv = dintv;
    /* initialize Station, including PCO */
    if (strstr(mode, "static"))
        skd = 'S';
    else if (strstr(mode, "kinematic"))
        skd = 'K';
    else if (strstr(mode, "dynamic"))
        skd = 'D';
    mOacs = new SsrOrbitClkAdapter();
}
void Controller::_finalize()
{
    if (sInstance != NULL)
        delete sInstance;
    sInstance = NULL;
}
int Controller::v_inputObs(Rnxobs *ob)
{
    /* Display rnxobs data */
    return 0;
}