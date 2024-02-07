#ifndef CONFIG_UTILS_INCLUDE_H_
#define CONFIG_UTILS_INCLUDE_H_
#include "include/Controller/Const.h"
#include <map>
#include <vector>
#include <queue>
namespace bamboo
{
    /// smooth the observation here
    ///reference: 单频载波相位移动开窗平滑伪距算法及精度分析[J]
    // class ObsSmoother {
    // public:
    //     ObsSmoother();
    //     void m_inputObs(int mjd, double sod, int isat, int* flag, double* obs);
    // public:
    //     double sm_value[MAXFREQ];
    //     int window, ncur[MAXFREQ]; // unit in s
    //     map<time_t, double> ion_rate;
    //     using sat_obsdata = map<time_t, map<string, double> >;
    //     map<string, queue<sat_obsdata>> data_container; // key : freq value data
    // };
}
#endif