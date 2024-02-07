//
// Created by k on 2024/2/4.
//
#include "include/Adapter/Ifpb/t_ifpb.h"
#include "include/Controller/Rnxobs.h"
#include "include/Adapter/Orbclk/OrbitClkAdapter.h"
#include "include/Adapter/Orbclk/SsrOrbitClkAdapter.h"
#include "include/Adapter/Upd/UpdAdapter.h"
#include "include/Adapter/CodeBias/CodeBiasAdapter.h"
#include "include/Controller/Controller.h"
#include "include/Rtklib/rtklib_fun.h"
#include "include/Utils/Com/Com.h"
#include "include/Interface/Interface.h"
#include <cstdlib>
#include "include/Interface/LoggerSsr.h"

using namespace  KPL_IO;
LoggerSsr* LoggerSsr::sInstance = NULL;
map<string, string> LoggerSsr::pbais = map<string,string>();
LoggerSsr::LoggerSsr() {

}

LoggerSsr* LoggerSsr::s_getInstance() {
    if(sInstance == NULL)
        sInstance = new LoggerSsr;
    return sInstance;
}
string LoggerSsr::getLastSsrInfo(string type) {
    if(type == "orbclk"){
        return m_getOrbclkInfo();
    }
    if(type == "ifpb"){
        return m_getIfpbInfo();
    }
    if(type == "codebias"){
        return m_getCodeBiasInfo();
    }
    if(type == "pbias"){
    }
    if(type == "all"){
        return m_getAllInfo();
    }
    return "";
}

string LoggerSsr::m_getOrbclkInfo(){
    bamboo::Controller* s_con = bamboo::Controller::s_getInstance();
    bamboo::SsrOrbitClkAdapter* ssrclk = (bamboo::SsrOrbitClkAdapter*)s_con->m_getOacAdapter();
    bamboo::Ssr_orbclk orbclk = ssrclk->m_getCurrentSsr();

    /* form string */
    stringstream ss = stringstream();
    char buff[1024];
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    // print utc
    sprintf(buff, "> %s\n",time_str(gpst2time(orbclk.wk,orbclk.sow), 2));
    ss << buff;
    for (int isat = 0; isat < dly->nprn; isat++)
    {
        if (orbclk.iod_orb[isat] != -1)
        {
            sprintf(buff, "%s orb: %10.3lf%10.3lf%10.3lf%10.3lf%10.3lf%10.3lf clk: %10.3lf%10.3lf%10.3lf ",\
                     dly->cprn[isat].c_str(), orbclk.dx[isat][0],orbclk.dx[isat][1],orbclk.dx[isat][2],orbclk.dx[isat][3],orbclk.dx[isat][4], orbclk.dx[isat][5], orbclk.c[isat][0], orbclk.c[isat][1], orbclk.c[isat][2]);
            ss << buff;
            sprintf(buff, "yaw: %10.3lf yrate: %10.3lf ", *orbclk.yaw, *orbclk.yrate);
            ss << buff << std::endl;
        }
    }
    return ss.str();
}

string LoggerSsr::m_getIfpbInfo() {
    bamboo::Controller* s_con = bamboo::Controller::s_getInstance();
    bamboo::IfpbAdapter *cur_ifpb =  s_con->m_getIfpbAdapter();
    bamboo::Ssr_orbclk orbclk  = ((bamboo::SsrOrbitClkAdapter*)s_con->m_getOacAdapter())->m_getCurrentSsr();

    stringstream ss = stringstream();
    char buff[1024];
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    // print utc
    sprintf(buff, "> %s\n",time_str(gpst2time(orbclk.wk,orbclk.sow), 2));
    ss << buff;
    for (int isat = 0; isat < dly->nprn; isat++)
    {
        if (orbclk.iod_orb[isat] != -1)
        {
            sprintf(buff, "%s ifpb: %10.3lf ",dly->cprn[isat].c_str(), cur_ifpb->m_getCorrection(dly->cprn[isat].c_str()).m_sum);
            ss << buff << std::endl;
        }
    }
    return ss.str();
}

