#ifndef INCLUDE_DEPLOYH_
#define INCLUDE_DEPLOYH_
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include "include/Controller/Station.h"
#include "include/Controller/Satellite.h"
#include "include/Controller/Const.h"
#include "include/Utils/Json/json.h"
using namespace std;
namespace bamboo
{
	class Upd_ambconfig
	{
	public:
		Upd_ambconfig()
		{
			for (int i = 0; i < MAXSYS; ++i)
			{
				iq_comb[i][0] = iq_comb[i][1] = iq_comb[i][2] = -1;
				ic_comb[i][0] = ic_comb[i][1] = ic_comb[i][2] = 0;
			}
			b_sm = true;
			b_tryfix = true;
			bioncor = false;
			bupd_use = false;
			bfix_individual = false;
			bresolve = false;
			memset(lamdw, 0, sizeof(lamdw));
			memset(lamdn, 0, sizeof(lamdn));
			memset(g, 0, sizeof(g));
		}
		string tag;						  /* configures tag */
		int bioncor;					  /* mw using ion correction*/
		int b_sm;						  /* whether smooth mw */
		int b_tryfix;					  /* fixed if upper layer fixed */
		map<char, string> comb, comb_ion; /* mw coefficient */
		string outfile;					  /* mw output file */
		string updfile;					  /* mw upd file */
		char fixmode[1024];				  /* ambiguity fix mode: lambda/round or lambda + round  */
		char updtype[256];				  /* UPD file type? snx ? bias ? comb */
		char ambtype[256];				  /* ambiguity type mw or raw, if mw is true, the minelev/mincommtime_round will be overwrote by updsmooth.json */
		double minelev;					  /* minimum elevation */
		double mincommtime_lam;			  /* minimum common time */
		double mincommtime_round;		  /* minimum common time for round */
		double maxdev;					  /* round max deviation */
		double maxsig;					  /* round max sigma */
		double minalpha;				  /* round min alpha */
		double cutoff;					  /* ambiguity resolution cutoff */
		double success_rate;			  /* success rate for ambiguity resolution */
		bool bfix_individual;			  /* fix individual for each constellation */
		bool bupd_use;					  /* whether use upd correction */
		bool bresolve;					  /* whether resolve ambiguity */
		double holdcutoff;				  /* ambiguity hold cutoff (rad) */
		double holdtimelim;				  /* ambiguity hold time limit */
		double holdcutlim;				  /* max dev to hold */
		int iq_comb[MAXSYS][MAXFREQ];	  /* identifier for wide-lane/N1 ambiguity in RAW mode,for UPD/PPP_IAR resolution */
		int ic_comb[MAXSYS][MAXFREQ];	  /* coefficient of each frequency  */
		double lamdw[MAXSAT];			  /* lamdw for this combination */
		double lamdn[MAXSAT];			  /* lamdn for this combination */
		double g[MAXSYS];				  /* f1 / f2 for this combination */
	};
	class Deploy
	{
	public:
		Deploy();
		~Deploy();
		void m_readJsonFile(int argc, const char *args[]);
		char statfile[256];	 /* stat log file */
		char tracefile[256]; /* trace log file */
		/* Gnss Process Flag */
		bool lpost;		 /* flag whether in post process mode */
		bool lsavorbclk; /* flag whether save orbit and clock that program used */
		bool lfixslip;	 /* flag whether fix slip */
		bool liar;		 /* flag whether execute iar */
		bool litrs;		 /* flag whether in itrs, in default is true */
		bool lsav;		 /* flag whether save all stat */
		bool lobssav;	 /* flag whether save observation */
		bool ldia;		 /* flag whether dia detect slip by post residual */
		bool lhelp;		 /* flag whether print help usage*/
		bool ltrace;	 /* flag whether open trace */
		bool lbdtime;	 /* flag whether observation is in BDS time */
		bool lsnr;		 /* flag whether snr for format */
		bool ldebug;	 /* flag whether open debug mode */
		bool lsync;		 /* flag whether sync real-time observation from different sites */
		bool lobssmooth; /* flag whether smooth code observation */
		/* Gnss Configure */
		int mjd;									  /* iteration mjd time*/
		int mjd0;									  /* first mjd of the process epoch */
		int mjd1;									  /* end mjd of the process epoch */
		int orbclk_intv;							  /* interval for saving orbit and clock that program used */
		double sod;									  /* iteration sod time */
		double sod0;								  /* first sod of the process epoch */
		double sod1;								  /* end sod of the process epoch */
		double seslen;								  /* process total time in s */
		double dintv;								  /* process interval */
		double obsintv;								  /* reading interval for rinex file */
		double scal_dx;								  /* multiply a priori station coordinate noise */
		double scal_qx;								  /* multiply a priori station coordinate process noise */
		map<string, double> scal_obs;				  /* key: G:L1, value: coefficient of a priori observation noise*/
		string cprn[MAXSAT];						  /* prn list */
		string prn_alias[MAXSAT];					  /* prn_alis list */
		int nprn;									  /* total number of satellite */
		int nsys;									  /* total number of constellation */
		int maxthd;									  /* total number of threads */
		int promode;								  /* process mode */
		int iref;									  /* index of reference constellation in SYS */
		char system[MAXSYS];						  /* constellation used in program */
		char ztdmod[256];							  /* ztd estimate mode */
		char ztdpri[256];							  /* ztd priori corrction mode */
		char obsdir[256];							  /* observation directory */
		char qcdir[256];							  /* qc log directory */
		char ephdir[256];							  /* ephemeris directory */
		char outdir[256];							  /* output directory */
		char tracedir[256];							  /* trace file directory */
		char externaldir[256];						  /* external files directory (dcb,upd,orb,clk,...)*/
		char uobs[256];								  /* observation used in program, PHASE/CODE */
		char cobs[256];								  /* observation combination type, RAW?IF?AFIF?*/
		char refpoint[256];							  /* reference point of satellite orbit products */
		char yawmodel[256];							  /* yaw model, MODEL: inner function POSTPRES: outter file */
		char ifpbmode[256];							  /* ifpb correction mode, NONE or IFPB_COR */
		int nfreq_if[MAXSYS];						  /* if number of ionosphere-free frequency */
		char freq_if[MAXSYS][MAXFREQ][LEN_FREQ];	  /* freq that form ionosphere-free combination */
		int nfreq[MAXSYS];							  /* number of frequency that processed in program in each constellation */
		char freq[MAXSYS][MAXFREQ][LEN_FREQ];		  /* frequency that processed in program, frequency in decending order */
		int nfreq_obs[MAXSYS];						  /* number of frequqency that read in program in each constellation,  [nfreq; extended] */
		char freq_obs[MAXSYS][MAXFREQ][LEN_FREQ];	  /* frequency that processed in program. the content in index 1:nfreq is the same as freq [freq, extended]*/
		char freqcomb_clk[MAXSYS][MAXFREQ][LEN_FREQ]; /* datum combination of clock products, for osb identification, and L5 dcb parameter set  (OBSELETE, with no use)*/
		int nfreqcomb_clk[MAXSYS];					  /* number of clock datum for each constellation (OBSELETE, with no use) */
		char c1p1cortype[256];						  /* c1p1 correction type,support: hist/codp/snx/trimodel */
		char ionpri[256];							  /* ionosphere priori correction */
		char ionmod[256];							  /* ionosphere estimation mode */
		char ambmod[256];							  /* ambiguity estimation mode */
		char sta_dcbmod[256];						  /* station dcb estimation mode */
		char sta_isbmod[256];						  /* station isb estimation mode */
		char sta_icbmod[256];						  /* station L5 dcb estimation mode */
		map<char, vector<string>> l5dcb_freq;		  /* station L5 dcb identifier*/
		double dcb_std0;							  /* dcb priori noise (m) */
		double dcb_qstd0;							  /* dcb process noise sqrt(m)/s */
		double icb_std0;							  /* L5 dcb priori noise (m) */
		double icb_qstd0;							  /* L5 dcb process noise sqrt(m)/s*/
		double isb_std0;							  /* isb priori noise (m) */
		double isb_qstd0;							  /* isb process noise sqrt(m)/s*/
		char clktype[MAXORBEPHTYPE][256];			  /* clock products type */
		char orbtype[MAXORBEPHTYPE][256];			  /* orbit products type */
		int tracelevel;								  /* trace level */
		char jsonConfig[LEN_STRING];				  /* json configure */
		char tablesConfig[LEN_STRING];				  /* tables configure path */
		/* realtime configure */
		list<string> ephcmds;		 /* real-time configures command */
		list<string> oaccmds;		 /* real-time configures command */
		map<string, string> obscmds; /* real-time configures command */
		/* ratio table */
		string pf_rtable; /* ratio table */
		/* Qc Part */
		double lg;				   /* PH - CD, for SF mode */
		double lw;				   /* MW slip threshold (cycle) */
		double dtec;			   /* ionosphere residual threshold (m) */
		double gap;				   /* observation gap to open new slip */
		double maxr_bias;		   /* maximum range bias to remove observations */
		double diabatch_wlimit;	   /* threshold for dia */
		double diabatch_esig;	   /* threshold for dia */
		double diasearch_esig;	   /* threshold for dia */
		double diasearch_esigdone; /* threshold for dia */
		char diamode[256];		   /* threshold for dia */
		bool ldia_reset;		   /* whether reset ambiguity if slip identified */

