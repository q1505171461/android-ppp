#include "include/IO_rtcm.h"
#include "include/Rtklib/rtklib_fun.h"
#include "include/Interface/Interface.h"
#include <android/log.h>
#include <sstream>
#include <iostream>
#define LOG_TAG "YourAppTag"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)


namespace KPL_IO
{
    static rtcm_t s_rtcm_obs;
    static rtcm_t s_rtcm_eph;
    static rtcm_t s_rtcm_ssr;
    static bool s_isInit = false;
    extern void SDK_init(const char *mode, const char *ant, double *refx, double *enu, double cut, double intv, const char * config_path)
    {
        if (!s_isInit)
        {
            KPL_IO::sta_t sta;
            strcpy(sta.antdes, ant);
            memcpy(sta.pos, refx, sizeof(double) * 3);
            memcpy(sta.del, enu, sizeof(double) * 3);
            init_rtcm(&s_rtcm_obs);
            init_rtcm(&s_rtcm_eph);
            init_rtcm(&s_rtcm_ssr);
            KPL_config_path = config_path;
            KPL_initialize(mode, &sta, cut, intv);
            s_isInit = true;
        }
    }
    extern void SDK_terminate()
    {
        if (s_isInit)
        {
            s_isInit = false;
            free_rtcm(&s_rtcm_obs);
            free_rtcm(&s_rtcm_eph);
            free_rtcm(&s_rtcm_ssr);
            KPL_finilize();
        }
    }
    extern void SDK_restart()
    {
        KPL_restart();
    }
    extern void SDK_setintv(double intv)
    {
        KPL_setintv(intv);
    }
    int IO_inputObsData(uint8_t data)
    {
        int ret = 0;
        if (0 == (ret = input_rtcm3(&s_rtcm_obs, data)))
            return 0;
        switch (ret)
        {
        case 1:
            /* observations */
            KPL_inputObs(s_rtcm_obs.time, &s_rtcm_obs.obs);
            break;
        case 2:
            /* navigation ephemeris */
            KPL_inputEph(&s_rtcm_obs.nav, s_rtcm_obs.ephsat, s_rtcm_obs.ephset ? MAXRTKSAT : 0);
            break;
        default:
            /* just return */
            break;
        }
        return ret;
    }
    int IO_inputEphData(uint8_t data)
    {
        int ret = 0;
        if (0 == (ret = input_rtcm3(&s_rtcm_eph, data)))
            return 0;
        switch (ret)
        {
        case 2:
            /* navigation ephemeris */
            KPL_inputEph(&s_rtcm_eph.nav, s_rtcm_eph.ephsat, s_rtcm_eph.ephset ? MAXRTKSAT : 0);
            break;
        default:
            /* just return */
            break;
        }
        return ret;
    }
    int IO_inputSsrData(uint8_t data)
    {
        int ret = 0;
        if (0 == (ret = input_rtcm3(&s_rtcm_ssr, data)))
            return 0;
        switch (ret)
        {
        case 10:
            /* ssr data */
            KPL_inputSsr(s_rtcm_ssr.ssr);
            break;
        case 20:
            /* upd data */
            KPL_inputSsrBias(s_rtcm_ssr.ssr);
            break;
        default:
            /* just return */
            break;
        }
        return ret;
    }

    void SDK_retrieve(const char *type, char *buff, int len){
        if (strcmp(type, "obsinfo") == 0){
            std::basic_stringstream<char> ss;
            ss << time_str(s_rtcm_obs.time, 2) << s_rtcm_obs.msg<< "收到观测值\n";
            strcpy(buff, ss.str().c_str());
            return;
        }
        KPL_retrieve(type, buff, len);
    }
}