/*
 * Rnxobs.cpp
 *
 *  Created on: 2018/5/5/
 *      Author: juntao, at wuhan university
 */
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "include/Controller/Controller.h"
#include "include/Controller/Deploy.h"
#include "include/Utils/Com/Com.h"
#include "include/Rtklib/rtklib.h"
#include "include/Rtklib/rtklib_fun.h"
#include "include/Controller/Rnxobs.h"
using namespace std;
using namespace bamboo;
bool Rnxobs::s_checkObs(int psat, double *obs, double *dop, double *snr, int *obsstat)
{
    int ifreq, isys, i, j, k, nrc, npc;
    bool breset = false;
    Deploy *dly = Controller::s_getInstance()->m_getConfigure();
    // isys = index_string(SYS, dly->cprn[psat][0]);
    isys = index_string(SYS, dly->prn_alias[psat][0]);
    //// exclude the third  frequency
    if (strstr(dly->cobs, "SF"))
    {
        //// single frequency mode
        if (strstr(dly->uobs, "CODE"))
        {
            if (obs[MAXFREQ] == 0)
            {
                if (obs[0] != 0.0 || obs[MAXFREQ] != 0)
                {
                    breset = true;
                    for (int i = 0; i < MAXFREQ; ++i)
                        obsstat[i] = DEL_LACK_RAorPH;
                }
                memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                memset(dop, 0, sizeof(double) * MAXFREQ);
                memset(snr, 0, sizeof(double) * MAXFREQ);
            }
        }
        if (strstr(dly->uobs, "PHASE") || strstr(dly->uobs, "GRAPHIC"))
        {
            if (obs[0] == 0 || obs[MAXFREQ] == 0)
            {
                if (obs[0] != 0.0 || obs[MAXFREQ] != 0)
                {
                    for (int i = 0; i < MAXFREQ; ++i)
                        obsstat[i] = DEL_LACK_RAorPH;
                    breset = true;
                }
                memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                memset(dop, 0, sizeof(double) * MAXFREQ);
                memset(snr, 0, sizeof(double) * MAXFREQ);
            }
        }
    }
    else
    {
        if (strstr(dly->uobs, "CODE"))
        {
            if (dly->nfreq[isys] >= 3)
            {
                /* multi frequencies data process*/
                for (i = 0, nrc = 0; i < dly->nfreq_obs[isys]; i++)
                {
                    if (fabs(obs[MAXFREQ + i]) > MAXWND)
                    {
                        nrc = nrc + 1;
                    }
                }
                if (nrc < 2)
                {
                    if (nrc > 0)
                    {
                        for (int i = 0; i < MAXFREQ; ++i)
                            obsstat[i] = obsstat[i] ? obsstat[i] : DEL_LACK_RAorPH;
                        breset = true;
                    }
                    memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                    memset(dop, 0, sizeof(double) * MAXFREQ);
                    memset(snr, 0, sizeof(double) * MAXFREQ);
                }
                /* check the difference here */
                int max_nv = 0, i_max = -1, nv;
                for (i = 0; i < dly->nfreq_obs[isys]; i++)
                {
                    if (fabs(obs[MAXFREQ + i]) < MAXWND)
                        continue;
                    for (j = 0, nv = 0; j < dly->nfreq_obs[isys]; j++)
                    {
                        if (i == j || fabs(obs[MAXFREQ + j]) < MAXWND)
                            continue;
                        if (fabs(obs[i + MAXFREQ] - obs[j + MAXFREQ]) < 50.0)
                            ++nv;
                    }
                    if (nv > max_nv)
                    {
                        max_nv = nv;
                        i_max = i;
                    }
                }
                /*no observation here*/
                if (i_max == -1)
                {
                    if (nrc > 0)
                    {
                        for (int i = 0; i < MAXFREQ; ++i)
                            obsstat[i] = fabs(obs[i + MAXFREQ]) > MAXWND ? DEL_BAD : DEL_NONE; // may be got no observation, so check first
                        breset = true;
                    }
                    memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                    memset(dop, 0, sizeof(double) * MAXFREQ);
                    memset(snr, 0, sizeof(double) * MAXFREQ);
                }
                for (i = 0; i < dly->nfreq_obs[isys] && i_max != -1; i++)
                {
                    if (fabs(obs[MAXFREQ + i]) < MAXWND)
                        continue;
                    if (fabs(obs[i + MAXFREQ] - obs[i_max + MAXFREQ]) > 50.0)
                    {
                        snr[i] = dop[i] = obs[i] = obs[MAXFREQ + i] = 0.0;
                        obsstat[i] = DEL_BAD;
                    }
                }
                //////////////////////////////////////////////////////
                /* check the count second time  */
                for (i = 0, nrc = 0; i < dly->nfreq_obs[isys]; i++)
                {
                    if (fabs(obs[MAXFREQ + i]) > MAXWND)
                        nrc = nrc + 1;
                }
                if (nrc < 2)
                {
                    if (nrc > 0)
                    {
                        for (int i = 0; i < MAXFREQ; ++i)
                            obsstat[i] = obsstat[i] ? obsstat[i] : DEL_LACK_RAorPH;
                        breset = true;
                    }
                    memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                    memset(dop, 0, sizeof(double) * MAXFREQ);
                    memset(snr, 0, sizeof(double) * MAXFREQ);
                }
                ////////////////////////////////////////////////////////
            }
            else
            {
                /* double frequencies data process, if the first of the two
                 * observation has no observation*/
                if (fabs(obs[MAXFREQ]) < MAXWND || fabs(obs[MAXFREQ + 1]) < MAXWND || fabs(obs[MAXFREQ] - obs[MAXFREQ + 1]) > 50.0)
                {
                    if (fabs(obs[MAXFREQ]) > MAXWND || fabs(obs[MAXFREQ + 1]) > MAXWND)
                    {
                        breset = true;
                        bool b_got_4 = fabs(obs[MAXFREQ]) > MAXWND && fabs(obs[MAXFREQ + 1]) > MAXWND;
                        for (int i = 0; i < MAXFREQ; ++i)
                            obsstat[i] = b_got_4 ? DEL_BAD : (obsstat[i] ? obsstat[i] : DEL_LACK_RAorPH);
                    }
                    memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                    memset(dop, 0, sizeof(double) * MAXFREQ);
                    memset(snr, 0, sizeof(double) * MAXFREQ);
                }
            }
        }
        /** check the phase observation here **/
        if (strstr(dly->uobs, "PHASE") || strstr(dly->uobs, "GRAPHIC"))
        {
            /* normal process */
            if (dly->nfreq[isys] >= 3)
            { /* triple frequencies data process*/
                /* check the count first  */
                for (i = 0; i < dly->nfreq_obs[isys]; i++)
                {
                    if (fabs(obs[i]) < MAXWND || fabs(obs[MAXFREQ + i]) < MAXWND)
                    {
                        obsstat[i] = obsstat[i] ? obsstat[i] : DEL_LACK_RAorPH;
                        snr[i] = dop[i] = obs[i] = obs[MAXFREQ + i] = 0.0;
                    }
                }
                for (i = 0, nrc = 0; i < dly->nfreq_obs[isys]; i++)
                {
                    if (fabs(obs[i]) > MAXWND)
                    { /* got both phase and code */
                        nrc = nrc + 1;
                    }
                }
                if (nrc < 2)
                {
                    if (nrc > 0)
                    {
                        for (int i = 0; i < MAXFREQ; ++i)
                            obsstat[i] = obsstat[i] ? obsstat[i] : DEL_LACK_RAorPH;
                        breset = true;
                    }
                    memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                    memset(dop, 0, sizeof(double) * MAXFREQ);
                    memset(snr, 0, sizeof(double) * MAXFREQ);
                }
            }
            else
            {
                if (fabs(obs[0]) < MAXWND || fabs(obs[1]) < MAXWND)
                {
                    if (fabs(obs[0]) > MAXWND || fabs(obs[1]) > MAXWND)
                    {
                        breset = true;
                        for (int i = 0; i < MAXFREQ; ++i)
                            obsstat[i] = obsstat[i] ? obsstat[i] : DEL_LACK_RAorPH;
                    }
                    memset(obs, 0, sizeof(double) * 2 * MAXFREQ);
                    memset(dop, 0, sizeof(double) * MAXFREQ);
                    memset(snr, 0, sizeof(double) * MAXFREQ);
                }
            }
        }
    }
    return breset;
}
