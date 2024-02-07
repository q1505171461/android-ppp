/*
 * Deploy.cpp
 *
 *  Created on: 2018/8/27
 *      Author: juntao, at wuhan university
 */
#include <time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <algorithm>
#include "include/Utils/Com/Com.h"
#include "include/Utils/Com/Logtrace.h"
#include "include/Utils/Com/Patterns.h"
#include "include/Utils//Json/json.h"
#include "include/Controller/Controller.h"
#include "include/Controller/Deploy.h"
#include <android/log.h>
#define LOG_TAG "YourAppTag"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
using namespace std;
using namespace bamboo;
extern char  bamboo::configPath[1024] = "";
Deploy::Deploy()
{
    lobssmooth = lobssav = lsavorbclk = lfixslip = lsync = ldebug = lbdtime = ltrace = lhelp = ldia = liar = lpost = litrs = lsav =
        lsnr = false;
    mjd = mjd0 = mjd1 = 0;
    seslen = sod = sod0 = sod1 = 0.0;
    dintv = 30;
    lw = 3.0;
    lg = 2.0;
    dtec = 0.72;
    orbclk_intv = 30;
    scal_dx = scal_qx = 1000.0;
    nprn = nsys = 0;
    maxthd = 1;
    gap = 1200;
    promode = PRO_PPP;
    iref = -1;
    maxr_bias = 50.0;
    tracelevel = 0;
    b_osbswitch = false;
    pf_rtable = "";
    memset(m_addargs, 0, sizeof(m_addargs));
    memset(freq_if, 0, sizeof(freq_if));
    strcpy(yawmodel, "MODEL");
    memset(clktype, 0, sizeof(clktype));
    memset(orbtype, 0, sizeof(orbtype));
    memset(nfreq, 0, sizeof(nfreq));
    memset(nfreq_obs, 0, sizeof(nfreq_obs));
    memset(tracefile, 0, sizeof(tracefile));
    memset(statfile, 0, sizeof(statfile));
    strcpy(ifpbmode, "IFPB_NONE");
}
Deploy::~Deploy() {}
void Deploy::m_printUsage()
{
    char buff[1024] = {0};
    cout << "GNSS real-time & post data process program@jtaowhu" << endl;
    cout << "     usage : bamboo -mode (pppv|ppp|clk|ifpb|extwl|extnl|hmw|rtk|nrtk|vrs) *.json" << endl;
    cout << "compliled on " << get_compile_date(buff) << endl;
    cout << "     option: -post | -itrs | -dia | -iar | -sql | -repslip | -trace | -pthread | -pams " << endl;
    cout << "             -orbclksav intv | -sav | -url nrtk_url | -vinput vrs input | -rinput ratio-input" << endl;
    cout << "  attention: args follow -pams will be packed up as a batch of args for input, flags should not follow -pams" << endl;
    cout << "    options: -pams -outN1(debug)/-oution/-outztd/-outres/-outmissar(debug)/-usecmr23|-outAtom|-usemwwl(csys)" << endl;
    cout << "    options: -pams -outflag/-outobsstat/-outambstat" << endl;
    cout << "    options: -pams -gensig(generate the parameters sigma),-skipv(skip using virtual stations for service)" << endl;
    cout << "    options: -usemwwlG/-usemwwlGC/-usemwwlGCB, will use mw wl for G/GC/GCB" << endl;
    cout << "    options: -rtifpb(generate ifpb in real-time)/-reback(bring N1 con back for PCE)" << endl;
    cout << "Contact: jtaowhu@whu.edu.cn" << endl;
    exit(1);
}

void Deploy::m_parseSatelliteConfigures()
{
    char fmt[256] = {0};
    Satellite itemSat;
    string line;
    ifstream f;
    double cursod;
    int isys, jsys, i, j, k, curmjd;
    auto itr = this->mTable.find(string("satconf"));
    if (itr == mTable.end())
    {
        LOGPRINT_EX("key = %s,can't find the value for the key!", "satconf");
        exit(1);
    }
    char fname[1024]={0};
    strcat(fname, configPath);
    strcat(fname, (*itr).second.c_str());
    f.open(fname, ios::in);
    if (!f.is_open())
    {
        LOGPRINT_EX("file = %s,can't open file to read", (*itr).second.c_str());
        exit(1);
    }
    nprn = nsys = 0;
    memset(system, 0, sizeof(system));
    while (getline(f, line))
    {
        if (strstr(line.c_str(), "-Satellite used"))
            break;
        if (line[0] != ' ')
            continue;
        sscanf(line.c_str(), "%s%s", itemSat.cprn, itemSat.type);
        if (-1 !=
            pointer_string(nprn, this->cprn, string(itemSat.cprn)))
        {
            LOGPRINT_EX("cprn = %s,exist the same satellite", itemSat.cprn);
            continue;
        }
        cprn[nprn] = string(itemSat.cprn);
        char ct = csys_bytype(itemSat.type);
        if (-1 == (index_string(this->system, ct)))
        {
            this->system[this->nsys] = ct;
            this->nsys = this->nsys + 1;
        }
        if (-1 == (isys = index_string(SYS, ct)))
        {
            LOGPRINT_EX("system = %s,do not support system", itemSat.type);
            exit(1);
        }
        prn_alias[nprn] = getcprn_bytype(itemSat.cprn, ct);
        strcpy(itemSat.prn_alias, prn_alias[nprn++].c_str());
        // get_freq(&itemSat, cobs, this->nfreq_obs[isys], this->freq_obs[isys]);
        if (this->lpost)
        {
            // if (0 == Antatx::s_rdSvnav(
            //              itemSat.cprn, this->mjd0 + this->sod0 / 86400.0,
            //              itemSat.type, &itemSat.mass, &itemSat.ifreq,
            //              itemSat.offs, itemSat.sid, itemSat.svn))
            //     exit(1);
            // if (0 == itemSat.AX.m_getAnt(
            //              itemSat.cprn, this->nfreq_obs, this->freq_obs,
            //              this->mjd0 + this->sod0 / 86400.0,
            //              this->mjd1 + this->sod1 / 86400.0, itemSat.type,
            //              itemSat.cprn, itemSat.xyz, NULL))
            // {
            //     LOGPRINT_EX(
            //         "ant = %s,cprn = %s,satellite antenna is not found in "
            //         "atx file",
            //         itemSat.type, itemSat.cprn);
            //     exit(1);
            // }
        }
        else
        {
            cur_time(&curmjd, &cursod);
            // if (0 == Antatx::s_rdSvnav(
            //              itemSat.cprn, curmjd + cursod / 86400.0,
            //              itemSat.type, &itemSat.mass, &itemSat.ifreq,
            //              itemSat.offs, itemSat.sid, itemSat.svn))
            //     exit(1);
            // if (0 == itemSat.AX.m_getAnt(itemSat.cprn, this->nfreq_obs, this->freq_obs, curmjd + cursod / 86400.0, curmjd + cursod / 86400.0, itemSat.type,
            //                              itemSat.cprn, itemSat.xyz, NULL))
            // {
            //     LOGPRINT_EX(
            //         "ant = %s,cprn = %s,satellite antenna is not found in "
            //         "atx file",
            //         itemSat.type, itemSat.cprn);
            //     exit(1);
            // }
        }
        get_freq(&itemSat, cobs, this->nfreq_obs[isys], this->freq_obs[isys]);
        this->SAT.push_back(itemSat);
    }
    if (0 == (nprn = SAT.size()))
    {
        LOGPRINT_EX("nrpn = %d,no selected satellite!", nprn);
        exit(1);
    }
    /////////////////////////////////////////////////////////////////////

    if (-1 != (i = index_string(system, 'G')) || -1 != (i = index_string(system, 'C')) ||
        -1 != (i = index_string(system, 'E')) || -1 != (i = index_string(system, 'R')))
        ;
    if (i > 0)
        swap(system[i], system[0]);
    this->iref = index_string(SYS, this->system[0]);
    f.close();
}
void Deploy::sortf(int nfreq_in[MAXSYS], char freq_in[MAXSYS][MAXFREQ][LEN_FREQ])
{
    char key[128] = {0};
    int iq, jq, isys;
    double fqv[MAXFREQ] = {0}, tmp;
    for (isys = 0; isys < MAXSYS; ++isys)
    {
        for (iq = 0; iq < nfreq_in[isys]; ++iq)
        {
            fqv[iq] = SYS[isys] != 'R' ? freqbysys(isys, -1, freq_in[isys][iq]) : (strstr(freq_in[isys][iq], "L1") ? GLS_L1 : GLS_L2);
        }
        for (iq = 0; iq < nfreq_in[isys]; ++iq)
        {
            for (jq = iq + 1; jq < nfreq_in[isys]; ++jq)
            {
                if (fqv[iq] < fqv[jq])
                {
                    tmp = fqv[jq];
                    fqv[jq] = fqv[iq];
                    fqv[iq] = tmp;

                    strcpy(key, freq_in[isys][jq]);
                    strcpy(freq_in[isys][jq], freq_in[isys][iq]);
                    strcpy(freq_in[isys][iq], key);
                }
            }
        }
    }
}

