//
// Created by juntao, at wuhan university   on 2020/11/2.
//

#ifndef BAMBOO_ORBITCLKADAPTER_H
#define BAMBOO_ORBITCLKADAPTER_H
#include <string>
#include <iostream>
#include "include/Controller/Const.h"
#ifdef USEEIGEN
#include </usr/local/include/USEEIGEN3/USEEIGEN/Core>
#include </usr/local/include/USEEIGEN3/USEEIGEN/Geometry>
#endif
namespace bamboo
{
    /// Interface
    class sat_orb
    {
    public:
        char cprn[LEN_PRN];
        int iode;
        double orb[6];
    };
    class sat_clk
    {
    public:
        char cprn[LEN_PRN];
        int iode;
        double clk[3];
    };
    // orbit for one epoch
    class orbit
    {
    public:
        int mjd;
        double sod;
        map<string, sat_orb> ORB;
    };
    // clk for one epoch
    class clk
    {
    public:
        int mjd;
        double sod;
        map<string, sat_clk> CLK;
    };

    class GPSEPH
    {
    public:
        GPSEPH()
        {
            mjd = 0;
            sod = 0.0;
            memset(cprn, 0, sizeof(cprn));
            signal_idx = 0;
            time(&gent);
        }
        char cprn[LEN_PRN];
        int mjd;
        double sod;
        // a[0]: SV clock offset
        // a[1]: SV clock drift
        // a[2]: SV clock drift rate
        double a0, a1, a2;
        // aode: age of ephemeris upload
        // crs, crc: Ortital radius correction
        // dn: Mean motion difference
        // m0: Mean anomaly at reference epoch
        // e: Eccentricity
        // cuc, cus: Latitude argument correction
        // roota: Square root of semi-major axis
        unsigned int aode;
        double crs, dn;
        double m0, cuc, e;
        double cus, roota;
        // toe, week: Ephemerides reference epoch in seconds with the week
        // cis, cic: Inclination correction
        // omega0: Longtitude of ascending node at the begining of the week
        // i0: Inclination at reference epoch
        // omega: Argument of perigee
        // omegadot: Rate of node's right ascension
        double toe, cic, omega0;
        double cis, i0, crc;
        double omega, omegadot;
        // idot: Rate of inclination angle
        // sesvd0:
        // resvd1:
        // accu: SV accuracy
        // hlth: SV health
        // tgd: Time group delay
        // aodc: Age of clock parameter upload
        double idot, resvd0, week, resvd1;
        double accu, hlth, tgd, tgd1, aodc;
        double iodc, signal_idx, delta_A, A_DOT, delta_n_dot;
        time_t gent;
    };
    class GLSEPH
    {
    public:
        GLSEPH()
        {
            mjd = 0;
            sod = 0.0;
            memset(cprn, 0, sizeof(cprn));
            time(&gent);
        }
        char cprn[LEN_PRN];
        int mjd, aode;
        double sod;
        // tau: SV clock bias
        // gama: SV relative frequency bias
        // tk: message frame time (tk+nd*86400) in seconds of the UTC week
        // pos: coordinate at ephemerides reference epoch in PZ-90
        // vel: velocity at ephemerides reference epoch in PZ-90
        // acc: acceleration at ephemerides reference epoch in PZ-90
        // health: SV health
        // frenum: frequency number
        // age: age of operation information
        double tau;
        double gamma;
        double tk;
        double pos[3];
        double vel[3];
        double acc[3];
        double health;
        double frenum;
        double age;
        time_t gent;
    };
    class SSRCLK
    {
    public:
        SSRCLK() { m_reset(); }
        void m_reset()
        {
            int isat;
            wk = 0;
            sow = 0.0;
            memset(c, 0, sizeof(double) * MAXSAT * 3);
            for (isat = 0; isat < MAXSAT; isat++)
            {
                iod[isat] = -1;
                yaw[isat] = 999;
                yrate[isat] = 999;
            }
        }
        int wk;
        double sow;
        int iod[MAXSAT];
        double c[MAXSAT][3];
        double yaw[MAXSAT];
        double yrate[MAXSAT];
#ifdef USEEIGEN
        map<string, ::USEEIGEN::Quaterniond> c2Q;
#endif
    };
    class SSRORB
    {
    public:
        SSRORB() { m_reset(); }
        void m_reset()
        {
            int isat;
            wk = 0;
            sow = 0.0;
            memset(dx, 0, sizeof(double) * MAXSAT * 6);
            for (isat = 0; isat < MAXSAT; isat++)
                iod[isat] = -1;
        }
        int wk;
        double sow;
        int iod[MAXSAT];
        double dx[MAXSAT][6];
    };
    class Ssr_orbclk
    {
    public:
        Ssr_orbclk()
        {
            int isat;
            wk = 0;
            sow = 0.0;
            memset(c, 0, sizeof(double) * MAXSAT * 3);
            memset(dx, 0, sizeof(double) * MAXSAT * 6);
            for (isat = 0; isat < MAXSAT; isat++)
            {
                iod_orb[isat] = -1;
                iod_clk[isat] = -1;
                yaw[isat] = yrate[isat] = 999;
            }
        }
        int wk;
        double sow;
        double yaw[MAXSAT];
        double yrate[MAXSAT];
        int iod_orb[MAXSAT];
        int iod_clk[MAXSAT];
        double c[MAXSAT][3];
        double dx[MAXSAT][6];
#ifdef USEEIGEN
        map<string, ::USEEIGEN::Quaterniond> c2Q;
#endif
    };
    class SatPos
    {
    public:
        int week;
        double sow;
        double satpos[3]; /* orbit in icrf */
    };
    using SatPosArray = std::vector<SatPos>;
    class SatPos_T
    {
    public:
        SatPos_T()
        {
            dintv = 0.0;
        }
        double dintv; /* interval of the orbit */
        std::map<string, double> acr_accu;
        std::map<std::string, SatPosArray> data;
    };
    class SatClk
    {
    public:
        SatClk()
        {
            week = 0;
            sow = 0.0;
            memset(a, 0, sizeof(a));
        }
        int week;
        double sow;
        double a[3];
    };
    using SatClkArray = std::vector<SatClk>;
    using SatClk_T = std::map<std::string, SatClkArray>;
    class OrbitAdapter
    {
    public:
        OrbitAdapter()
        {
            orbType = ORB_NONE;
        }
        virtual ~OrbitAdapter()
        {
        }
        virtual int v_readOrbit(const char *cprn, int mjd, double sod, double *xsat)
        {
            cout << "***(ERROR):Base Class is not implement!" << endl;
            return 0;
        }
        virtual int v_readOrbit(const char *cprn, int mjd, double sod, double *xsat, int *iode)
        {
            cout << "***(ERROR):Base Class is not implement!" << endl;
            return 0;
        }
        int m_getOrtType()
        {
            return orbType;
        }

