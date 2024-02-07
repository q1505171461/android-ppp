//
// Created by juntao on 2021/4/5.
//

#ifndef BAMBOO_CODEBIASADAPTER_H
#define BAMBOO_CODEBIASADAPTER_H
#include <map>
#include <mutex>
#include "include/Controller/Deploy.h"
using namespace std;
namespace bamboo
{
    class Rnxobs;
    class CodeBiasAdapter
    {
    public:
        void v_inputCodeBias(const char *cprn, const char *mode, double dval);

    public:
        using t_codebias = map<string, double>;
        CodeBiasAdapter();
        ~CodeBiasAdapter();
        bool m_inquireCodeBias(bool, const char *cprn, const char *code, double *val);
        void m_corrCodeBias(int mjd, double sod, Rnxobs *sit);
        int inquireDCB(int mjd, double sod, int isat, const char *obt1, const char *obt2, double &bias);
        inline map<string, t_codebias> m_getCodeBias() { return m_mem; }

    protected:
        std::mutex m_lock;
        map<string, t_codebias> m_mem; /* IF datum for L1/L2 B1/B3 E1/E5a L1/L2 */
    };
} // namespace bamboo
#endif // BAMBOO_CODEBIASADAPTER_H