void Deploy::m_sortOrderofFreq()
{
    int isys, i, j, iq;
    /* update freq_obs and freq_clk and frequency to form ionosphere combination */
    for (isys = 0; isys < MAXSYS; ++isys)
    {
        nfreq_if[isys] = 0;
        for (iq = 0; iq < 2 && nfreq[isys] >= 2; ++iq)
        {
            strcpy(freq_if[isys][iq], freq[isys][iq]); // saving the first frequency before sort order
            ++nfreq_if[isys];
        }
        if (nfreq[isys] > 2) // use freq_clk ? what is the point?
        {
            l5dcb_freq[SYS[isys]] = vector<string>();
            for (iq = 2; iq < nfreq[isys]; ++iq)
            {
                l5dcb_freq[SYS[isys]].push_back(freq[isys][iq]); // l5 dcb identifier
            }
            // for (iq = 0; iq < nfreq[isys]; ++iq)
            // {
            //     if (pointer_charstr(nfreqcomb_clk[isys], LEN_FREQ, (const char *)freqcomb_clk[isys], freq[isys][iq]) == -1)
            //         l5dcb_freq[SYS[isys]].push_back(freq[isys][iq]); // l5 dcb identifier
            // }
        }
    }
    sortf(nfreq_if, freq_if); // maybe bugf for single-frequency if only set one frequency in the configures
    sortf(nfreq, freq);
    sortf(nfreqcomb_clk, freqcomb_clk);
    /**frequency_obs is the same order with the first nfreq[isys]**/
    for (isys = 0; isys < MAXSYS; isys++)
    {
        for (i = 0; i < this->nfreq[isys]; i++)
        {
            if (-1 == (j = pointer_charstr(MAXFREQ, LEN_FREQ, (char *)freq_obs[isys], freq[isys][i])))
            {
                LOGPRINT_EX("freq = %s,this frequency is not contained in the freq_obs", freq[isys][i]);
                exit(1);
            }
            string s = freq_obs[isys][i];
            memcpy(freq_obs[isys][i], freq_obs[isys][j], sizeof(char) * LEN_FREQ);
            memcpy(freq_obs[isys][j], s.c_str(), sizeof(char) * s.size());
        }
    }
}
void Deploy::m_readJsonFile(int argc, const char *args[])
{
    int isys, curmjd, i, isit, j;
    map<string, string>::iterator itr;
    int iargc, iy, im, id, ih, imi;
    double cursod, dsec;
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    Json::Value root;
    JSONCPP_STRING errs;
    const char *curItem = "";
    vector<string> strtmp;
    char value[1024] = {0}, cex, key[256], fmt[512], buf[128] = {0}, mode[256], checkv[256], hmwf[256] = {0};
    string prn, second, line, _jsonFile;
    fstream f;
    this->SAT.clear();
    this->mTable.clear();
    Satellite itemSat;
    //////////////// parse command line ///////////////
    for (iargc = 0; iargc < argc; iargc++)
    {
        if (strstr(args[iargc], "-mode"))
        {
            strcpy(mode, args[++iargc]);
            _jsonFile = string(args[++iargc]);
            strcpy(this->jsonConfig, _jsonFile.c_str());
            if (!strcmp(mode, "hmw") || !strcmp(mode, "extwl"))
            {
                strcpy(hmwf, args[++iargc]);
            }
        }
        if (strstr(args[iargc], "-post"))
            lpost = true;
        if (strstr(args[iargc], "-itrs"))
            litrs = true;
        if (strstr(args[iargc], "-iar"))
            liar = true;
        if (strstr(args[iargc], "-dia"))
            ldia = true;
        if (!strncmp(args[iargc], "-help", 5))
            lhelp = true;
        if (strstr(args[iargc], "-trace"))
            ltrace = true;

        if (strstr(args[iargc], "-snr"))
            lsnr = true;
        if (strstr(args[iargc], "-debug"))
            ldebug = true;

        if (strstr(args[iargc], "-rinput"))
        {
            this->pf_rtable = args[++iargc];
        }

        if (strstr(args[iargc], "-pams"))
        {
            for (int ic = iargc + 1; ic < argc; ++ic)
                sprintf(m_addargs + strlen(m_addargs), "%s ", args[ic]);
            iargc = argc;
        }
    }
    if (lhelp)
    {
        m_printUsage();
        exit(1);
    }
    // parse the configure here
    f.open(_jsonFile.c_str(), ios::in);
    if (!f.is_open())
    {
        LOGPRINT_EX("file = %s,can't open file to read!\n", _jsonFile.c_str());
        exit(1);
    }
    bool parse_ok = Json::parseFromStream(rbuilder, f, &root, &errs);
    f.close();
    if (!parse_ok)
    {
        LOGPRINT_EX("file = %s,can't parse file!\n", _jsonFile.c_str());
        exit(1);
    }
    try
    {
        /* acquire mode */

        this->promode = PRO_PPP;

        /////////////////////////////// tables directory part //////////////////
        curItem = "path";
        bamboo::excludeAnnoValue(ephdir, root[curItem]["broad"].asCString());
        bamboo::excludeAnnoValue(tracedir, root[curItem]["trace"].asCString());
        bamboo::excludeAnnoValue(obsdir, root[curItem]["obs"].asCString());
        bamboo::excludeAnnoValue(externaldir, root[curItem]["external"].asCString());
        bamboo::excludeAnnoValue(qcdir, root[curItem]["qclog"].asCString());
        bamboo::excludeAnnoValue(outdir, root[curItem]["output"].asCString());

        if (!strlen(tracefile))
            sprintf(tracefile, "%s%cbamboo.trace", tracedir, FILEPATHSEP);
        sprintf(statfile, "%s%cbamboo.stat", tracedir, FILEPATHSEP);
        if (access(tracedir, 0) != 0)
        {
            LOGPRINT_EX("tracedir = %s,trace dir is not exist!", tracedir);
            exit(1);
        }
        if (this->ltrace)
            Logtrace::s_defaultlogger.m_openLog(tracefile, true, 7200);
        Logtrace::s_defaultlogger.m_openLog(statfile, true, 7200);
        //////////////////////////////////////////////////////////////////////////////
        curItem = "files";
        bamboo::excludeAnnoValue(tablesConfig, root["files"].asCString());
        m_parseTablesConfigures(tablesConfig);
        { /* parse common configure */
            curItem = "common";
            m_readJsonItem(root, "common");
        }
        /////////////////////////// datum part ////////////////////////
        curItem = "dcbdatum";
        m_readJsonItem(root, "dcbdatum");
        ///////////////////////////// Threshold Part ///////////////////////////
        curItem = "threshold";
        bamboo::excludeAnnoValue(value, root[curItem]["turboedit"].asCString());
        sscanf(value, "%lf%lf%lf%lf", &lg, &lw, &gap, &dtec);
        try
        {
            bamboo::excludeAnnoValue(value, root[curItem]["rangelimit"].asCString());
            sscanf(value, "%lf", &maxr_bias);
            LOGPRINT_EX("rangelimit set as %lf", maxr_bias);
        }
        catch (...)
        {
            LOGPRINT_EX("ranglimit not exist, use prefined value : %lf", maxr_bias);
        }
        bamboo::excludeAnnoValue(value, root[curItem]["dia"].asCString());
        sscanf(value, "%s%s", diamode, checkv);
        if (strstr(checkv, "RESET"))
            ldia_reset = true;
        else
            ldia_reset = false;
        bamboo::excludeAnnoValue(value,
                                 root[curItem]["dia_search-limit"].asCString());
        sscanf(value, "%lf%lf", &diasearch_esig, &diasearch_esigdone);
        bamboo::excludeAnnoValue(value,
                                 root[curItem]["dia_batch-limit"].asCString());
        sscanf(value, "%lf%lf", &diabatch_wlimit, &diabatch_esig);
        /* sort the order of frequency */
        m_sortOrderofFreq();
        /* satellite part */
        curItem = "SAT";
        m_parseSatelliteConfigures();
        /* ambiguity configures, must behind sort freq, since involves index of frequency */
        curItem = "amb";
        m_readJsonItem(root, "amb");
        //////////////////////////////////////////////////////////////////////////////
        m_checkConfigure();
    }
    catch (...)
    {
        LOGPRINT_EX("item = %s,failed to parase item", curItem);
        LOGE("item = %s,failed to parase item", curItem);
        exit(1);
    }
}
void Deploy::m_readJsonItem(Json::Value &root, const char *item)
{
    int iy, im, id, ih, imin, nsys, i, j, isys, fd;
    double dsec;
    char value[256] = {0}, key[1024] = {0};
    try
    {
        // read common configure
        if (strstr(item, "common"))
        {
            bamboo::excludeAnnoValue(value, root[item]["time"].asCString());
            sscanf(value, "%d%d%d%d%d%lf%lf", &iy, &im, &id, &ih, &imin, &dsec,
                   &seslen);
            yr2year(iy);
            this->mjd0 = md_julday(iy, im, id);
            this->sod0 = ih * 3600 + imin * 60 + dsec;
            this->mjd1 = (int)(this->mjd0 + (this->sod0 + seslen) / 86400);
            this->sod1 =
                this->sod0 + seslen - (this->mjd1 - this->mjd0) * 86400.0;
            bamboo::excludeAnnoValue(value, root[item]["interval"].asCString());
            sscanf(value, "%lf", &dintv);
            obsintv = dintv;
            if (strstr(value, ":"))
                sscanf(value + index_string(value, ':') + 1, "%lf", &obsintv);
            bamboo::excludeAnnoValue(ztdpri, root[item]["ztdpri"].asCString());
            bamboo::excludeAnnoValue(ztdmod,
                                     root[item]["ztdmodel"].asCString());
            bamboo::excludeAnnoValue(uobs, root[item]["obsused"].asCString());
            bamboo::excludeAnnoValue(value,
                                     root[item]["noisecoef"].asCString());
            sscanf(value, "%lf%lf", &this->scal_dx, &this->scal_qx);
            bamboo::excludeAnnoValue(refpoint,
                                     root[item]["refpoint"].asCString());
            bamboo::excludeAnnoValue(yawmodel,
                                     root[item]["yawmodel"].asCString());
            bamboo::excludeAnnoValue(value, root[item]["freqobs"].asCString());
            m_parseFrequency(freq_obs, nfreq_obs, value);
            /* get the frequency used here */
            bamboo::excludeAnnoValue(value, root[item]["freqused"].asCString());
            m_parseFrequency(freq, nfreq, value);
            // generate the different noise coef for different constellation
            try
            {
                /// GPS:1_1_1 GLS:1_1 GAL:1_1_1 BD2:1_1_1 BD3:1_1_1 QZS:1_1_1 # the same order as freqused
                bamboo::excludeAnnoValue(value, root[item]["noisebyfreq"].asCString());
                m_parseObsNoiseScal(value, scal_obs);
            }
            catch (...)
            {

                /// generate the using the default value
                for (int isys = 0; isys < MAXSYS; ++isys)
                {
                    for (int ifreq = 0; ifreq < nfreq[isys]; ++ifreq)
                    {
                        scal_obs[toString(SYS[isys]) + freq[isys][ifreq]] = 1.0;
                    }
                }
            }

            ////////////////////////////////////////////////////////
            bamboo::excludeAnnoValue(value, root[item]["threads"].asCString());
            sscanf(value, "%d", &maxthd);
            bamboo::excludeAnnoValue(cobs, root[item]["obscomb"].asCString());
            {
                /// DCB model
                bamboo::excludeAnnoValue(value, root[item]["dcbmodel"]["p1p2_dcb"].asCString());
                sscanf(value, "%s%lf%lf", sta_dcbmod, &dcb_std0, &dcb_qstd0);
                bamboo::excludeAnnoValue(value, root[item]["dcbmodel"]["icb_dcb"].asCString());
                sscanf(value, "%s%lf%lf", sta_icbmod, &icb_std0, &icb_qstd0);
                bamboo::excludeAnnoValue(value, root[item]["dcbmodel"]["isb_dcb"].asCString());
                sscanf(value, "%s%lf%lf", sta_isbmod, &isb_std0, &isb_qstd0);
            }
            bamboo::excludeAnnoValue(this->ionmod, root[item]["ionmodel"].asCString());
            bamboo::excludeAnnoValue(this->ionpri, root[item]["ionpri"].asCString());
            bamboo::excludeAnnoValue(this->ambmod, root[item]["ambmode"].asCString());
            for (i = 0; i < root[item]["orbtype"].size(); i++)
            {
                bamboo::excludeAnnoValue(orbtype[i],
                                         root[item]["orbtype"][i].asCString());
            }
            for (i = 0; i < root[item]["clktype"].size(); i++)
            {
                bamboo::excludeAnnoValue(clktype[i],
                                         root[item]["clktype"][i].asCString());
            }
            bamboo::excludeAnnoValue(value,
                                     root[item]["tracelevel"].asCString());
            sscanf(value, "%d", &tracelevel);
        }
        else if (strstr(item, "dcbdatum"))
        {
            bamboo::excludeAnnoValue(c1p1cortype,
                                     root[item]["dcbtype"].asCString());
            bamboo::excludeAnnoValue(value,
                                     root[item]["clockdatum"].asCString());
            m_parseFrequency(freqcomb_clk, nfreqcomb_clk, value);
            for (isys = 0; isys < MAXSYS; isys++)
            {
                if ((nfreqcomb_clk[isys] > 0 && nfreqcomb_clk[isys] > 2))
                {
                    LOGPRINT_EX("the number of frequency of clockdatum or dcbestdatum for %c must <= 2", SYS[isys]);
                    exit(1);
                }
            }
            bamboo::excludeAnnoValue(ifpbmode, root[item]["ifpb_cor"].asCString());
        }

        else if (strstr(item, "amb"))
        {
            bamboo::excludeAnnoValue(value, root[item].asCString());
            m_parseAmbiguityConfigures(value);
        }
    }
    catch (...)
    {
        LOGPRINT_EX("item = %s,parse item error!", item);
        LOGE("item = %s,parse item error!", item);
        exit(1);
    }
}
void Deploy::m_parseObsNoiseScal(char *freq_seq, map<string, double> &scal)
{
    int i, nsys, isys, j, n_noise;
    char freq[MAXSYS][LEN_STRING] = {0}, c_value[MAXFREQ][LEN_FREQ] = {0};

    split_string(true, freq_seq, ' ', ' ', ' ', &nsys, (char *)freq, LEN_STRING);
    for (i = 0; i < nsys; ++i)
    {
        isys = -1;
        j = index_string(freq[i], ':');
        if (strstr(freq[i], "GPS") != NULL)
            isys = index_string(SYS, 'G');
        else if (strstr(freq[i], "GLS") != NULL)
            isys = index_string(SYS, 'R');
        else if (strstr(freq[i], "GAL") != NULL)
            isys = index_string(SYS, 'E');
        else if (strstr(freq[i], "BD2") != NULL)
            isys = index_string(SYS, 'B');
        else if (strstr(freq[i], "BDS") != NULL ||
                 strstr(freq[i], "BD3") != NULL)
            isys = index_string(SYS, 'C');
        else if (strstr(freq[i], "QZS") != NULL)
            isys = index_string(SYS, 'J');
        else
        {
            LOGPRINT_EX("freq = %s: unknown GNSS system", freq[i]);
            LOGE("freq = %s: unknown GNSS system", freq[i]);
            exit(1);
        }
        if (isys != -1)
        {
            split_string(true, freq[i] + j + 1, ' ', ' ', '_', &n_noise, (char *)c_value, LEN_FREQ);
            for (int ifreq = 0; ifreq < n_noise; ++ifreq)
            {
                scal[toString(SYS[isys]) + this->freq[isys][ifreq]] = atof(c_value[ifreq]);
            }
        }
    }
}
void Deploy::m_parseFrequency(char freq_in[MAXSYS][MAXFREQ][LEN_FREQ], int *nfreq_in, char *freq_seq)
{
    /// split string to fill the frequency here
    int nsys, i, isys, j;
    char freq[MAXSYS][LEN_STRING] = {0};
    split_string(true, freq_seq, ' ', ' ', ' ', &nsys, (char *)freq, LEN_STRING);
    for (i = 0; i < nsys; i++)
    {
        isys = -1;
        j = index_string(freq[i], ':');
        if (strstr(freq[i], "GPS") != NULL)
            isys = index_string(SYS, 'G');
        else if (strstr(freq[i], "GLS") != NULL)
            isys = index_string(SYS, 'R');
        else if (strstr(freq[i], "GAL") != NULL)
            isys = index_string(SYS, 'E');
        else if (strstr(freq[i], "BDS") != NULL ||
                 strstr(freq[i], "CPS") != NULL ||
                 strstr(freq[i], "CMS") != NULL)
            isys = index_string(SYS, 'C');
        else if (strstr(freq[i], "BD2"))
            isys = index_string(SYS, 'B');
        else if (strstr(freq[i], "BD3"))
            isys = index_string(SYS, 'C');
        else if (strstr(freq[i], "QZS") != NULL)
            isys = index_string(SYS, 'J');
        else
        {
            LOGPRINT_EX("freq = %s: unknown GNSS system", freq[i]);
            LOGE("freq = %s: unknown GNSS system", freq[i]);
            exit(1);
        }
        if (isys != -1)
        {
            split_string(true, freq[i] + j + 1, ' ', ' ', '_', nfreq_in + isys,
                         (char *)(freq_in[isys]), LEN_FREQ);
            for (j = 0; j < nfreq_in[isys]; j++)
            {
                if (strstr(freq_in[isys][j], "L1") != NULL ||
                    strstr(freq_in[isys][j], "G1") != NULL ||
                    strstr(freq_in[isys][j], "E1") != NULL ||
                    !strcmp(freq_in[isys][j], "B1c"))
                    strcpy(freq_in[isys][j], "L1");
                else if (strstr(freq_in[isys][j], "L2") != NULL ||
                         strstr(freq_in[isys][j], "G2") != NULL ||
                         strstr(freq_in[isys][j], "E2") != NULL ||
                         !strcmp(freq_in[isys][j], "B1"))
                    strcpy(freq_in[isys][j], "L2");
                else if (strstr(freq_in[isys][j], "L5") != NULL ||
                         strstr(freq_in[isys][j], "E5a") != NULL ||
                         !strcmp(freq_in[isys][j], "B2a"))
                    strcpy(freq_in[isys][j], "L5");
                else if (strstr(freq_in[isys][j], "E6") != NULL ||
                         !strcmp(freq_in[isys][j], "B3") ||
                         strstr(freq_in[isys][j], "LEX") != NULL)
                    strcpy(freq_in[isys][j], "L6");
                else if (strstr(freq_in[isys][j], "E5b") != NULL ||
                         !strcmp(freq_in[isys][j], "B2b"))
                    strcpy(freq_in[isys][j], "L7");
                else if (strstr(freq_in[isys][j], "E5") != NULL ||
                         !strcmp(freq_in[isys][j], "B2"))
                    strcpy(freq_in[isys][j], "L8");
            }
        }
    }
}
void Deploy::m_parseTablesConfigures(const char *t_path)
{
    fstream f;
    char value[1024] = {0};
    string curItem = "";
    JSONCPP_STRING errs;
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    f.open(t_path, ios::in);
    if (!f.is_open())
    {
        printf("file = %s,can't open file to read!\n", t_path);
        exit(1);
    }
    bool parse_ok = Json::parseFromStream(rbuilder, f, &root, &errs);
    f.close();
    if (!parse_ok)
    {
        printf("file = %s,can't parse file!\n", t_path);
        exit(1);
    }
    try
    {
        curItem = "files";
        bamboo::excludeAnnoValue(value, root[curItem]["satconf"][0].asCString());
        this->mTable["satconf"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["satnav"][0].asCString());
        this->mTable["satnav"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["antnam"][0].asCString());
        this->mTable["antnam"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["antpcv"][0].asCString());
        this->mTable["antpcv"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["leapsc"][0].asCString());
        this->mTable["leapsc"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["ocload"][0].asCString());
        this->mTable["ocload"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["ut1t10"][0].asCString());
        this->mTable["ut1t10"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["predefine"][0].asCString());
        this->mTable["predefine"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["polut1"][0].asCString());
        this->mTable["polut1"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["gptgrd_w"][0].asCString());
        this->mTable["gptgrd_w"] = string(value);
        bamboo::excludeAnnoValue(value, root[curItem]["rinput"][0].asCString());
        this->mTable["ratio"] = string(value);
#ifdef USEDE405
        bamboo::excludeAnnoValue(value, root[curItem]["jpleph"][0].asCString());
        this->mTable["jpleph"] = string(value);
#endif
    }
    catch (...)
    {
        printf("item = %s,file = %s,failed to extract config", curItem.c_str(),
               t_path);
        exit(1);
    }
}
bool Deploy::m_checkConfigure()
{
    char strbuf[1024];
    /// check configure and output into the trace file
    if (!strstr(this->cobs, "RAW") && !strstr(this->cobs, "IF") &&
        !strstr(this->cobs, "SF"))
    {
        LOGPRINT_EX("cobs = %s,key = obscomb,only support RAW,IF and SF", cobs);
        exit(1);
    }
    if (!strstr(this->uobs, "PHASE") && !strstr(this->uobs, "CODE") &&
        !strstr(this->uobs, "GRAPHIC"))
    {
        LOGPRINT_EX("uobs = %s,key = obsused,only support the combination of PHASE,GRAPHIC and CODE",
                    uobs);
        exit(1);
    }
    if (!strstr(this->refpoint, "APC_ITRF") &&
        !strstr(this->refpoint, "CoM_ITRF"))
    {
        LOGPRINT_EX(
            "refpoint = %s,key = refpoint,only support APC_ITRF and CoM_ITRF",
            refpoint);
        exit(1);
    }
    if (!strstr(this->ztdmod, "ZTD"))
    {
        LOGPRINT_EX(
            "ztdmod = %s,key = ztdmod,only support ZTD:MINUTES,ZTDSTO and "
            "ZTDNONE",
            ztdmod);
        exit(1);
    }
    if (!strstr(this->ztdpri, "ZTD_VMF1") &&
        !strstr(this->ztdpri, "ZTD_NONE") &&
        !strstr(this->ztdpri, "ZTD_INTERP"))
    {
        LOGPRINT_EX("ztdpri = %s,key = ztdmod,only support ZTD_VMF1,ZTD_INTERP, and ZTD_NONE",
                    ztdpri);
        exit(1);
    }
    if (!strstr(this->ionmod, "ION_NONE") && !strstr(this->ionmod, "ION_EST"))
    {
        LOGPRINT_EX("ionmod = %s,key = ionmodel,only support ION_EST and ION_NONE",
                    ionmod);
        exit(1);
    }
    if (!strstr(this->ionpri, "ION_POSTBRD") &&
        !strstr(this->ionpri, "ION_POSTPRE") &&
        !strstr(this->ionpri, "ION_RTBRD") &&
        !strstr(this->ionpri, "ION_RTPRE") &&
        !strstr(this->ionpri, "ION_NONE") &&
        !strstr(this->ionpri, "ION_DDPRE") &&
        !strstr(this->ionpri, "ION_INTERP"))
    {
        LOGPRINT_EX(
            "ionpri = %s,key = ionpri,only support "
            "ION_POSTBRD,ION_POSTPRE,ION_RTBRD,ION_RTPRE,ION_DDPRE and ION_NONE",
            ionpri);
        exit(1);
    }
    if (!strstr(this->ambmod, "CONT") && !strstr(this->ambmod, "INST") &&
        !strstr(this->ambmod, "HOLD"))
    {
        LOGPRINT_EX("ambmod = %s,key = ambmode,only support CONT,INST and HOLD",
                    ambmod);
        exit(1);
    }

    for (int i = 0; i < MAXORBEPHTYPE; i++)
    {
        if (!strlen(orbtype[i]))
            continue;
        if (!strstr(this->orbtype[i], "ORB_POSTBRD") &&
            !strstr(this->orbtype[i], "ORB_POSTPRE") &&
            !strstr(this->orbtype[i], "ORB_RTBRD") &&
            !strstr(this->orbtype[i], "ORB_RTPRE") &&
            !strstr(this->orbtype[i], "ORB_RTFTP"))
        {
            LOGPRINT_EX(
                "orbtype = %s,key = orbtype,only support "
                "ORB_POSTBRD,ORB_POSTPRE,ORB_RTBRD and ORB_RTPRE,ORB_RTFTP",
                orbtype[i]);
            exit(1);
        }
    }
    for (int i = 0; i < MAXORBEPHTYPE; i++)
    {
        if (!strlen(clktype[i]))
            continue;
        if (!strstr(this->clktype[i], "CLK_POSTBRD") &&
            !strstr(this->clktype[i], "CLK_POSTPRE") &&
            !strstr(this->clktype[i], "CLK_RTBRD") &&
            !strstr(this->clktype[i], "CLK_RTPRE"))
        {
            LOGPRINT_EX(
                "orbtype = %s,key = clktype,only support "
                "CLK_POSTBRD,CLK_POSTPRE,CLK_RTBRD and CLK_RTPRE",
                clktype[i]);

            exit(1);
        }
    }

    LOG("%-20s: %s", "program", str_promode.at(promode).c_str());
    LOG("%-20s: %s", "post", lpost ? "true" : "false");
    if (lpost)
    {
        LOG("%-20s: %s", "obs start", run_timefmt(mjd0, sod0, strbuf));
        LOG("%-20s: %s", "obs end", run_timefmt(mjd1, sod1, strbuf));
    }
    LOG("%-20s: %lf", "intv", dintv);
    LOG("%-20s: %s", "uobs opt", uobs);
    LOG("%-20s: %s", "comb opt", cobs);
    LOG("%-20s: %s", "ionos opt", ionmod);
    LOG("%-20s: %s", "ionpri opt", ionpri);
    LOG("%-20s: %s", "tropo opt", ztdmod);
    LOG("%-20s: %s", "orbit opt", orbtype);
    LOG("%-20s: %s", "clk opt", clktype);
    LOG("%-20s: %s", "amb mode", !liar ? "float" : "fix");

    if (strstr(cobs, "IF") && strstr(ionmod, "ION_EST"))
    {
        LOGPRINT_EX(
            "cobs = %s,ionmod = %s,ignoring the %s mode when use %s,reset the "
            "ionmod ION_NONE",
            cobs, ionmod, cobs, ionmod);

        strcpy(ionmod, "ION_NONE");
    }
    if ((!strstr(uobs, "PHASE") && !strstr(uobs, "GRAPHIC")) && ldia)
    {
        LOGPRINT_EX(
            "uobs = %s,dia = true,dia should not be set when phase is not "
            "used,reset the dia flag",
            uobs);

        ldia = false;
    }
    if (access(this->qcdir, 0) != 0)
    {
        LOGPRINT_EX("dir = %s,qclog dir is not exist", qcdir);
    }
    if (access(this->outdir, 0) != 0)
    {
        LOGPRINT_EX("dir = %s,output dir is not exist", outdir);
    }

    return true;
}

