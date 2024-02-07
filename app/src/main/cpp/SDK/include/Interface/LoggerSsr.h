//
// Created by k on 2024/2/4.
//

#ifndef FORTEST_LOGGERSSR_H
#define FORTEST_LOGGERSSR_H
class LoggerSsr {
public:
    string getLastSsrInfo(string);
    static LoggerSsr* s_getInstance();
    static map<string, string> pbais;
protected:
    LoggerSsr();
    string m_getOrbclkInfo();
    string m_getIfpbInfo();
    static LoggerSsr* sInstance;

    string m_getCodeBiasInfo();

    string m_getPBiasInfo();

    string m_getAllInfo();
};

#endif //FORTEST_LOGGERSSR_H
