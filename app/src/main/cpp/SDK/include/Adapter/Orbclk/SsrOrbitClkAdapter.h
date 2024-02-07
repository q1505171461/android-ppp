//
// Created by juntao, at wuhan university   on 2020/11/3.
//
#ifndef BAMBOO_SSRORBITCLKADAPTER_H
#define BAMBOO_SSRORBITCLKADAPTER_H
#include "include/Adapter/Orbclk/OrbitClkAdapter.h"
#include "BrdOrbitClkAdapter.h"
#include <mutex>
namespace bamboo
{
    class SsrOrbitClkAdapter : public BrdOrbitClkAdapter
    {
    public:
        SsrOrbitClkAdapter();
        virtual int v_readOrbit(const char *cprn, int mjd, double sod, double *xsat);
        virtual int v_readClk(const char *cprn, int mjd, double sod, double *sclk);
        // for real-time oc
        virtual int v_readOrbit(const char *cprn, int mjd, double sod, double *xsat, int *iode);
        virtual int v_readClk(const char *cprn, int mjd, double sod, double *sclk, int *iode);

        virtual int v_readClkDrift(const char *cprn, int mjd, double sod, double *vclk)
        {
            return BrdOrbitClkAdapter::v_readClkDrift(cprn, mjd, sod, vclk);
        }
        virtual void v_inputSsr(SSRORB *, SSRCLK *); // ssr correction

        virtual void v_openOutFile(string, double ver);
        virtual void v_outFileEph();
        virtual void v_closeOutFile();

        inline Ssr_orbclk m_getCurrentSsr() { return m_orbclks.back(); }

    protected:
        char f_out[1024];
        double ptime_chk;
        std::mutex q_mutex;
        Ssr_orbclk m_curSsr;
        list<Ssr_orbclk> m_orbclks;
    };
}
#endif // BAMBOO_SSRORBITCLKADAPTER_H