void Deploy::m_parseMWconfigure(Upd_ambconfig &config, string wltag, string file)
{
    /* check the command here */
    char tag[1024] = {0}, value[1024] = {0};
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    Json::CharReaderBuilder rbuilder;
    Json::Value root;
    JSONCPP_STRING errs;
    fstream f;
    f.open(file.c_str(), ios::in);
    if (!f.is_open())
    {
        LOGPRINT_EX("file = %s,can't open upd configuration file to read!",
                    file.c_str());
        exit(1);
    }
    bool parse_ok = Json::parseFromStream(rbuilder, f, &root, &errs);
    if (!parse_ok)
    {
        LOGPRINT_EX("file = %s,failed to parse upd configuration file!",
                    file.c_str());
        exit(1);
    }
    try
    {
        int nsz = root["obs_smoother"]["smooth"].size();
        for (int isz = 0; isz < nsz; ++isz)
        {
            bamboo::excludeAnnoValue(
                tag, root["obs_smoother"]["smooth"][isz]["tag"].asCString());
            if (wltag != tag)
                continue;
            bamboo::excludeAnnoValue(
                value,
                root["obs_smoother"]["smooth"][isz]["value"].asCString());
            /// parse the config here
            m_parseFreqValue(value, config.comb);
            bamboo::excludeAnnoValue(
                value,
                root["obs_smoother"]["smooth"][isz]["minelev"].asCString());
            config.minelev = atof(value) * DEG2RAD;
            bamboo::excludeAnnoValue(
                value, root["obs_smoother"]["smooth"][isz]["mincommtime"]
                           .asCString());
            config.mincommtime_round = atof(value);
            ///
            bamboo::excludeAnnoValue(
                value, root["obs_smoother"]["smooth"][isz]["ion"]["switch"]
                           .asCString());
            if (strstr(value, "on"))
                config.bioncor = true;

            bamboo::excludeAnnoValue(
                value, root["obs_smoother"]["smooth"][isz]["ion"]["value"]
                           .asCString());
            m_parseFreqValue(value, config.comb_ion);
            bamboo::excludeAnnoValue(
                value,
                root["obs_smoother"]["smooth"][isz]["outfile"].asCString());
            config.outfile = value;
            bamboo::excludeAnnoValue(
                value,
                root["obs_smoother"]["smooth"][isz]["updfile"].asCString());
            config.updfile = value;
            break;
        }
        if (wltag != tag)
        {
            LOGPRINT_EX("tag = %s, no smooth tag found in %s", wltag.c_str(), file.c_str());
            exit(1);
        }
    }
    catch (...)
    {
        LOGPRINT_EX("file = %s,upd configuration format error!", file.c_str());
        exit(1);
    }
}
/// @brief parse ambiguity combination
/// @param config
/// @param freq_seq
void Deploy::m_parseAMBFrequency(Upd_ambconfig &config, char *freq_seq)
{
    int i, iq, nfreq_p[MAXSYS] = {0};
    char freq_p[MAXSYS][MAXFREQ][LEN_FREQ];
    m_parseFrequency(freq_p, nfreq_p, freq_seq);
    sortf(nfreq_p, freq_p);
    for (i = 0; i < MAXSYS; ++i)
    {
        if (config.bresolve && strstr(config.ambtype, "NL") && nfreq_p[i] != 2 && -1 != index_string(system, SYS[i]))
        {
            LOGPRINT("tag = %s, ambiguity config NL is set but number of %c comb freqeuency is < 2", config.tag.c_str(), SYS[i]);
            exit(1);
        }
        for (iq = 0; iq < nfreq_p[i] && iq < 2; ++iq)
        {
            config.iq_comb[i][iq] = pointer_charstr(nfreq[i], LEN_FREQ, (char *)freq[i], freq_p[i][iq]);
            if (config.iq_comb[i][iq] == -1 && config.bresolve && -1 != index_string(system, SYS[i]))
            {
                LOGPRINT("tag = %s, freq = %s, parse AMB frequency failed, %s is not in freq",
                         config.tag.c_str(), freq_p[i][iq], freq_p[i][iq]);
                exit(1);
            }
        }
        if (nfreq_p[i] < 2)
        {
            config.iq_comb[i][1] = config.iq_comb[i][0]; /* 占位， N1有时候只有一个 */
        }
    }
}
void Deploy::m_parseAMBWavelength(Upd_ambconfig &config)
{
    /* RAW mode */
    int i, isys;
    for (i = 0; i < nprn; ++i)
    {
        isys = index_string(SYS, prn_alias[i][0]);
        if (config.iq_comb[isys][0] != -1 && config.iq_comb[isys][1] != -1)
        {
            double fq0 = freqbytp(i, freq[isys][config.iq_comb[isys][0]]);
            double fq1 = freqbytp(i, freq[isys][config.iq_comb[isys][1]]);

            config.lamdw[i] = VEL_LIGHT / (fq0 - fq1);
            config.lamdn[i] = VEL_LIGHT / (fq0 + fq1);

            config.g[isys] = fq0 / fq1;
        }
    }
    if (strstr(config.ambtype, "mw"))
    {
        int i, j, isys, nfreq_sp, ifq, iq_set, bvalid = false;
        char freq_c[LEN_STRING] = {0}, freq_sp[2 * MAXFREQ][LEN_STRING];
        // isys = index_string(SYS, dly->cprn[isat][0]);
        for (auto &comb : config.comb)
        {
            strcpy(freq_c, comb.second.c_str());
            isys = index_string(SYS, comb.first);
            if (-1 == index_string(system, SYS[isys]))
                continue;
            split_string(true, freq_c, ' ', ' ', '_', &nfreq_sp, (char *)(freq_sp), LEN_STRING);
            for (ifq = 0; ifq < nfreq_sp; ++ifq)
            {
                bvalid = false;
                string key = string(freq_sp[ifq]).substr(0, 3);
                if (key[0] != 'L')
                    continue;

                if (-1 == (iq_set = pointer_charstr(nfreq[isys], LEN_FREQ, (char *)freq[isys], key.substr(1, 2).c_str())))
                {
                    LOGPRINT("freq = %c:%s, mw configure set error, no this frequency in freqused!", SYS[isys], key.substr(1, 2).c_str());
                    exit(1);
                }
                for (j = 0; j < 2; ++j)
                {
                    if (config.iq_comb[isys][j] == iq_set)
                        bvalid = true;
                }
                if (bvalid == false)
                {
                    LOGPRINT("freq = %c:%s, no this frequency in %s:comb!", SYS[isys], key.substr(1, 2).c_str(), config.tag.c_str());
                    exit(1);
                }
            }
        }
    }
}
void Deploy::m_parseFreqValue(char *freq_seq, map<char, string> &comb_in)
{
    /* parse the config to */
    int i, nsys, isys, j;
    char freq[MAXSYS][LEN_STRING] = {0};
    comb_in.clear();
    split_string(true, freq_seq, ' ', ' ', ' ', &nsys, (char *)freq, LEN_STRING);
    for (i = 0; i < nsys; ++i)
    {
        isys = -1;
        j = index_string(freq[i], ':');
        if (strstr(freq[i], "GPS") != NULL)
            isys = index_string(SYS, 'G');
        else if (strstr(freq[i], "GLS") != NULL)
            isys = index_string(SYS, 'R');
        else if (strstr(freq[i], "GAL") != NULL)
            isys = index_string(SYS, 'E');
        else if (strstr(freq[i], "BD2") != NULL)
            isys = index_string(SYS, 'B');
        else if (strstr(freq[i], "BDS") != NULL ||
                 strstr(freq[i], "BD3") != NULL)
            isys = index_string(SYS, 'C');
        else if (strstr(freq[i], "QZS") != NULL)
            isys = index_string(SYS, 'J');
        else
        {
            LOGPRINT_EX("freq = %s: unknown GNSS system", freq[i]);
            LOGE("freq = %s: unknown GNSS system", freq[i]);
            exit(1);
        }
        if (isys != -1)
        {
            comb_in[SYS[isys]] = freq[i] + j + 1;
        }
    }
}
void Deploy::m_parseAmbiguityConfigures(const char *pfile)
{
    int i, j, isys;
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    Json::Value root;
    fstream f;
    string curItem;
    char value[1024] = {0};
    const char *type[] = {"ewl", "wl", "n1"};
    JSONCPP_STRING errs;
    f.open(pfile, ios::in);
    if (!f.is_open())
    {
        LOGPRINT_EX("file = %s,can't open file to write!", pfile);
        LOGE("file = %s,can't open file to write!", pfile);
    }
    bool parse_ok = Json::parseFromStream(rbuilder, f, &root, &errs);
    f.close();
    if (!parse_ok)
    {
        LOGPRINT_EX("file = %s,failed to parse file", pfile);
        LOGE("file = %s,failed to parse file", pfile);
        return;
    }
    try
    {
        string item = "amb";
        for (j = 0; j < sizeof(type) / sizeof(char *); ++j)
        {
            const char *tp = type[j];
            Upd_ambconfig &config = strstr(type[j], "ewl") ? ewl_config : (strstr(type[j], "wl") ? wl_config : nl_config);
            config.tag = type[j];
            bamboo::excludeAnnoValue(value, root[item][tp]["upd_switch"].asCString());
            if (strstr(value, "on") || strstr(value, "ON"))
                config.bupd_use = true;
            bamboo::excludeAnnoValue(value, root[item][tp]["cutoff_amb"].asCString());
            sscanf(value, "%lf", &config.minelev);
            config.minelev = config.minelev * DEG2RAD;
            bamboo::excludeAnnoValue(config.fixmode, root[item][tp]["fixmode"].asCString());
            bamboo::excludeAnnoValue(config.updtype, root[item][tp]["upd_type"].asCString());
            bamboo::excludeAnnoValue(config.ambtype, root[item][tp]["amb_type"].asCString());
            /* analyse configure */
            bamboo::excludeAnnoValue(value, root[item][tp]["analyse"]["minseccommon"].asCString());
            sscanf(value, "%lf", &config.mincommtime_round);
            bamboo::excludeAnnoValue(value, root[item][tp]["analyse"]["decision"].asCString());
            sscanf(value, "%lf%lf%lf", &config.maxdev, &config.maxsig, &config.minalpha);

            bamboo::excludeAnnoValue(value, root[item][tp]["lambda"]["minseccommon"].asCString());
            sscanf(value, "%lf", &config.mincommtime_lam);
            bamboo::excludeAnnoValue(value, root[item][tp]["lambda"]["success_rate"].asCString());
            sscanf(value, "%lf", &config.success_rate);
            bamboo::excludeAnnoValue(value, root[item][tp]["lambda"]["fix_individual"].asCString());
            if (strstr(value, "true"))
                config.bfix_individual = true;
            /* holding configure */
            bamboo::excludeAnnoValue(value, root[item][tp]["hold"][0].asCString());
            sscanf(value, "%lf", &config.holdcutoff);
            config.holdcutoff = config.holdcutoff * DEG2RAD;
            bamboo::excludeAnnoValue(value, root[item][tp]["hold"][1].asCString());
            sscanf(value, "%lf", &config.holdtimelim);
            bamboo::excludeAnnoValue(value, root[item][tp]["hold"][2].asCString());
            sscanf(value, "%lf", &config.holdcutlim);
            bamboo::excludeAnnoValue(value, root[item][tp]["resolution"].asCString());
            if (strstr(value, "on"))
                config.bresolve = liar;
            if (root[item][tp]["lambda"].isMember("fix_ifuplayer"))
            {
                bamboo::excludeAnnoValue(value, root[item][tp]["lambda"]["fix_ifuplayer"].asCString());
                if (strstr(value, "false") || strstr(value, "off"))
                    config.b_tryfix = false;
            }

            bamboo::excludeAnnoValue(value, root[item][tp]["comb"].asCString());
            m_parseAMBFrequency(config, value);
            if (strstr(config.ambtype, "mw"))
            {
                /* parse the smoothconfig */
                bamboo::excludeAnnoValue(value, root[item][tp]["analyse"]["smoothconfig"].asCString());
                m_parseMWconfigure(config, tp, value);
                /* parse the wavelength */
            }
            m_parseAMBWavelength(config);
            /* check configure */
            if (!strstr(config.fixmode, "round") && !strstr(config.fixmode, "lambda"))
            {
                LOGPRINT_EX("%s:fixmode = %s, fixmode only support 'round' and 'lambda'", tp, config.fixmode);
                exit(1);
            }
            if (!strstr(config.updtype, "snx") && !strstr(config.updtype, "comb"))
            {
                LOGPRINT_EX("%s:upd_type = %s, only support 'snx' and 'comb'", tp, config.updtype);
                exit(1);
            }
            if (!strstr(config.ambtype, "mw") && !strstr(config.ambtype, "raw") && !strstr(config.ambtype, "NL"))
            {
                LOGPRINT_EX("%s:amb_type = %s, only support 'mw' 'raw' and 'NL' ", tp, config.ambtype);
                exit(1);
            }
            if (strstr(cobs, "IF") && strstr(tp, "n1") && !strstr(config.ambtype, "NL"))
            {
                LOGPRINT_EX("%s:amb_type = %s, N1 only support 'NL' for IF ", tp, config.ambtype);
                exit(1);
            }
        }
        /* check NL config, WL and NL should use the same type */
        if (strstr(nl_config.ambtype, "NL"))
        {
            for (int is = 0; is < nsys; ++is)
            {
                isys = index_string(SYS, system[is]);
                if (nl_config.bresolve && wl_config.bresolve)
                {
                    if (wl_config.iq_comb[isys][0] == -1 || wl_config.iq_comb[isys][1] == -1 || nl_config.iq_comb[isys][0] == -1 || nl_config.iq_comb[isys][1] == -1)
                    {
                        LOGPRINT("wl or nl comb configure set error for %c!", SYS[isys]);
                        exit(1);
                    }
                    if (wl_config.iq_comb[isys][0] != nl_config.iq_comb[isys][0] || wl_config.iq_comb[isys][1] != nl_config.iq_comb[isys][1])
                    {
                        LOGPRINT("wl and NL comb configure is set different for %c!", SYS[isys]);
                        exit(1);
                    }
                }
            }
        }
        bamboo::excludeAnnoValue(value, root[item]["osb_switch"].asCString());
        if (strstr(value, "on") || strstr(value, "ON"))
            b_osbswitch = true;
        /* read table */
        if (pf_rtable == "" && mTable.find("ratio") != mTable.end())
            pf_rtable = mTable["ratio"];
    }
    catch (...)
    {
        LOGPRINT_EX("file = %s,can't parse file updating!", pfile);
        return;
    }
}
