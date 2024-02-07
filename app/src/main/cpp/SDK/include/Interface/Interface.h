#include "include/Rtklib/rtklib.h"

/* whether the SDK is initialized */
extern int KPL_isInitialize;

/* config_path */
extern const char* KPL_config_path;

/* initialize the SDK */
extern void KPL_initialize(const char *mode, KPL_IO::sta_t *sta, double cut, double intv);

/* finalize the SDK */
extern void KPL_finilize();

/* restart */
extern void KPL_restart();

/* input observations */
extern void KPL_inputObs(KPL_IO::gtime_t, KPL_IO::obs_t *obs);

/* input SSR data */
extern void KPL_inputSsr(KPL_IO::ssr_t *ssr);

/* input broadcast ephemeris data */
extern void KPL_inputEph(KPL_IO::nav_t *nav, int psat, int offset);

/* input Ssr bias */
extern void KPL_inputSsrBias(KPL_IO::ssr_t *ssr);

/* input set the process interval, minimum is 1.0 s */
extern void KPL_setintv(double intv);

extern void KPL_retrieve(const char *, char *, int);