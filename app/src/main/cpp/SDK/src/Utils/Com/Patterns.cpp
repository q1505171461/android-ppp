/*
 * PatternName.cpp
 *
 *  Created on: 2018年2月6日
 *      Author: juntao, at wuhan university
 */
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include "include/Utils/Com/Patterns.h"
#include "include/Utils/Com/Com.h"
#include "include/Controller/Const.h"
#include "include/Controller/Controller.h"
#include "include/Utils/Com/Logtrace.h"
using namespace std;
using namespace bamboo;
Patterns *Patterns::m_Instance = NULL;
Patterns::Patterns()
{
	memset(this->fform, 0, sizeof(this->fform));
	memset(this->fname, 0, sizeof(this->fname));
	this->MAXFILE = 100;
	Patterns::readFile();
}
Patterns::~Patterns()
{
}
void Patterns::readFile()
{
	string line;
	ifstream in;
	Deploy *dly = Controller::s_getInstance()->m_getConfigure();
	map<string, string>::iterator itr;
	/// gnss.cfg part
	itr = dly->mTable.find(string("predefine"));
	if (itr == dly->mTable.end())
	{
		LOGE("item = %s,can't access key itme", "predefine");
		cout << "ERROR(PatternName):can not access pndfln file!\n"
			 << endl;
		exit(1);
	}
	in.open((*itr).second.c_str(), ios::in);
	if (!in.is_open())
	{
		LOGE("file = %s,can't access file", (*itr).second.c_str());
		cout << "ERROR(s_getPatternName) can not access to " << (*itr).second << " file!" << endl;
		exit(1);
	}
	nfile = 0;
	while (getline(in, line))
	{
		sscanf(line.c_str(), "%s%s", fname[nfile], fform[nfile]);
		nfile++;
		if (nfile > MAXFILE)
		{
			LOGE("nfile = %d,too many files defined in panda_file_name", nfile);
			cout << "ERROR(get_file_name):too many files defined in panda_file_name!" << endl;
			exit(1);
		}
	}
	in.close();
}
void Patterns::m_getPatternName(int ldefined, const char *keyword, const char *param_list, int iyear,
								int imonth, int iday, int ihour, char *name)
{
	char varword[1024];
	char form[1024] = {'\0'};
	int npar, nword, mjd, week, wd, idoy;
	bool lfound;
	char word[40][256], param[40][256];
	int i, j, k, index;
	if (ldefined)
		strcpy(form, name);
	else
	{
		for (i = 0; i < nfile; i++)
		{
			if (!strcmp(fname[i], keyword))
			{
				strcpy(form, fform[i]);
				break;
			}
		}
		if (len_trim(form) == 0)
		{
			LOGE("keyword = %s,keyword not defined in panda_file_name", keyword);
			LOGPRINT("keyword = %s,keyword not defined in panda_file_name", keyword);
			exit(1);
		}
	}
	split_string(false, form, ' ', ' ', '-', &nword, (char *)word, 256);
	strcpy(varword, param_list);
	split_string(true, varword, ' ', ' ', ':', &npar, (char *)param, 256);
	mjd = md_julday(iyear, imonth, iday);
	gpsweek(iyear, imonth, iday, &week, &wd);
	mjd2doy(mjd, &iyear, &idoy);
	sprintf(param[npar], "Y=%1.1d", iyear % 10);
	sprintf(param[npar + 1], "YY=%2.2d", iyear % 100);
	sprintf(param[npar + 2], "YYYY=%4.4d", iyear);
	sprintf(param[npar + 3], "DDD=%3.3d", idoy);
	sprintf(param[npar + 4], "HH=%2.2d", ihour);
	sprintf(param[npar + 5], "WWWW=%4.4d", week);
	sprintf(param[npar + 6], "GPSW=%4.4d", week);
	sprintf(param[npar + 7], "WWWWD=%5.5d", week * 10 + wd);
	sprintf(param[npar + 8], "MJD=%5.5d", mjd);
	sprintf(param[npar + 9], "MM=%2.2d", imonth);
	npar += 10;
	memset(name, 0, strlen(name) * sizeof(char));
	for (i = 0; i < nword; i++)
	{
		if (len_trim(word[i]) == 0)
			continue;
		trim(word[i]);
		if (i % 2 != 0)
		{
			lfound = false;
			for (j = 0; j < npar; j++)
			{
				//if (strstr(param[j], word[i]) != NULL) {
				if (!strncmp(param[j], word[i], strlen(word[i])))
				{
					lfound = true;
					k = len_trim(name);
					index = index_string(param[j], '=');
					substringEx(varword, param[j], index + 1,
								strlen(param[j]) - index - 1);
					strcpy(name + k, varword);
					break;
				}
			}
			if (!lfound)
			{
				LOGE("word = %s,keyword = %s,word is not defined in keyword", word[i], keyword);
				LOGPRINT("word = %s,keyword = %s,word is not defined in keyword", word[i], keyword);
				exit(1);
			}
		}
		else
		{
			k = len_trim(name);
			strcpy(name + k, word[i]);
		}
	}
}