		/* Ambiguity resolution Part */
		bool b_osbswitch;		  /* whether use osb to perform ambiguity resolution */
		Upd_ambconfig ewl_config; /* ambiguity resolution configure for ewl */
		Upd_ambconfig wl_config;  /* ambiguity resolution configure for wl */
		Upd_ambconfig nl_config;  /* ambiguity resolution configure for nl */
		Upd_ambconfig aux_config; /* ambiguity resolution configure for other combinations */

		map<string, Upd_ambconfig> mw_config; /* for mw observation */
		/* Gnss inner structure */
		vector<Satellite> SAT;		/* Satellite configure */
		map<string, string> mTable; /* table path */

		/* Debug configures */
		char m_addargs[1024];
        char configPath[1024];
		/* output gga url */
		string out_url; /* ppp url */
	protected:
		/* inner functions */
		void sortf(int nfreq_in[MAXSYS], char freq_in[MAXSYS][MAXFREQ][LEN_FREQ]);					   /* sort the frequency */
		void m_parseMWconfigure(Upd_ambconfig &, string, string);									   /* parse MW configure */
		void m_parseFreqValue(char *, map<char, string> &);											   /* parse frequency configure */
		void m_parseObsNoiseScal(char *, map<string, double> &);									   /* parse observation noise coefficient configure */
		void m_parseFrequency(char freq_in[MAXSYS][MAXFREQ][LEN_FREQ], int *nfreq_in, char *freq_seq); /* parse frequency configure */
		void m_parseTablesConfigures(const char *t_path);											   /* prase table configure */
		void m_parseAmbiguityConfigures(const char *t_path);										   /* parse ambiguity resolution */
		void m_parseAMBFrequency(Upd_ambconfig &, char *freq_seq);									   /* parse ambiguity combination */
		void m_parseAMBWavelength(Upd_ambconfig &);													   /* fill wavelength for each ambiguity configures */
		void m_parseSatelliteConfigures();															   /* parse satellite configure */
		void m_sortOrderofFreq();																	   /* sort the frequency order, in frequency descending order */
		void m_readJsonItem(Json::Value &root, const char *item);									   /* parse main.configure */
		void m_printUsage();																		   /* print usage */
		bool m_checkConfigure();																	   /* check configure */
	};
} // namespace bamboo
#endif /* INCLUDE_DEPLOYH_ */
