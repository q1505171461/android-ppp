/*
 * Rnxobs.h
 *
 *  Created on: 2018/2/3
 *      Author: juntao, at wuhan university
 */

#ifndef INCLUDE_BAMBOO_RNXOBS_H_
#define INCLUDE_BAMBOO_RNXOBS_H_
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Const.h"
using namespace std;
namespace bamboo
{
    class Rnxobs
    {
    public:
        Rnxobs()
        {
            mjd = 0;
            sod = 0;
            memset(snr, 0, sizeof(snr));
            memset(obsstat, 0, sizeof(obsstat));
            memset(obs, 0, sizeof(double) * MAXSAT * 2 * MAXFREQ);
            memset(fob, 0, sizeof(char) * MAXSAT * MAXFREQ * 2 * LEN_OBSTYPE);
        }

    public:
        static bool s_checkObs(int isat, double *obs, double *dop, double *snr, int *obstat);

    public:
        int mjd;                                    /* current time */
        double sod;                                 /* current time */
        double obs[MAXSAT][2 * MAXFREQ];            /* current obs */
        double dop[MAXSAT][MAXFREQ];                /* current dop */
        double snr[MAXSAT][MAXFREQ];                /* current snr */
        char fob[MAXSAT][2 * MAXFREQ][LEN_OBSTYPE]; /* current obs type */
        int obsstat[MAXSAT][MAXFREQ];               /* observation status */
    };

} // namespace bamboo
#endif /* INCLUDE_BAMBOO_RNXOBS_H_ */
