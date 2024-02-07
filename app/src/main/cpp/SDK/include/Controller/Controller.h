//
// Created by juntao on 2020/11/4.
//
#ifndef BAMBOO_CONTROLLER_H
#define BAMBOO_CONTROLLER_H
#include <map>
#include <vector>
#include "Rnxobs.h"
#include "include/Rtklib/rtklib.h"
#include "include/Controller/Deploy.h"
#include "include/Utils/Com/Logstream.h"
#include "include/Adapter/Ifpb/IfpbAdapter.h"
#include "include/Adapter/Upd/UpdAdapter.h"
#include "include/Adapter/Antobx/YawAttitudeAdapter.h"
#include "include/Adapter/Orbclk/OrbitClkAdapter.h"
#include "include/Adapter/CodeBias/CodeBiasAdapter.h"
namespace bamboo
{
    class Controller
    {
    public:
        Controller();
        ~Controller();
        void _initilize(const char *, KPL_IO::sta_t *, double, double); /* start up */
        void _finalize();
        int v_inputObs(Rnxobs *ob);
        void m_setIntv(double intv);

    public:
        inline Deploy *m_getConfigure() { return &dly; } /* get current configure */
        static Controller *s_getInstance();              /* single instance mode */
    public:
        /* process functions */
        void m_corrdcb(Rnxobs *);       /* correct code dcb */
        void m_corrosb(Rnxobs *ob);     /* correct phase osb */
        void m_setStartEpoch(time_t t); /* setting current start epoch */
        /* products generater */
        void m_makeupOrbclkCorrs(int mjd, double sod);
        Station m_initStation(KPL_IO::sta_t *t_n, double cut, char);
        /* adapter getters */
        inline UpdAdapter *m_getUpdAdapter() { return &mUpdAdapter; }          /* return upd adater */
        inline IfpbAdapter *m_getIfpbAdapter() { return &ifpbAdapter; }        /* return ifpb adapter */
        inline YawAttitudeAdapter &m_getAttAdapter() { return m_att_adapter; } /* return att adapter */
        inline OrbitClkAdapter *m_getOacAdapter() { return mOacs; }            /* return clock and orbit adapter */
        inline CodeBiasAdapter *m_getCodeBiasAdapter() { return &mCodebias; }  /* return codebias */
    protected:
        int m_argc;                       /* number of args */
        char *m_args[1024];               /* args */
        time_t t_startepoch;              /* start time epoch */
        Deploy dly;                       /* configures */
        UpdAdapter mUpdAdapter;           /* osb correction adapter */
        IfpbAdapter ifpbAdapter;          /* Ifpb correction adapter */
        OrbitClkAdapter *mOacs;           /* orbit adapters */
        YawAttitudeAdapter m_att_adapter; /* attitude adapter */
        CodeBiasAdapter mCodebias;        /* code bias adapter */
        static Controller *sInstance;     /* static instance */
    };
} // namespace bamboo
#endif // BAMBOO_CONTROLLER_H