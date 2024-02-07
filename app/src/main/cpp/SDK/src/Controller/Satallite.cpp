/*
 * Satallite.cpp
 *
 *  Created on: 2018/8/26
 *      Author: juntao, at wuhan university
 */
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "include/Utils/Com/Com.h"
#include "include/Utils/Com/Logtrace.h"
#include "include/Controller/Controller.h"
#include "include/Controller/Satellite.h"
using namespace std;
using namespace bamboo;
Satellite::Satellite()
{
    ifreq = 0;
    sse_angle = mass = 0.0;
    memset(xyz, 0, sizeof(xyz));
    memset(offs, 0, sizeof(offs));
    memset(satpos, 0, sizeof(satpos));
    memset(clk, 0, sizeof(clk));
    xclk = 0.0;
}
Satellite::~Satellite() {}
