/*
 * Taiutc.cpp
 *
 *  Created on: 2018年2月7日
 *      Author: juntao, at wuhan university
 */

#include <iostream>
#include <string>
#include <fstream>
#include "include/Utils/Com/Taiutc.h"
#include "include/Controller/Controller.h"
using namespace std;
using namespace bamboo;
Taiutc *Taiutc::m_Instance = NULL;
Taiutc::Taiutc()
{
	m_rdUtcFile();
	//Recovery::s_regRecall("leapsc",this);
}
Taiutc::~Taiutc()
{
	//Recovery::s_rmvRecall("leapsc",this);
}
void Taiutc::v_onRecall(string oprName)
{
	if (strstr(oprName.c_str(), "leapsc"))
		m_rdUtcFile();
}
void Taiutc::m_rdUtcFile()
{
	string line;
	ifstream in;
	int jd, leap;
	map<string, string>::iterator itr;
	/// gnss.cfg part
	Deploy *dly = Controller::s_getInstance()->m_getConfigure();
	itr = dly->mTable.find("leapsc");
	if (itr == dly->mTable.end())
	{
		LOGE("item = %s,file_table not exist key key", "leapsc");
		cout << "***ERROR(readCommFil):file_table not exist leapsc item "
			 << endl;
		exit(1);
	}
    char fname[1024] = {0};
    strcat(fname,dly->configPath);
    strcat(fname,(*itr).second.c_str());
	in.open(fname, ios::in);
	if (!in.is_open())
	{
		LOGE("file = %s,can't open file", (*itr).second.c_str());
		cout << "***ERROR(rdUtcFile):no such file" << (*itr).second << endl;
		exit(1);
	}
	this->ls.clear();
	this->jdt.clear();

	while (getline(in, line))
	{
		if (strstr(line.c_str(), "+leap sec"))
			break;
	}
	while (getline(in, line))
	{
		if (strstr(line.c_str(), "-leap sec"))
			break;
		sscanf(line.c_str(), "%d%d", &jd, &leap);
		this->ls.push_back(leap);
		this->jdt.push_back(jd);
	}
	in.close();
}
int Taiutc::m_getTaiutc(int mjd)
{
	int i;
	if (jdt.size() != 0)
	{
		if (mjd < jdt[0])
		{
			printf("mjd = %d,mjde = %d,epoch before table start", mjd, jdt[0]); // incase of loop
			LOGE("mjd = %d,mjde = %d,epoch before table start", mjd, jdt[0]);
			exit(1);
		}
		if (mjd > jdt[jdt.size() - 1])
		{
			printf("mjd = %d,mjde = %d,epoch after table start", mjd, jdt[jdt.size() - 1]);
			LOGE("mjd = %d,mjde = %d,epoch after table start", mjd, jdt[jdt.size() - 1]);
			exit(1);
		}
	}
	for (i = 0; i < jdt.size() - 1; i++)
		if (mjd > jdt[i] && mjd < jdt[i + 1])
			return this->ls[i];
	return 0;
}
