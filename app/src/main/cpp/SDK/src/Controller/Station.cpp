/*
 * Station.cpp
 *
 *  Created on: 2018/2/7
 *      Author: juntao, at wuhan university
 */
#include "include/Controller/Station.h"
#include <iostream>
#include <string>
#include "include/Utils/Com/Com.h"
#include "include/Utils/Com/Logtrace.h"
#include "include/Controller/Controller.h"
#include "include/Controller/Deploy.h"
using namespace std;
using namespace bamboo;
// Done at 8.26,change this shit
Station::Station(string staname)
{
    int isat, isys;
    memset(skd, 0, sizeof(skd));
    memset(clk, 0, sizeof(clk));
    memset(pcv, 0, sizeof(pcv));
    memset(x, 0, sizeof(x));
    memset(geod, 0, sizeof(geod));
    memset(rot_l2f, 0, sizeof(rot_l2f));
    memset(enu, 0, sizeof(enu));
    memset(enu0, 0, sizeof(enu0));
    memset(rclock, 0, sizeof(rclock));
    memset(olc, 0, sizeof(olc));
    memset(vdop, 0, sizeof(vdop));
    memset(refx, 0, sizeof(refx));
    memset(sys2code, 0, sizeof(sys2code));

    // initial value
    strcpy(clk, "CLK");
    skd[0] = 'K';
    dx0[0] = dx0[1] = dx0[2] = 1;
    qx[0] = qx[1] = qx[2] = 1;
    cutoff = 7.0 * DEG2RAD;
    dztd0 = 0.1;
    qztd = 4e-4;
    dion0 = qion = 20; // meters
    zdd = zwd = dgrd0 = qgrd = 0.0;
    for (isys = 0; isys < MAXSYS; isys++)
    {
        sigr[isys] = 0.3;
        sigp[isys] = 0.003;
        dclk0[isys] = 9000;
        qclk[isys] = 900;
    }
    this->name = staname;
    v = R = NULL;
    idx = NULL;
    nb = 0;
}
Station::~Station() {}
