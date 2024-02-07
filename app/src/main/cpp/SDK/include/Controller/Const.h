#ifndef INCLUDE_BAMBOO_CONST_H_
#define INCLUDE_BAMBOO_CONST_H_
#include <string>
#include <cmath>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <map>
#include <list>
#include <iostream>
#include <vector>
#include <functional>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <condition_variable>
#else
#include <pthread.h>
#include <unistd.h>

#include "include/Utils/Com/Logtrace.h"
#endif
using namespace std;
// Done at 8.26
namespace bamboo
{
// #define USEDE405
// #define USEGSL
// #define MKL
// #define USEPCV
// #define USEEIGEN
extern  char  configPath[1024];
#include <android/log.h>
#define LOG_TAG "YourAppTag"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#ifdef DATA_FLOAT
    typedef float Scalar;
#else
    typedef double Scalar;
#endif
#define LOGTAG
#ifdef LOGTAG
#define LOGI(level, format, ...) Logtrace::m_logStatInfo(format "\n", ##__VA_ARGS__);
#define LOGINFO_TAG(tag, format, ...) Logtrace::m_logtagTrace(tag, format "\n", ##__VA_ARGS__);
#define LOGINFO_TAG_EX(tag, format, ...)                                                 \
    {                                                                                    \
        Logtrace::m_logtagTrace(tag, format "\n", ##__VA_ARGS__);                        \
        Logtrace::m_logPrint("[%s,%d]:" format "\n", __func__, __LINE__, ##__VA_ARGS__); \
    }
#define LOGERROR(format, ...) Logtrace::m_logError(format, ##__VA_ARGS__);
#define LOGE(format, ...) Logtrace::m_logTrace(LEVEL_MAX, "[%s,%d]: " format "\n", __func__, __LINE__, ##__VA_ARGS__);
#define LOG(format, ...) Logtrace::m_logNoFormatTrace(format "\n", ##__VA_ARGS__);
#define LOGMAT(mat, lda, n, m, p, q, ball, msg) Logtrace::m_logMatrix(mat, lda, n, m, p, q, ball, msg);
#define LOGSTAT(newline, format, ...) Logtrace::m_logStat(newline, format, ##__VA_ARGS__);
#define LOGSTATINFO(format, ...) Logtrace::m_logStatInfo(format "\n", ##__VA_ARGS__);
#define LOGPRINT(format, ...) Logtrace::m_logPrint("[%s,%d]:" format "\n", __func__, __LINE__, ##__VA_ARGS__);
#define LOGPRINT_EX(format, ...)                                                         \
    {                                                                                    \
        Logtrace::m_logPrint("[%s,%d]:" format "\n", __func__, __LINE__, ##__VA_ARGS__); \
        LOGSTATINFO(format, ##__VA_ARGS__);                                              \
    }
#define LOGSTREAM(format, ...)  \
    {                           \
        LOGPRINT(##__VA_ARGS__) \
    }
#else
#define LOG(format, ...)
#define LOGI(level, format, ...)
#define LOGE(format, ...)
#define LOG(format, ...)
#define LOGMAT(mat, lda, n, m, p, q, ball, msg)
#define LOGSTAT(newline, format, ...)
#define LOGSTATINFO(format, ...)
#define LOGPRINT(format, ...) Logtrace::m_logPrint("[%s@%s,%d]:" format "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__);
#define LOGPRINT_EX(format, ...)                                                                      \
    {                                                                                                 \
        Logtrace::m_logPrint("[%s@%s,%d]:" format "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        LOGSTATINFO(format, ##__VA_ARGS__);                                                           \
    }
#endif
    const unsigned char RTCM2PREAMB = 0x66;   /* rtcm ver.2 frame preamble */
    const unsigned char RTCM3PREAMB = 0xD3;   /* rtcm ver.3 frame preamble */
    const unsigned char ATOMPREAMB = 0xDB;    /* atom preamble */
    const unsigned char ATOMPREAMB_UD = 0xDA; /* atom preamble */
    const int MAXFREQ = 4;
    const int MAXSYS = 7;
    const int MAXSAT = 160;
    const int MAXSIT = 1200;
    const int MAXOC = 120;
    const int MAXOBSTYP = 50;
    const int MAXPORT = 10;
    const int MAXICS = 21;
    const int MAXECLIP = 120;
    const int MAXCOEFPAR = 3 + MAXSYS + 1 + 1 + 2 + MAXSYS; // XYZ RECCLK ZTD AMB ION DCB (ION maybe two,DCB is maxsys)
    const int MAXDGR = 7;
    const int MAXPNT = 2 * (MAXDGR + 1);
    const int MAXVARS = 3 * (MAXICS + 1);
    const int MAXORBEPHTYPE = 8;

    const int MINCMNSIT = 4; // 4;
    const int MAX_GRID = 73;
    const int MeanEarthRadius = 6371000;
    const int IONO_SHELL_H = 450000;
    const int MAXTABLE = 20;
    const int MAXBASE = 20;

    const char SYS[] = "GRECBJS"; /// G:GPS R:GLONASS E:GALILEO B:BDS-2 C:BDS-3 J:QZSS L:LEO
    const double MAXWND = 1E-2;
    const char OBSTYPE[MAXSYS][MAXFREQ][32] = { // WPCIXSAQLDBYMZN  //"WPCIXSAQLDBYMZN ";
        {                                       // G:
         //   {"WPCIXSAQLDBYMZN "},
         //   {"WPCIXSAQLDBYMZN "},
         //   {"WPCIXSAQLDBYMZN "}},
         {"WPCISXAQLDBYMZN "},
         {"WPCISXAQLDBYMZN "},
         {"WPCISXAQLDBYMZN "},
         {"WPCISXAQLDBYMZN "}},
        {// R:
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "}},
        {// E:
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "}},
        {// C:
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "}},
        {// B:
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "}},
        {// J:
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "}},
        {// S:
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "},
         {"WPCIXSAQLDBYMZN "}}};
    const double PI = 3.1415926535897932384626433832795028841971693993;
    const double VEL_LIGHT = 299792458.0;
    const double E_MAJAXIS = 6378136.60;

    const double DEG2RAD = PI / 180.0;
    const double RAD2DEG = 180.0 / PI;

    const double ARCSEC2RAD = PI / (3600.0 * 180.0);
    const double CONE = 2.7182818284590452353602874713526624977572470936999;

    const double EARTH_A = 6378137;
    const double EARTH_ALPHA = 1 / 298.257223563;
    const double ESQUARE = (EARTH_A * EARTH_A - (EARTH_A - EARTH_A * EARTH_ALPHA) * (EARTH_A - EARTH_A * EARTH_ALPHA)) / (EARTH_A * EARTH_A);

    const double OFF_GPS2TAI = 19.0;
    const double OFF_TAI2TT = 32.184;
    const double OFF_GPS2TT = OFF_GPS2TAI + OFF_TAI2TT;
    const double OFF_MJD2JD = 2400000.50;

    const double E_ROTATE = 7.2921151467E-5;
    const double GME = 3.986004415E14; // TT-compatible
    const double GPS_FREQ = 10230000.0;
    const double GPS_L1 = 154 * GPS_FREQ;
    const double GPS_L2 = 120 * GPS_FREQ;
    const double GPS_L5 = 115 * GPS_FREQ;

    const double GLS_FREQ = 178000000.0;
    const double GLS_L1 = 9 * GLS_FREQ;
    const double GLS_L2 = 7 * GLS_FREQ;
    const double GLS_dL1 = 562500.0;
    const double GLS_dL2 = 437500.0;

    const double GAL_E1 = GPS_L1;
    const double GAL_E5 = 1191795000.0;
    const double GAL_E5a = GPS_L5;
    const double GAL_E5b = 1207140000.0;
    const double GAL_E6 = 1278750000.0;

    const double BDS_B1 = 1561098000.0;
    const double BDS_B2b = 1207140000.0;
    const double BDS_B3 = 1268520000.0;
    const double BDS_B1c = 1575420000.0;
    const double BDS_B2a = 1176450000.0;
    const double BDS_B2 = 1191795000.0;
    const double QZS_L1 = GPS_L1;
    const double QZS_L2 = GPS_L2;
    const double QZS_L5 = GPS_L5;
    const double QZS_LEX = 1278750000.0;

    //  const double SNRCONST_FREQ1[3] = {0.00224,0.00077,0.00224};
    const double SNRCONST_FREQ1[3] = {1.61 * 10e-5, 1.61 * 10e-5, 1.61 * 10e-5};

    const int LEN_OBSTYPE = 4;
    const int LEN_FREQ = 4;
    const int LEN_ANTENNA = 32;
    const int LEN_PRN = 5;
    const int LEN_PORT = 256;
    const int LEN_STRING = 1024;
    const int LEN_SATTYPE = 20;
    const int LEN_PCVTYPE = 5;
    const int LEN_CLKTYPE = 5;
    const int LEN_COSPARID = 16;
    const int LEN_SITENAME = 5;
    const int LEN_ZTDMAP = 3;
    const int LEN_EPHION = 4;

    const int ORB_NONE = 0;
    const int CLK_NONE = 0;
    const int ORB_BRD = 1;
    const int CLK_BRD = 1;
    const int ORB_SP3 = 2;
    const int CLK_SP3 = 2;
    const int ORB_SSR = 3;
    const int CLK_SSR = 3;

    const int PRO_PPP = 0;
    const int PRO_CLK = 1;
    const int PRO_RTK = 2;
    const int PRO_NRTK = 3;
    const int PRO_VRS = 4;
    const int PRO_RTORB = 5;

    const int PRO_EXTUPD_WL = 6;
    const int PRO_HMW_WL = 7;

    const int PRO_EXTFCB_NL = 8;
    const int PRO_VISUAL = 9;
    const int PRO_QC = 10;
    const int PRO_ESTIFPB = 11;
    const int PRO_MODELZTD = 12;
    const int PRO_UPD2OSB = 13;
    const int PRO_UPDPOST = 14;
    const int PRO_UPDVERIFY = 15;
    const int PRO_TRIOSB = 16;
    const int PRO_BACKIAR = 17;
    const int PRO_PPPCENTER = 18;
    const int PRO_NRTKATOM = 19;
    const int PRO_DSTEC = 20;
    const int PRO_RTKV = 21;

    const int ATOMTP_ION = 1;
    const int ATOMTP_ZTD = 2;
    const int ATOMTP_OFCZTD = 3;

    const int ATOMTP_NONE = 0;
    const int ATOMTP_INTP = 1;
    const int ATOMTP_FIX = 2;
    const int MAXPAM = 20;
    const map<const int, const string> str_promode = {
        {PRO_PPP, "PPP"},
        {PRO_CLK, "CLK"},
        {PRO_RTK, "RTK"},
        {PRO_NRTK, "NRTK"},
        {PRO_RTORB, "RTORB"},
        {PRO_VISUAL, "VISUAL"},
        {PRO_EXTUPD_WL, "EXT-UPD WL"},
        {PRO_HMW_WL, "SMOOTH-HMW WL"},
        {PRO_EXTFCB_NL, "SMOOTH-UPD NL"},
        {PRO_VRS, "VRS"},
        {PRO_ESTIFPB, "IFPB_ESTIMATION"},
        {PRO_MODELZTD, "ZTD_MODEL"},
        {PRO_UPD2OSB, "UPD2OSB"},
        {PRO_UPDPOST, "UPDPOST"},
        {PRO_UPDVERIFY, "UPDVERIFY"},
        {PRO_TRIOSB, "TRIOSB"},
        {PRO_BACKIAR, "BACKIAR"},
        {PRO_PPPCENTER, "PPP_CENTER"},
        {PRO_NRTKATOM, "NRTK_ATOM"},
        {PRO_DSTEC, "DSTEC"},
        {PRO_RTKV, "RTKV"}};

    const int LEVEL_IO = 0;    //
    const int LEVEL_GNSS = 1;  //
    const int LEVEL_FRAME = 2; //
    const int LEVEL_ALGO = 3;  //
    const int LEVEL_MAX = 4;

    const int ToHandle = 0; //
    const int ToWait = 1;   //
    const int DataTOut = 2; //
    const int DataOutT = 3; //
    const int DataNEx = 4;  //
    const int DataErr = 5;  //
    const int DataCmpl = 6; //

    const int NetClose = -1; //
    const int NetCont = 0;   //
    const int NetTOut = 1;   //
    const int NetUAuth = 2;  //
    const int NetAuth = 3;   //

    /*** AMBIGUITY STATE ****/
    const int AMBTYPE_NONE = 0;
    const int AMBTYPE_EWL = 1;
    const int AMBTYPE_EWLCOMB = 2;
    const int AMBTYPE_WL = 3;
    const int AMBTYPE_IF = 4;
    const int AMBTYPE_RAW = 5;

    /*** observation state */
    const int DEL_NONE = 0;
    const int DEL_LACK_RAorPH = 1;
    const int DEL_LESS_4NUM = 2;
    const int DEL_LACK_IFCB = 3;
    const int DEL_MODEL_CHK = 4;
    const int DEL_BAD = 5;
    const int DEL_LACK_DCB = 6;
    const int DEL_LACK_OSB = 7;
    const int DEL_LACK_EPH = 8;
    const int DEL_LACK_BS = 9;
    const int DEL_CUTOFF = 10;

    /*** ambiguity state */
    const int AMB_SF_RESET = 1; /// srif-filter reset
    const int AMB_BIG_GAP = 2;  /// for RTK，rover is 01/ base is 11 /both is 21
    const int AMB_MW_GAP = 3;
    const int AMB_LG_GAP = 4;
    const int AMB_AFIF_GAP = 5;
    const int AMB_DIA_GAP = 6;
    const int AMB_DIA_FAKE = 7; /* dont save into files */
    const int AMB_EPC_GAP = 8;  /* dont save into files */

    const char str_obs_state[][16] = {"NONE", "DEL_LACK_RAorPH", "DEL_LESS_4NUM", "DEL_LACK_IFCB", "DEL_MODEL_CHK", "DEL_BAD", "DEL_LACK_DCB",
                                      "DEL_LACK_OSB", "DEL_LACK_EPH", "DEL_LACK_BS", "DEL_CUTOFF"};

    const char str_amb_state[][16] = {"NONE", "AMB_SF_RESET", "AMB_BIG_GAP", "AMB_M_W_GAP", "AMB_L_G_GAP", "AMB_AIF_GAP", "AMB_DIA_GAP", "AMB_DIA_FAKE", "AMB_EPC_GAP"};

    const int LEVEL_TRACE = LEVEL_IO;
    const int LEVEL_ERROR = LEVEL_MAX;

    const char str_tracetype[][16] = {"IO", "GNSS", "FRAME", "ALGO", "ERROR"};

    const int WL_AMB_FLOAT = 1; /* MW WL */
    const int WL_AMB_ION = 2;   /* resolved by ionosphere constraint */
    const int WL_AMB_MW = 3;    /* MW resolved, with enough span */
    const int WL_AMB_PH = 4;    /* AFIF WL */
    const int WL_AMB_PPP = 5;   /* PPP WL */

    const int AMB_HOLD = 8;   // HOLD
    const int AMB_LOOPC = 7;  // LOOP CHECKED
    const int AMB_SRCH = 6;   // SRCH through loop
    const int AMB_FIX = 5;    // N1-fixed
    const int AMB_WLFIX = 3;  // WL-fixed
    const int AMB_EWLFIX = 2; // ewl-fixed
    const int AMB_EST = 1;    // float state
    const int AMB_INIT = 0;   // initialize status

    const int SOL_NONE = 0;   // invalid
    const int SOL_SINGLE = 1; // single solution
    const int SOL_FLOAT = 2;  // float
    const int SOL_EWLFIX = 3; // ewl-fix
    const int SOL_WLFIX = 4;  // wl-fix
    const int SOL_FIX = 5;    // nl-fix

    const int MAXGPS_OBS = 12;
    const int MAXGAL_OBS = 12;
    const int MAXBDS_OBS = 30;
    const int MAXQZS_OBS = 4;
    const int MAXGLO_OBS = 10;

    const int TYPERA = 1;        /* range observation*/
    const int TYPEPH = 2;        /* phase observation */
    const int TYPERA_AFIF = 3;   /* AFIF range observation */
    const int TYPEPH_AFIF = 4;   /* AFIF phase observation */
    const int TYPECON = 5;       /* constraint observation*/
    const int INVALID_UPD = 999; /* invalid upd value */

#define _IF_0(f1, f2) ((f1 * f1) / (f1 * f1 - f2 * f2))
#define _IF_1(f1, f2) (-(f2 * f2) / (f1 * f1 - f2 * f2))
#define _WL_0(f1, f2) (f1 / (f1 - f2))
#define _WL_1(f1, f2) (-f2 / (f1 - f2))
#define _NL_0(f1, f2) (f1 / (f1 + f2))
#define _NL_1(f1, f2) (f2 / (f1 + f2))

#ifdef _WIN32
#define def_thread_t HANDLE
#define def_dev_t HANDLE
#define def_cond_t CONDITION_VARIABLE
#define def_lock_t CRITICAL_SECTION
#define def_initlock(f) InitializeCriticalSection(f)
#define def_lock(f) EnterCriticalSection(f)
#define def_unlock(f) LeaveCriticalSection(f)
#define def_destroylock(f) DeleteCriticalSection(f)

#define def_initcond(c) InitializeConditionVariable(c)
#define def_notify(c) WakeConditionVariable(c)
#define def_wait(c, f) SleepConditionVariableCS(c, f, INFINITE)
#define def_destroycond(c)

#define FILEPATHSEP '\\'
#define def_pthread_id_self() GetCurrentThreadId()
#else

#define def_thread_t pthread_t
#define def_dev_t int
#define def_cond_t pthread_cond_t
#define def_lock_t pthread_mutex_t
#define def_initlock(f) pthread_mutex_init(f, NULL)
#define def_lock(f) pthread_mutex_lock(f)
#define def_unlock(f) pthread_mutex_unlock(f)
#define def_destroylock(f) pthread_mutex_destroy(f)

#define def_initcond(c) pthread_cond_init(c, NULL)
#define def_notify(c) pthread_cond_signal(c)
#define def_wait(c, f) pthread_cond_wait(c, f)
#define def_destroycond(c) pthread_cond_destroy(c)

#define FILEPATHSEP '/'

#define def_pthread_id_self() pthread_self()
#endif

#define BDSTOINT(type, value) (type)(round(value))

#define BDSADDBITS(a, b)                                                               \
    {                                                                                  \
        bitbuffer = (bitbuffer << (a)) | (BDSTOINT(long long, b) & ((1ULL << a) - 1)); \
        numbits += (a);                                                                \
        while (numbits >= 8)                                                           \
        {                                                                              \
            buffer[size++] = bitbuffer >> (numbits - 8);                               \
            numbits -= 8;                                                              \
        }                                                                              \
    }
#define BDSADDBITSFLOAT(a, b, c)                      \
    {                                                 \
        long long i = BDSTOINT(long long, (b) / (c)); \
        BDSADDBITS(a, i)                              \
    };

    // map<string,map<char,list<string>> > info_amb_combination = {
    //         {
    //                 {"wl",
    //                         {"G", {"L1", "L2"}},
    //                         {"R", {"L1", "L2"}},
    //                         {"E", {"L1", "L2"}},
    //                         {"c", {"L1", "L2"}}
    //                 }
    //         }
    // }

#define bbo_if(isys, l1) pointer_charstr(bamboo::MAXFREQ, bamboo::LEN_FREQ, (char *)dly->freq_obs[isys], l1)
#define idx_AFIF(f, c, opt) (opt->nfreq[c] > 2 ? pointer_charstr(opt->nfreq[c], bamboo::LEN_FREQ, (char *)opt->freq[c], opt->freq_if[c][f]) : f)

    template <typename T>
    string toString(const T &t)
    {
        ostringstream oss; //
        oss << t;          //
        return oss.str();
    }
    class Signal
    {
    public:
        Signal()
        {
            def_initlock(&mutex_);
            def_initcond(&condition_);
        }
        ~Signal()
        {
            def_destroylock(&mutex_);
            def_destroycond(&condition_);
        }
        inline void m_lock()
        {
            def_lock(&mutex_);
        }
        inline void m_unlock()
        {
            def_unlock(&mutex_);
        }
        inline void m_notify()
        {
            def_notify(&condition_);
        }
        inline void m_wait()
        {
            def_wait(&condition_, &mutex_);
        }

    protected:
        def_lock_t mutex_;
        def_cond_t condition_;
    };
    /// functions
    template <typename T>
    inline void SAFEDEL(void *&target)
    {
        if (nullptr != target)
        {
            T *temp = static_cast<T *>(target);
            delete temp;
            temp = nullptr;
            target = nullptr;
        }
    }
    template <class T>
    inline T MAX(T a, T b)
    {
        return a > b ? a : b;
    }
    template <class T>
    inline T MIN(T a, T b)
    {
        return a > b ? b : a;
    }

    template <class T, class P>
    inline T SIGN(T a, P b)
    {
        return b > 0 ? a : -a;
    }
    template <class T>
    inline int NINT(T a)
    {
        return (int)(a + SIGN(1, a) * 0.5);
    }
    template <class T>
    inline int FLOOR(T a)
    {
        return fabs(a - ((int)a)) > 0.5 ? NINT(a) - SIGN(1, a) : NINT(a) + SIGN(1, a);
    }

    template <class T>
    inline T ABS(T a)
    {
        return a > 0 ? a : -a;
    }

    template <class T>
    inline T maxlist(list<T> &in)
    {
        T max = 0;
        for (auto p = begin(in); p != end(in); ++p)
        {
            if ((*p) > max)
                max = *p;
        }
        return max;
    }
    template <class T>
    inline void bbo_dellist(list<T> &in, T del)
    {
        for (auto p = begin(in); p != end(in);)
        {
            if (del == (*p))
            {
                p = in.erase(p);
                break;
            }
            ++p;
        }
    }

    template <class T>
    inline int bbo_index_vector(vector<T> &in, T check)
    {
        for (auto p = begin(in); p != end(in);)
        {
            if (check == (*p))
            {
                return p - begin(in);
            }
            ++p;
        }
        return -1;
    }

    template <class T>
    inline bool bbo_contains_list(list<T> &in, T check)
    {
        for (auto p = begin(in); p != end(in);)
        {
            if (check == (*p))
            {
                return true;
            }
            ++p;
        }
        return false;
    }
    template <class T>
    inline void bbo_delvector(vector<T> &in, T del)
    {
        for (auto p = begin(in); p != end(in);)
        {
            if (del == (*p))
            {
                p = in.erase(p);
                break;
            }
            ++p;
        }
    }
    template <class T>
    inline bool bbo_contains_vector(vector<T> &in, T check)
    {
        for (auto p = begin(in); p != end(in);)
        {
            if (check == (*p))
            {
                return true;
            }
            ++p;
        }
        return false;
    }
    template <class T>
    inline T sum(T *array, int ndim)
    {
        int i;
        T sum0 = 0;
        for (i = 0; i < ndim; i++)
        {
            sum0 += array[i];
        }
        return sum0;
    }
    template <class T>
    inline T maxarray(T *array, int ndim)
    {
        if (ndim <= 0)
            return 0;
        T max = array[0];
        for (int i = 1; i < ndim; ++i)
        {
            if (max < array[i])
            {
                max = array[i];
            }
        }
        return max;
    }

#define exit(f)                      \
    {                                \
        LOGPRINT_EX("exit by code"); \
        exit(f);                     \
    }

} // namespace bamboo
#endif /* INCLUDE_BAMBOO_CONST_H_ */
