/*
 * Com.h
 *
 *  Created on: 2018/2/12
 *      Author: juntao, at wuhan university
 */

#ifndef INCLUDE_COM_COM_H_
#define INCLUDE_COM_COM_H_

#include <iostream>
#include <vector>
#include "include/Controller/Station.h"
#include "include/Controller/Satellite.h"
#include "include/Controller/Rnxobs.h"
#include "include/Adapter/Orbclk/BrdOrbitClkAdapter.h"
using namespace std;
namespace bamboo
{
	extern int md_julday(int, int, int);

	extern void fillobs(char *line, int nobs, int itemlen, double ver);

	extern void filleph(char *line, double ver);

	extern int pointer_string(int, string[], string);

	extern int pointer_charstr(int row, int col, const char *string_array, const char *string);

	extern int index_string(const char *src, char key);

	char *substringEx(char *dest, const char *src, int start, int length);

	extern char *left_justify_string(char *string);

	extern int len_trim(const char *pStr);

	extern char *trim(char *pStr);

	extern void xyzblh(double *x, double scale, double a0, double b0, double dx, double dy,
					   double dz, double *geod);

	void blhxyz(double *geod, double a0, double b0, double *x);

	extern void rot_enu2xyz(double lat, double lon, double (*rotmat)[3]);

	extern void m_wtclkhd(const char *filename, int mjd, double intv);

	extern void m_wtsp3hd(const char *filname, int mjd, double intv);

	extern void oceanload_coef(double lat, double lon, double (*olc)[11]);

	void brdtime(char *cprn, int *mjd, double *sod);

	extern void matmpy(double *A, double *B, double *C, int row, int colA, int colB);

	extern void transpose(double *in, double *out, int row, int col);

	extern void cross(double *v1, double *v2, double *vout);

	extern void unit_vector(int n, double *v, double *u, double *length);

	extern double dot(int n, double *v1, double *v2);

	extern double distance(int n, double *v1, double *v2);

	void xyz2rtn(double *xsat, double *R, double *T, double *N);

	extern void betau(double *xsat, double *xsun, double *beta, double *u);

	extern void phase_windup_itrs(int *lfirst, double (*rot_f2j)[3], double (*rot_l2f)[3],
								  double *xbf, double *ybf, double *zbf, double *xrec2sat, double *dphi0,
								  double *dphi);

	extern void phase_windup(int *lfirst, double (*rot_f2j)[3], double (*rot_l2f)[3],
							 double *xbf, double *ybf, double *zbf, double *xrec2sat, double *dphi0,
							 double *dphi);

	extern void split_string(bool lnoempty, char *string, char c_start, char c_end,
							 char seperator, int *nword, char *keys, int len);

	extern void gpsweek(int year, int month, int day, int *week, int *wd);

	extern void mjd2doy(int jd, int *iyear, int *idoy);

	extern void cur_time(int *mjd, double *sod);

	extern void timinc(int jd, double sec, double delt, int *jd1, double *sec1);

	extern char *runtime(char *buf);

	extern char *runlocaltime(char *buf);

	extern char *run_timefmt(int mjd, double sod, char *buf);

	extern char *run_timefmt(time_t tt, char *buf);

	extern void mjd2date(int jd, double sod, int *iyear, int *imonth, int *iday, int *ih,
						 int *imin, double *sec);

	extern void yeardoy2monthday(int iyear, int idoy, int *imonth, int *iday);

	extern bool isUpperLetter(unsigned char data);

	extern void findkey(ifstream &in, const char *keyword, const char *sinex_bracket, char *value);

	extern char *toLower(char *value);

	extern char *toUpper(char *value);

	int index_freq(char (*cfreq)[LEN_FREQ], int nfreq, const char *freq);

	extern void get_freq(Satellite *SAT, const char *type, int nfreq, char (*cfreq)[LEN_FREQ]);

	double freqbysys(int isys, int ifreq, const char *freq);

	extern double freqbytp(int isat, const char *freq);

