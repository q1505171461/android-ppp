/*
 * Taiutc.h
 *
 *  Created on: 2018/2/7
 *      Author: juntao, at wuhan university
 */

#ifndef INCLUDE_COM_TAIUTC_H_
#define INCLUDE_COM_TAIUTC_H_

#include <iostream>
#include <string>
#include <vector>
using namespace std;
namespace bamboo{
class Taiutc{
public:
	Taiutc();
	virtual ~Taiutc();
	virtual void v_onRecall(string);
	int m_getTaiutc(int mjd);
	static inline Taiutc* s_getInstance(){
		if(m_Instance == NULL){
			m_Instance = new Taiutc;
		}
		return m_Instance;
	}
	static inline void s_destroyInstance(){
		if(m_Instance != NULL)
			delete m_Instance;
		m_Instance = NULL;
	}
private:
	void m_rdUtcFile();
	vector<int> ls;
	vector<int> jdt;
	static Taiutc* m_Instance;
};
}
#endif /* INCLUDE_COM_TAIUTC_H_ */