    protected:
        int orbType;
    };
    class ClkAdapter
    {
    public:
        ClkAdapter()
        {
            clkType = CLK_NONE;
        }
        virtual ~ClkAdapter()
        {
        }
        virtual int v_readClk(const char *cprn, int mjd, double sod, double *sclk)
        {
            std::cout << "***(ERROR):Base Class is not implement!" << std::endl;
            return 0;
        }
        virtual int v_readClk(const char *cprn, int mjd, double sod, double *sclk, int *iode)
        {
            std::cout << "***(ERROR):Base Class is not implement!" << std::endl;
            return 0;
        }
        virtual int v_readClkDrift(const char *cprn, int mjd, double sod, double *vclk)
        {
            *vclk = 0.0;
            std::cout << "***(ERROR):Base class is not implement" << std::endl;
            return 0;
        }
        int m_getClkType()
        {
            return clkType;
        }

    protected:
        int clkType;
    };
    class OrbitClkAdapter : public ClkAdapter, public OrbitAdapter
    {
    public:
        OrbitClkAdapter()
        {
            isOutputOpen = false;
            outVer = 3.1;
        }
        virtual ~OrbitClkAdapter()
        {
            v_closeOutFile();
        }
        virtual void v_openOutFile(string, double ver)
        {
            isOutputOpen = true;
        }
        virtual void v_outFileEph()
        {
        }
        virtual void v_closeOutFile()
        {
            isOutputOpen = false;
        }
        virtual bool v_isOutFileOpen()
        {
            return isOutputOpen;
        }
        /// get broadcast ephemeris here
        virtual map<string, GPSEPH> v_getCurrentGpsEph(double ptime) { return {}; }
        virtual map<string, GLSEPH> v_getCurrentGlsEph(double ptime) { return {}; }

        /// input function here
        virtual void v_inputEph(GPSEPH *, GLSEPH *) {} // broadcast ephemeris
        virtual void v_inputSsr(SSRORB *, SSRCLK *) {} // ssr correction
        virtual void v_inputSp3(SatPos_T *) {}         // satellite position
        virtual void v_inputClk(SatClk_T *) {}         // satellite clock
        virtual void v_inputSsr_reply(list<orbit> &, list<clk> &){};

    protected:
        bool isOutputOpen; // output part
        double outVer;
        char outFile[1024];
    };
} // namespace bamboo
#endif // BAMBOO_ORBITCLKADAPTER_H
