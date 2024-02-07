/*
 * Station.h
 *
 *  Created on: 2018/2/7
 *      Author: juntao, at wuhan university
 */
#ifndef INCLUDE_BAMBOO_STATION_H_
#define INCLUDE_BAMBOO_STATION_H_
#include <iostream>
#include <map>
#include <cstdio>
#include <string>
#include <queue>
#include "include/Controller/Const.h"
#include "include/Rtklib/rtklib.h"
using namespace std;
namespace bamboo
{
	/* for weighting observations */
	class t_cmc
	{
	public:
		int ncmc[2 * MAXFREQ]{};
		double cmc[2 * MAXFREQ]{};
		double cmc_mean[2 * MAXFREQ]{};
	};
	class t_cmc_queue
	{
	public:
		inline double m_obtain_sp(int iq)
		{
			return (sig[iq + MAXFREQ] != 0.0 && sig[MAXFREQ] != 0.0) ? sig[iq + MAXFREQ] / sig[MAXFREQ] : -1;
		}
		queue<double> q_cmc[2 * MAXFREQ];
		double q_cmc_mean[2 * MAXFREQ]{};
		double sig[2 * MAXFREQ]{};
	};
	class Ssat_t
	{
	public:
		Ssat_t()
		{
			ionp = 1.0;
			iode = -1;
			hmap = zmap = azim = elev = phwp = prephi = 0.0;
			memset(flag, 0, sizeof(flag));
			memset(obs, 0, sizeof(obs));
			memset(omc, 0, sizeof(omc));
			memset(dloudx, 0, sizeof(dloudx));
			memset(R, 0, sizeof(R));
			memset(obsstat, 0, sizeof(obsstat));
			ifirst = 1;
		}
		void m_reset_epoch()
		{
			memset(R, 0, sizeof(R));
			memset(dloudx, 0, sizeof(dloudx));
			memset(omc, 0, sizeof(omc));
			memset(ipp, 0, sizeof(ipp));
			memset(vsats, 0, sizeof(vsats));
			iode = -1;
			phwp = elev = azim = zmap = hmap = trpm = ionm = 0.0;
		}
		int flag[MAXFREQ];					/* amb flag for different obs type */
		int slip[MAXFREQ];					/* slip flag for each frequency on raw observation */
		double azim;						/* azim for satellite in rad */
		double elev;						/* elevation for satellite in rad */
		double dloudx[3];					/* line of sight */
		int ifirst;							/* flag whether to reset prephi */
		int iode;							/* iode for the continuous omc computation */
		double prephi;						/* previous for phase windup correction */
		double phwp;						/* current phase windup correction */
		double ipp[3];						/* current ipp b/l/zenith for satellite */
		double hmap;						/* dry map for troposphere */
		double zmap;						/* wet map for troposphere */
		double trpm;						/* troposphere model value, in (m) */
		double ionm;						/* ionosphere model value, in STEC */
		double ionp;						/* ionosphere model coefficient */
		double lifamb[MAXFREQ][2];			/* living time */
		double spndel[MAXFREQ][2];			/* delete time */
		double obs[MAXFREQ * 2];			/* observation for each satellite */
		double dop[MAXFREQ];				/* dop value for each satellite */
		double snr[MAXFREQ];				/* snr value for each satellite */
		char fob[MAXFREQ * 2][LEN_OBSTYPE]; /* observation type */
		double omc[MAXFREQ * 2];			/* omc value for each satellite */
		double R[MAXFREQ * 2];				/*std value for observations on each frequency */
		int vsats[MAXFREQ];					/* slip has been processed flag */
		int obsstat[MAXFREQ];				/* observation status, observation*/
		t_cmc t_Cmc;						/* CMC observation */
	};
	/* station configure */
	class Station
	{
	public:
		Station(string staname);
		~Station();
		string name;					/* name of this station */
		char skd[8];					/* process mode, S/K/D*/
		char pcv[8];					/* pcv tag*/
		char clk[8];					/* clk tag */
		char map[8];					/* map tag*/
		char obsfile[1024];				/* obsfile path */
		double x[3];					/* XYZ coordinate */
		double dx0[3];					/* priori noise of XYZ */
		double qx[3];					/* process noise of XYZ */
		double geod[3];					/* station blh */
		double rot_l2f[3][3];			/* conversion for local to ecef */
		double enu[MAXSYS][MAXFREQ][3]; /* PCO for antenna */
		double enu0[3];					/* antenna height e/n/u */
		string rectyp;					/* receiver type */
		string anttyp;					/* antenna type */
		double dztd0;					/* ztd initial noise */
		double qztd;					/* ztd process noise */
		double dgrd0;					/* ztd gradient noise */
		double qgrd;					/* ztd gradient process noise */
		double dion0;					/* ionosphere initial noise */
		double qion;					/* ionosphere process noise */
		double rclock[MAXSYS];			/* receiver clock */
		double dclk0[MAXSYS];			/* clock parameter priori noise */
		double qclk[MAXSYS];			/* clock parameter process noise */
		double sigr[MAXSYS];			/* initial weight for pseudo-range observation (m)*/
		double sigp[MAXSYS];			/* initial weight for carrier-phase observation (m) */
		double cutoff;					/* cutoff for the observation */
		double olc[6][11];				/* sea tide coefficient */
		double zdd;						/* zenith dry delay */
		double zwd;						/* zenith wet delay */
		double grd[2];					/* ztd gradient */
		double vdop[5];					/* current dop value */
		double refx[3];					/* reference coordinate for output */
		Ssat_t ob[MAXSAT];				/* satellite state */
		char logFile[128];				/* qc log file path */
		char sys2code[MAXSYS][MAXFREQ]; /* system to code */
		double intvlog;					/* qc log interval */
		int mjd;						/* current observation mjd */
		double sod;						/* current observation sod */
		int jdlog;						/* log mjd */
		double sodlog;					/* log sod */
		double *v;						/* residual */
		double *R;						/* weight */
		int (*idx)[4];					/* idx for each equation,isat,ifreq,type,iobs */
		int nb;							/* number of observation */
		t_cmc_queue tCmcQueue[MAXSYS];	/* CMC QUEUE*/
	};
} // namespace bamboo
#endif /* INCLUDE_BAMBOO_STATION_H_ */
