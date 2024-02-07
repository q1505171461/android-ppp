//
// Created by juntao on 2021/4/1.
//

#ifndef BAMBOO_UPDADAPTER_H
#define BAMBOO_UPDADAPTER_H
#include <list>
#include <map>
#include <mutex>
#include "include/Controller/Const.h"
using namespace std;
namespace bamboo
{
    /// individual bias for each frequency
#define t_UPD_comb 1
#define t_UPD_bias 2
    class Updc
    {
    public:
        Updc()
        {
            mjd = 0;
            sod = 0.0;
            memset(bias, 0, sizeof(bias));
            memset(comb_bias, 0, sizeof(comb_bias));
        }
        int m_getType() { return tpy; }
        virtual ~Updc(){};
        virtual bool v_checkSatellite(const char *cprn, const char *freq) { return false; }
        int mjd, refsat[MAXSYS], tpy;
        double sod;
        double bias[MAXSAT][MAXFREQ], xsig[MAXSAT][MAXFREQ];
        double comb_bias[MAXSAT], comb_xsig[MAXSAT];
    };

    class Upd_bias : public Updc
    {
    public:
        Upd_bias();
        virtual ~Upd_bias() {}
        void m_reset();
        virtual bool v_checkSatellite(const char *cprn, const char *freq);
    };
    /// combined bias
    class Upd_comb : public Updc
    {
    public:
        Upd_comb();
        virtual ~Upd_comb() {}
        void m_reset();
        double ptime[MAXSAT];
        bool m_validComb(char csys);
        virtual bool v_checkSatellite(const char *cprn, const char *freq);
    };
    class UpdAdapter
    {
    public:
        using t_combmp = map<string, list<Upd_comb>>;
        UpdAdapter();
        void v_inputBias(Upd_bias &);
        void v_inputComb(const char *mode, const char *, Upd_comb &);
        void m_inquireBias(int mjd, double sod, Upd_bias &);
        void m_inquireComb(int mjd, double sod, const char *, const char *mode, Upd_comb &);

        void m_removeComb(const char *tp, const char *mode)
        {
            if (m_tp2m.find(tp) != m_tp2m.end() && m_tp2m[tp].find(mode) != m_tp2m[tp].end())
            {
                m_tp2m[tp][mode].clear();
            }
        };

    protected:
        std::mutex m_lock;
        list<Upd_bias> m_updbias;     /* for bias */
        map<string, t_combmp> m_tp2m; /* snx : t_combmp, "ewl/wl" : t_combmp*/
    };
} // namespace bamboo
#endif // BAMBOO_UPDADAPTER_H