string LoggerSsr::m_getCodeBiasInfo() {
    bamboo::Controller* s_con = bamboo::Controller::s_getInstance();
    auto cur_code_bais =  s_con->m_getCodeBiasAdapter()->m_getCodeBias();
    bamboo::Ssr_orbclk orbclk  = ((bamboo::SsrOrbitClkAdapter*)s_con->m_getOacAdapter())->m_getCurrentSsr();
    stringstream ss = stringstream();
    char buff[1024];
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    // print utc
    sprintf(buff, "> %s\n",time_str(gpst2time(orbclk.wk,orbclk.sow), 2));
    ss << buff;
    for (int isat = 0; isat < dly->nprn; isat++)
    {
        if (orbclk.iod_orb[isat] != -1)
        {
            sprintf(buff, "%s cbias: ",dly->cprn[isat].c_str());
            ss << buff << cur_code_bais[dly->cprn[isat].c_str()].size() << " ";
            for (auto one : cur_code_bais[dly->cprn[isat].c_str()]){
                sprintf(buff, "%5s%10.3lf", one.first.c_str(), one.second);
                ss << buff;
            }
            ss << "\n";
        }
    }
    return ss.str();
}

string LoggerSsr::m_getPBiasInfo() {
    bamboo::Controller* s_con = bamboo::Controller::s_getInstance();
    return  "";
}

string LoggerSsr::m_getAllInfo() {
    bamboo::Controller* s_con = bamboo::Controller::s_getInstance();
    bamboo::SsrOrbitClkAdapter* ssrclk = (bamboo::SsrOrbitClkAdapter*)s_con->m_getOacAdapter();
    bamboo::Ssr_orbclk orbclk = ssrclk->m_getCurrentSsr();
    bamboo::IfpbAdapter *cur_ifpb =  s_con->m_getIfpbAdapter();
    auto cur_code_bais =  s_con->m_getCodeBiasAdapter()->m_getCodeBias();

    /* form string */
    stringstream ss = stringstream();
    char buff[1024];
    bamboo::Deploy *dly = bamboo::Controller::s_getInstance()->m_getConfigure();
    // print utc
    sprintf(buff, "> %s\n",time_str(gpst2time(orbclk.wk,orbclk.sow), 2));
    ss << buff;
    for (int isat = 0; isat < dly->nprn; isat++)
    {
        if (orbclk.iod_orb[isat] != -1)
        {
            sprintf(buff, "%s orb: %10.3lf%10.3lf%10.3lf%10.3lf%10.3lf%10.3lf clk: %10.3lf%10.3lf%10.3lf ",\
                     dly->cprn[isat].c_str(), orbclk.dx[isat][0],orbclk.dx[isat][1],orbclk.dx[isat][2],orbclk.dx[isat][3],orbclk.dx[isat][4], orbclk.dx[isat][5], orbclk.c[isat][0], orbclk.c[isat][1], orbclk.c[isat][2]);
            ss << buff;
            sprintf(buff, "yaw: %10.3lf yrate: %10.3lf ", *orbclk.yaw, *orbclk.yrate);
            ss << buff ;
            sprintf(buff, "pbias: %s", pbais[dly->cprn[isat].c_str()].c_str());
            ss << buff ;
            sprintf(buff, " ifpb: %10.3lf " , cur_ifpb->m_getCorrection(dly->cprn[isat].c_str()).m_sum);
            ss << buff ;
            ss << "cbias: " << cur_code_bais[dly->cprn[isat].c_str()].size() << " ";
            for (auto one : cur_code_bais[dly->cprn[isat].c_str()]){
                sprintf(buff, "%5s%10.3lf", one.first.c_str(), one.second);
                ss << buff;
            }
            ss << "\n";
        }
    }
    return ss.str();
}

