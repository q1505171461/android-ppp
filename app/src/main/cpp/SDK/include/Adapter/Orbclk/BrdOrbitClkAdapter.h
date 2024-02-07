//
// Created by juntao, at wuhan university   on 2020/11/2.
//

#ifndef BAMBOO_BRDORBITCLKADAPTER_H
#define BAMBOO_BRDORBITCLKADAPTER_H
#include "BroadcastEphUtils.h"
#include "OrbitClkAdapter.h"
#include <mutex>
#include <functional>
#include <algorithm>
using namespace std;
namespace bamboo
{
    class BrdOrbitClkAdapter : public OrbitClkAdapter, public BroadcastEphUtils
    {
    public:
        BrdOrbitClkAdapter();
        virtual int v_readOrbit(const char *cprn, int mjd, double sod, double *xsat);
        virtual int v_readClk(const char *cprn, int mjd, double sod, double *sclk);
        // for real-time oc
        virtual int v_readOrbit(const char *cprn, int mjd, double sod, double *xsat, int *iode);
        virtual int v_readClk(const char *cprn, int mjd, double sod, double *sclk, int *iode);

        // for clock drift
        virtual int v_readClkDrift(const char *cprn, int mjd, double sod, double *vclk);
        virtual map<string, GPSEPH> v_getCurrentGpsEph(double ptime);
        virtual map<string, GLSEPH> v_getCurrentGlsEph(double ptime);

        // for output
        virtual void v_openOutFile(string, double ver);
        virtual void v_outFileEph();
        virtual void v_closeOutFile();

        void m_outFileHeader();
        // for member functions
        virtual void v_inputEph(GPSEPH *, GLSEPH *);

    protected:
        void m_inputEphPost(GPSEPH *, GLSEPH *);
        void m_inputEphRealTime(GPSEPH *, GLSEPH *);
        std::mutex q_mutex;
    };
}
#endif // BAMBOO_BRDORBITCLKADAPTER_H