	extern int read_siteinfo(Station *SIT);

	extern int read_siteinfo(string name, double *x);

	void wksow2mjd(int week, double sow, int *mjd, double *sod);

	void time2mjd(time_t time, int *mjd, double *sod);

	time_t mjd2time(int mjd, double sod);

	double mjd2wksow(int mjd, double sod, int *week, double *sow);

	void yr2year(int &yr);

	double timdif(int jd2, double sod2, int jd1, double sod1);

	int all(int *vx, int n, int value);

	int linit(int *ldat, char csys, int value);

	bool chos(int ndel, int imax, int *idel);

	int ambslv(int ncad, double *q22, double *bias, double *disall);

	int ambslv_(int ncad, double *q22, double *bias, double *bias_sec, double *disall);

	long double GetX(double RadLat, double eSquare, double a);

	bool bl2Gaussxy(double RadLat, double RadLon, double *x, double *y,
					double RadLon0, double eSquare, double a, double heightchange);

	unsigned long CRC24(long size, const unsigned char *buf);

	unsigned int getbdsiode(GPSEPH &bdseph);

	bool getTwoIndexOfValidObs(double obs[2 * MAXFREQ], int &i, int &j);

	bool validSatObs(const char *uobs, const char *cobs, double obs[2 * MAXFREQ], int ifreq);

	bool validSatObs(const char *uobs, const char *cobs, double (*obs)[2 * MAXFREQ], int isat, int ifreq);

	int genAode(char csys, int mjd, double sod, double toe, int inade, GPSEPH *eph);

	void bdsCodeCorbyElv(int Prn, double elvRad, double *codeCor);

	int getNoZeroCount(double *xl, int n);

	void excludeAnnoValue(char *value, const char *in);

	string zipJson(string json);

	void readlogfile(Station &sit, int mjd, double sod, double intv);

	void read_amblog(Station &sit, Station &refsit, int mjd, double sod, double intv);

	void read_amblog(Station &sit, int mjd, double sod, double intv);

	void read_amblog_new(Station &sit, Station &refsit, int mjd, double sod, double intv);

	void get_compile_date_base(uint8_t *Year, uint8_t *Month, uint8_t *Day);

	void read_amblog_new(Station &sit, int mjd, double sod, double intv);

	char *get_compile_date(char *g_date_buf);

	void cor_BDScodebias(int, double *, double);

	void bds_code_cor(char *ctype, int nfreq, char cfreq[MAXFREQ][LEN_FREQ],
					  double elev, double *bias);

	void CHI2(int N, double CH, double *P, double *D);

	void NORMAL(double U, double *P);

	void PNORMAL(double Q, double *U);

	void pchi2(int N, double Q, int L, double *XX);

	bool chitst(int flag, int ndof, double osig, double nsig, double zone);

	int pointer_station(vector<Station> &stalist, string staname);

	void filesync_unlock(def_dev_t lockfd, int locksize);

	int filesync_lock(const char *orbmutex, def_dev_t &lockfd, int &locksize);

	int filesync_lock_nosync(const char *orbmutex, def_dev_t &lockfd, int &locksize);

	int getIndexOfStation(vector<Station> &stalist, string name);

	char csys_bytype(const char *type);

	string get_wl_tag_forinput(char csys);

	bool check_sys(int bunionBDS, int isat, int isys);

	int index_sys(bool bunionBDS, int isat);

	string getcprn_bytype(string cprn, char csys);
	double stec2m(double stec, int isat, const char *L1);

	double m2stec(double delay, double fq);

	bool validConfig(const char *tag);

	bool bbo_hasobs(int isys, double *obs);

	int bbo_sel_avaobs(int isys, double *obs);

	string s_validCmd(map<string, map<string, string>> &, string, string);

	void sunmoonpos_eci(KPL_IO::gtime_t tut, double *rsun, double *rmoon);

} // namespace bamboo
#endif /* INCLUDE_COM_COM_H_ */
