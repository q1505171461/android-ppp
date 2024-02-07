/*
 * utils.cpp
 *
 *  Created on: 2018/2/4
 *      Author: juntao, at wuhan university
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iomanip>
#include <string>
#include <map>
#include <fstream>
#include <sys/file.h>
#include "include/Utils/Com/Com.h"
#include "include/Utils/Com/Taiutc.h"
#include "include/Utils/Com/Logtrace.h"
#include "include/Rtklib/rtklib_fun.h"
#include "include/Controller/Controller.h"
using namespace std;
using namespace KPL_IO;
namespace bamboo
{
    int md_julday(int iyear, int imonth, int iday)
    {
        int iyr, result;
        int doy_of_month[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304,
                                334};
        if (iyear < 0 || imonth < 0 || iday < 0 || imonth > 12 || iday > 366 || (imonth != 0 && iday > 31))
        {
            LOGE("iyear = %d,imonth = %d,iday = %d,incorrect argument", iyear, imonth, iday);
            LOGPRINT("iyear = %d,imonth = %d,iday = %d,incorrect argument", iyear, imonth, iday);
            exit(1);
        }
        iyr = iyear;
        if (imonth <= 2)
            iyr -= 1;
        result = 365 * iyear - 678941 + iyr / 4 - iyr / 100 + iyr / 400 + iday;
        if (imonth != 0)
            result = result + doy_of_month[imonth - 1];
        return result;
    }
    void gpsweek(int year, int month, int day, int *week, int *wd)
    {
        int mjd;
        if (year != 0)
        {
            if (month != 0)
                mjd = md_julday(year, month, day);
            else
                mjd = md_julday(year, 1, 1) + day - 1;
        }
        else
            mjd = day;
        *week = (mjd - 44244) / 7;
        *wd = mjd - 44244 - (*week) * 7;
    }

    void mjd2doy(int jd, int *iyear, int *idoy)
    {
        *iyear = (jd + 678940) / 365;
        *idoy = jd - md_julday(*iyear, 1, 1);
        while (*idoy <= 0)
        {
            (*iyear)--;
            *idoy = jd - md_julday(*iyear, 1, 1) + 1;
        }
    }

    void cur_time(int *mjd, double *sod)
    {
        //	struct timeval tv;
        //    struct tm* ptr;
        //	gettimeofday(&tv,NULL);
        //	ptr = gmtime (&tv.tv_sec);
        //	*mjd = md_julday(ptr->tm_year + 1900,ptr->tm_mon + 1,ptr->tm_mday);
        //	*sod = ptr->tm_hour * 3600.0 + ptr->tm_min * 60.0 + ptr->tm_sec + tv.tv_usec / 1e6;
        //
        //	timinc(*mjd, *sod, Taiutc::s_getInstance()->m_getTaiutc(*mjd) - 19.0, mjd, sod);
        struct tm *ptr;
        time_t rawtime;
        time(&rawtime);
        ptr = gmtime(&rawtime);
        *mjd = md_julday(ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday);
        *sod = ptr->tm_hour * 3600.0 + ptr->tm_min * 60.0 + ptr->tm_sec;
        timinc(*mjd, *sod, Taiutc::s_getInstance()->m_getTaiutc(*mjd) - 19.0, mjd, sod);
    }
    void timinc(int jd, double sec, double delt, int *jd1, double *sec1)
    {
        *sec1 = sec + delt;
        int inc = (int)(*sec1 / 86400.0);
        *jd1 = jd + inc;
        *sec1 = *sec1 - inc * 86400.0;
        if (*sec1 >= 0)
            return;
        *jd1 = *jd1 - 1;
        *sec1 = *sec1 + 86400;
    }
    char *runtime(char *buf)
    {
        int iy, im, id, ih, imin, mjd;
        double dsec, sod;
        cur_time(&mjd, &sod);
        mjd2date(mjd, sod, &iy, &im, &id, &ih, &imin, &dsec);
        if (buf != NULL)
            sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d", iy, im, id, ih, imin, (int)dsec);
        return buf;
    }
    char *runlocaltime(char *buf)
    {
        int iy, im, id, ih, imin, mjd;
        double dsec, sod;
        struct tm *ptr;
        time_t rawtime;
        time(&rawtime);
        ptr = localtime(&rawtime);
        mjd = md_julday(ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday);
        sod = ptr->tm_hour * 3600.0 + ptr->tm_min * 60.0 + ptr->tm_sec;
        mjd2date(mjd, sod, &iy, &im, &id, &ih, &imin, &dsec);
        if (buf != NULL)
            sprintf(buf, "%02d-%02d-%02d %02d:%02d:%02d", iy, im, id, ih, imin,
                    (int)dsec);
        return buf;
    }
    char *run_timefmt(int mjd, double sod, char *buf)
    {
        int iy, im, id, ih, imin;
        double dsec;
        mjd2date(mjd, sod, &iy, &im, &id, &ih, &imin, &dsec);
        if (buf != NULL)
            sprintf(buf, "%02d-%02d-%02d/%02d:%02d:%02d", iy, im, id, ih, imin, (int)dsec);
        return buf;
    }
    char *run_timefmt(time_t tt, char *buf)
    {
        int iy, im, id, ih, imin, mjd;
        double dsec, sod;
        time2mjd(tt, &mjd, &sod);
        mjd2date(mjd, sod, &iy, &im, &id, &ih, &imin, &dsec);
        if (buf != NULL)
            sprintf(buf, "%02d-%02d-%02d/%02d:%02d:%02d", iy, im, id, ih, imin, (int)dsec);
        return buf;
    }
    void yeardoy2monthday(int iyear, int idoy, int *imonth, int *iday)
    {
        int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int id, i;
        if ((iyear % 4 == 0 && iyear % 100 != 0) || iyear % 400 == 0)
            days_in_month[1] = 29;
        id = idoy;
        for (i = 0; i < 12; i++)
        {
            id = id - days_in_month[i];
            if (id > 0)
                continue;
            *iday = id + days_in_month[i];
            *imonth = i + 1;
            break;
        }
    }
    double timdif(int jd2, double sod2, int jd1, double sod1)
    {
        return 86400.0 * (jd2 - jd1) + sod2 - sod1;
    }
    void yr2year(int &yr)
    {
        if (yr > 1900)
            return;
        if (yr <= 30)
            yr += 2000;
        else
            yr += 1900;
    }
    double mjd2wksow(int mjd, double sod, int *week, double *sow)
    {
        double wk, sw;
        wk = (int)((mjd + sod / 86400.0 - 44244.0) / 7.0);
        sw = (mjd - 44244.0 - wk * 7) * 86400.0 + sod;
        if (week)
            *week = wk;
        if (sow)
            *sow = sw;
        return sw;
    }
    time_t mjd2time(int mjd, double sod)
    {
        const int doy[] = {1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
        time_t time = 0;
        int days, sec, year, mon, day, hour, min;
        double dsec;
        mjd2date(mjd, sod, &year, &mon, &day, &hour, &min, &dsec);
        if (year < 1970 || 2099 < year || mon < 1 || 12 < mon)
            return time;

        /* leap year if year%4==0 in 1901-2099 */
        days = (year - 1970) * 365 + (year - 1969) / 4 + doy[mon - 1] + day - 2 + (year % 4 == 0 && mon >= 3 ? 1 : 0);
        sec = (int)floor(dsec);
        time = (time_t)days * 86400 + hour * 3600 + min * 60 + sec;
        return time;
    }
    void time2mjd(time_t time, int *mjd, double *sod)
    {
        double ep[6];
        const int mday[] = {/* # of days in a month */
                            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 31, 28, 31, 30, 31, 30, 31,
                            31, 30, 31, 30, 31, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
                            31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int days, sec, mon, day;

        /* leap year if year%4==0 in 1901-2099 */
        days = (int)(time / 86400);
        sec = (int)(time - (time_t)days * 86400);
        for (day = days % 1461, mon = 0; mon < 48; mon++)
        {
            if (day >= mday[mon])
                day -= mday[mon];
            else
                break;
        }
        ep[0] = 1970 + days / 1461 * 4 + mon / 12;
        ep[1] = mon % 12 + 1;
        ep[2] = day + 1;
        ep[3] = sec / 3600;
        ep[4] = sec % 3600 / 60;
        ep[5] = sec % 60;

        *mjd = md_julday((int)ep[0], (int)ep[1], (int)ep[2]);
        *sod = ep[3] * 3600.0 + ep[4] * 60.0 + ep[5];
    }
    void wksow2mjd(int week, double sow, int *mjd, double *sod)
    {
        if (mjd != NULL)
            *mjd = (int)(sow / 86400.0) + week * 7 + 44244;
        if (sod != NULL)
            *sod = fmod(sow, 86400.0);
    }
    void mjd2date(int jd, double sod, int *iyear, int *imonth, int *iday, int *ih,
                  int *imin, double *sec)
    {
        int doy = 0;
        mjd2doy(jd, iyear, &doy);
        yeardoy2monthday(*iyear, doy, imonth, iday);
        if (ih)
            *ih = static_cast<int>(sod / 3600.0);
        if (imin)
            *imin = static_cast<int>((sod - (*ih) * 3600.0) / 60.0);
        if (sec)
            *sec = sod - (*ih) * 3600.0 - (*imin) * 60.0;
    }
    void fillobs(char *line, int nobs, int itemlen, double ver)
    {
        int OFFSET = 0, len, i;
        char tmp[256];
        if (ver > 3.0)
            OFFSET = 3;
        len = len_trim(line);
        for (i = len; i < itemlen * nobs + OFFSET; i++)
            line[i] = ' ';
        line[itemlen * nobs + OFFSET] = '\0';
        for (i = 0; i < nobs; i++)
        {
            memset(tmp, 0, sizeof(char) * 256);
            substringEx(tmp, line + OFFSET + i * itemlen, 0, itemlen - 2); // last is signal strength
            if (len_trim(tmp) == 0)
            {
                line[itemlen * i + OFFSET + 1] = '0';
            }
        }
    }
    void filleph(char *line, double ver)
    {
        int i, len, cpre = 4;
        char tmp[128];
        if (ver < 3.0)
            cpre = 3;
        len = len_trim(line);
        for (i = len; i < cpre + 19 * 4; i++)
        {
            line[i] = ' ';
        }
        line[cpre + 19 * 4] = '\0';
        for (i = 0; i < 4; i++)
        {
            substringEx(tmp, line + cpre + 19 * i, 0, 19);
            if (len_trim(tmp) == 0)
            {
                line[cpre + 19 * i + 1] = '0';
            }
        }
    }
    int pointer_string(int cnt, string string_array[], string str)
    {
        int itr, idx = -1;
        for (itr = 0; itr < cnt; itr++)
        {
            if (string_array[itr] == str)
            {
                idx = itr;
                break;
            }
        }
        return idx;
    }
    /**********************CHANGED*****************************/
    int pointer_charstr(int row, int col, const char *string_array, const char *string)
    {
        int i;
        const char *pStr = (const char *)string_array;
        for (i = 0; i < row; i++)
        {
            if (strncmp(pStr + i * col, string, strlen(string)) == 0)
                break;
        }
        if (i == row)
            i = -1;
        return i;
    }
    int index_string(const char *src, char key)
    {
        int len = strlen(src);
        int i;
        for (i = 0; i < len; i++)
        {
            if (src[i] == key)
                break;
        }
        if (i == len)
            return -1;
        else
            return i;
    }
    // start: started with index of zero
    char *substringEx(char *dest, const char *src, int start, int length)
    {
        int i, j = 0;
        int len = strlen(src);
        if (start < 0 || start >= len || start + length > len)
        {
            dest[0] = '\0';
            return NULL;
        }
        for (i = start; i < start + length; i++)
        {
            dest[j] = src[i];
            j++;
        }
        dest[j] = '\0';
        return dest;
    }
    char *left_justify_string(char *string)
    {
        int p = 0;
        while (*(string + p) == ' ' || *(string + p) == '\n' || *(string + p) == '\r')
            p++;
        return string + p;
    }
    char *toUpper(char *value)
    {
        int len = strlen((const char *)value);
        int i;
        for (i = 0; i < len; i++)
        {
            if (value[i] >= 'a' && value[i] <= 'z')
                value[i] -= 32;
        }
        return value;
    }

    char *toLower(char *value)
    {
        int len = strlen((const char *)value);
        int i;
        for (i = 0; i < len; i++)
        {
            if (value[i] >= 'A' && value[i] <= 'Z')
                value[i] += 32;
        }
        return value;
    }
    bool isUpperLetter(unsigned char data)
    {
        return data >= 'A' && data <= 'Z';
    }
    void findkey(ifstream &in, const char *keyword, const char *sinex_bracket, char *value)
    {
        int index_eq;
        int index_exf, index_exc, index_ex;
        in.clear();
        in.seekg(0, ios::beg);
        string line;
        if (len_trim(sinex_bracket) != 0)
        {
            while (getline(in, line))
            {
                if (strlen(line.c_str()) >= strlen(sinex_bracket) && !strncmp(line.substr(1, strlen(sinex_bracket)).c_str(), sinex_bracket, strlen(sinex_bracket)) && line[0] == '+')
                    break;
            }
        }
        strcpy(value, "EMPTY");
        while (getline(in, line))
        {
            if (line[0] == '#' || line[0] == '*')
                continue;
            if (len_trim(sinex_bracket) != 0 && strlen(line.c_str()) >= strlen(sinex_bracket) && !strncmp(line.substr(1, strlen(sinex_bracket)).c_str(), sinex_bracket, strlen(sinex_bracket)) && line[0] == '-')
                return;
            if (strstr(line.c_str(), keyword) != NULL)
            {
                index_eq = line.find_first_of('=');
                index_exf = line.find_first_of('!');
                index_exc = line.find_first_of('#');
                index_ex = index_exf != -1 && index_exc != -1 ? MIN(index_exf, index_exc) : (index_exf != -1 ? index_exf : index_exc);
                if (index_ex != -1)
                {
                    substringEx(value, line.c_str(), index_eq + 1, index_ex - index_eq - 1);
                    return;
                }
                else
                {
                    substringEx(value, line.c_str(), index_eq + 1,
                                strlen(line.c_str()) - index_eq - 1);
                    return;
                }
            }
        }
    }
    char *trim(char *pStr)
    {
        int len = strlen(pStr);
        char *pIndex = pStr + len - 1;
        while (*pIndex == '\n' || *pIndex == '\r' || *pIndex == '\0' || *pIndex == ' ')
        {
            *pIndex = '\0';
            if (pStr == pIndex)
                break;
            pIndex--;
        }
        return pStr;
    }
    int len_trim(const char *pStr)
    {
        int length = strlen(pStr);
        int count = length;
        int i;
        for (i = length - 1; i >= 0; i--)
        {
            if (pStr[i] == '\0' || pStr[i] == '\n' || pStr[i] == '\r' || pStr[i] == ' ')
                count--;
            else
                break;
        }
        return count;
    }
    void split_string(bool lnoempty, char *string, char c_start, char c_end,
                      char seperator, int *nword, char *keys, int len)
    {
        int i0 = 0, i1, ilast, i;
        char varword[1024];
        char *word = (char *)keys;
        i1 = len_trim(string) - 1;
        if (c_start != ' ')
            i0 = index_string(string, c_start);
        if (c_end != ' ')
            i1 = index_string(string, c_end);
        *nword = 0;
        if (i1 == -1 || i0 > i1)
            return;

        ilast = i0;
        if (string[i1] != seperator)
            string[i1 + 1] = seperator;
        string[i1 + 2] = '\0';

        for (i = i0; i < i1 + 2; i++)
        {
            if (string[i] == seperator)
            {
                if (i - 1 >= ilast)
                {
                    substringEx(varword, string, ilast, i - ilast);
                    strcpy(word + (*nword) * len, varword);
                    (*nword)++;
                }
                else
                {
                    if (!lnoempty)
                    {
                        strcpy(word + (*nword) * len, "  ");
                        (*nword)++;
                    }
                }
                ilast = i + 1;
            }
        }
    }
    void blhxyz(double *geod, double a0, double b0, double *x)
    {
        double a, b, e2, N, W;
        if (a0 == 0 || b0 == 0)
        {
            a = 6378137.0;
            b = 298.257223563; // alpha = (a-b)/a
        }
        else
        {
            a = a0;
            b = b0;
        }
        if (b <= 6000000)
            b = a - a / b;
        e2 = (a * a - b * b) / (a * a);

        W = sqrt(1 - e2 * pow(sin(geod[0]), 2));
        N = a / W;
        x[0] = (N + geod[2]) * cos(geod[0]) * cos(geod[1]);
        x[1] = (N + geod[2]) * cos(geod[0]) * sin(geod[1]);
        x[2] = (N * (1 - e2) + geod[2]) * sin(geod[0]);
    }

    void xyzblh(double *x, double scale, double a0, double b0, double dx, double dy,
                double dz, double *geod)
    {
        double a, b;
        long double xp, yp, zp, s, e2, rhd, rbd, n, zps, tmp1, tmp2;
        int i;

        if (a0 == 0 || b0 == 0)
        {
            a = 6378137.0;
            b = 298.257223563; // alpha = (a-b)/a
        }
        else
        {
            a = a0;
            b = b0;
        }
        for (i = 0; i < 3; i++)
            geod[i] = 0;
        xp = x[0] * scale + dx;
        yp = x[1] * scale + dy;
        zp = x[2] * scale + dz;

        if (b <= 6000000)
            b = a - a / b;
        e2 = (a * a - b * b) / (a * a);
        s = sqrt(xp * xp + yp * yp);
        geod[1] = atan(yp / xp);
        if (geod[1] < 0)
        {
            if (yp > 0)
                geod[1] += PI;
            if (yp < 0)
                geod[1] += 2 * PI;
        }
        else
        {
            if (yp < 0)
                geod[1] += PI;
        }
        zps = zp / s;
        geod[2] = sqrt(xp * xp + yp * yp + zp * zp) - a;
        geod[0] = atan(zps / (1.0 - e2 * a / (a + geod[2])));
        n = 1;
        rhd = rbd = 1;
        while (rbd * n > 1e-4 || rhd > 1e-4)
        {
            n = a / sqrt(1.0 - e2 * sin(geod[0]) * sin(geod[0]));
            tmp1 = geod[0];
            tmp2 = geod[2];
            geod[2] = s / cos(geod[0]) - n;
            geod[0] = atan(zps / (1.0 - e2 * n / (n + geod[2])));
            rbd = ABS(tmp1 - geod[0]);
            rhd = ABS(tmp2 - geod[2]);
        }
    }
    void rot_enu2xyz(double lat, double lon, double (*rotmat)[3])
    {
        double coslat, sinlat, coslon, sinlon;
        coslat = cos(lat - PI / 2);
        sinlat = sin(lat - PI / 2);
        coslon = cos(-PI / 2 - lon);
        sinlon = sin(-PI / 2 - lon);

        rotmat[0][0] = coslon;
        rotmat[0][1] = sinlon * coslat;
        rotmat[0][2] = sinlon * sinlat;
        rotmat[1][0] = -sinlon;
        rotmat[1][1] = coslon * coslat;
        rotmat[1][2] = coslon * sinlat;
        rotmat[2][0] = 0;
        rotmat[2][1] = -sinlat;
        rotmat[2][2] = coslat;
    }
    void matmpy(double *A, double *B, double *C, int row, int colA, int colB)
    {
        int i, j, k;
        double value;
        double *dest = (double *)calloc(row * colB, sizeof(double));
        for (i = 0; i < row; i++)
        {
            for (j = 0; j < colB; j++)
            {
                value = 0;
                for (k = 0; k < colA; k++)
                {
                    value += A[i * colA + k] * B[k * colB + j];
                }
                dest[i * colB + j] = value;
            }
        }
        for (i = 0; i < row; i++)
        {
            for (j = 0; j < colB; j++)
            {
                C[i * colB + j] = dest[i * colB + j];
            }
        }
        free(dest);
    }
    void transpose(double *in, double *out, int row, int col)
    {
        double *tmp = (double *)calloc(row * col, sizeof(double));
        int i, j;
        for (i = 0; i < row; i++)
            for (j = 0; j < col; j++)
                tmp[j * row + i] = in[i * col + j];

        for (j = 0; j < col; j++)
            for (i = 0; i < row; i++)
                out[j * row + i] = tmp[j * row + i];
        free(tmp);
    }
    void cross(double *v1, double *v2, double *vout)
    {
        double tmp[3];
        tmp[0] = v1[1] * v2[2] - v1[2] * v2[1];
        tmp[1] = v1[2] * v2[0] - v1[0] * v2[2];
        tmp[2] = v1[0] * v2[1] - v1[1] * v2[0];

        vout[0] = tmp[0];
        vout[1] = tmp[1];
        vout[2] = tmp[2];
    }
    void unit_vector(int n, double *v, double *u, double *length)
    {
        int i;
        double len = 0.0;
        for (i = 0; i < n; i++)
        {
            len = len + v[i] * v[i];
        }
        len = sqrt(len);
        for (i = 0; i < n; i++)
            u[i] = v[i] / len;
        if (length)
            *length = len;
    }
    double dot(int n, double *v1, double *v2)
    {
        int i;
        double res;
        res = 0.0;
        for (i = 0; i < n; i++)
            res = res + v1[i] * v2[i];
        return res;
    }
    double distance(int n, double *v1, double *v2)
    {
        int i;
        double *dx = (double *)calloc(n, sizeof(double)), dist;
        for (i = 0; i < n; i++)
            dx[i] = v1[i] - v2[i];
        dist = sqrt(dot(n, dx, dx));
        free(dx);
        return dist;
    }
    void xyz2rtn(double *xsat, double *R, double *T, double *N)
    {
        double length;
        unit_vector(3, xsat, R, &length);
        unit_vector(3, xsat + 3, N, &length);
        cross(R, N, N);
        unit_vector(3, N, N, &length);
        cross(N, R, T);
        unit_vector(3, T, T, &length);
    }
    void betau(double *xsat, double *xsun, double *beta, double *u)
    {
        double xsat_unit[3], nop_unit[3];
        double xsun_unit[3], vsat_unit[3], det;
        unit_vector(3, xsat, xsat_unit, &det);
        cross(xsat, xsat + 3, nop_unit);
        unit_vector(3, nop_unit, nop_unit, &det);
        unit_vector(3, xsun, xsun_unit, &det);
        *beta = PI / 2.0 - acos(dot(3, nop_unit, xsun_unit));

        *u = acos(dot(3, xsat_unit, xsun_unit) / cos(*beta)); // https://wenku.baidu.com/view/e6bb285daf1ffc4ffe47acbc.html

        unit_vector(3, xsat + 3, vsat_unit, &det);

        if (dot(3, vsat_unit, xsun_unit) > dot(3, vsat_unit, xsat_unit) * dot(3, xsat_unit, xsun_unit))
            *u = -(*u);
        *u = *u - PI;

        while (*u < 0)
            *u = *u + 2 * PI;
        while (*u > 2 * PI)
            *u = *u - 2 * PI;
    }
    void phase_windup_itrs(int *lfirst, double (*rot_f2j)[3], double (*rot_l2f)[3],
                           double *xbf, double *ybf, double *zbf, double *xrec2sat, double *dphi0,
                           double *dphi)
    {
        // purpose  : phase wind-up correction ( the receiver and satellite antenna orientation dependent
        //            phase corrections). See Wu J.T., et al., Manuscripta Geogetica (1993) 18, pp91-98
        //           lfirst -- first call for this satellite-station pair
        //           rot_f2j -- rotation matrix from earth-fixed to inertial (J2000)
        //           rot_l2f -- rotation matrix from station system (enu right-hand system) to earth-fixed
        //           xbf,ybf,zbf -- unit vectors of spacecraft-fixed system (rotation matrix from body-fixed
        //                          to inertial)
        //           xrec2sat -- vector from rec. to satellite  for k-direction
        //           dphi0 -- initial dphi
        //           dphi  --  phase correction
        int j, n;
        double x_f[3], y_f[3], dummy[3], rlength, kusi, cosp, ph;
        double k[3], d_r[3], d_s[3];
        // right hand system
        double x_l[3] = {0.0, 1.0, 0.0};
        double y_l[3] = {-1.0, 0.0, 0.0};

        unit_vector(3, xrec2sat, k, &rlength);

        for (j = 0; j < 3; j++)
            k[j] = -k[j];

        /*equivalent antenna dipole for both receiver and satellite antenna
         D = x_j - k (k . x_j) - k x y_j
         k is the unit vector of signal transmitting direction

         transfer the unit vector of the antenna dipole unit in local/antenna system to
         inertial system.
         For ground receiver local   => earth fixed      => inertial
         For LEO    receiver antenna => spacecraft fixed => inertial*/

        matmpy((double *)rot_l2f, x_l, x_f, 3, 3, 1);
        matmpy((double *)rot_l2f, y_l, y_f, 3, 3, 1);
        // D = x_j - k(k . x_j) + k x y_j
        rlength = dot(3, k, x_f);
        cross(k, y_f, dummy);

        for (j = 0; j < 3; j++)
            d_r[j] = x_f[j] - rlength * k[j] + dummy[j];

        unit_vector(3, d_r, d_r, &rlength);

        // the same for the satellite antenna
        rlength = dot(3, k, xbf);
        cross(k, ybf, dummy);
        for (j = 0; j < 3; j++)
            d_s[j] = xbf[j] - rlength * k[j] - dummy[j];
        unit_vector(3, d_s, d_s, &rlength);

        cosp = dot(3, d_s, d_r);
        if (cosp < -1.0)
            cosp = -1.0; // here is different
        if (cosp > 1.0)
            cosp = 1.0;
        ph = acos(cosp) / 2.0 / PI;
        cross(d_s, d_r, dummy);
        kusi = dot(3, k, dummy);
        *dphi = SIGN(1.0, kusi) * ph;
        if (*lfirst)
        {
            *lfirst = false;
            n = 0;
        }
        else
            n = NINT(*dphi0 - *dphi);
        *dphi = n + *dphi;
        // save for the next epoch
        (*dphi0) = (*dphi);
    }

    void phase_windup(int *lfirst, double (*rot_f2j)[3], double (*rot_l2f)[3],
                      double *xbf, double *ybf, double *zbf, double *xrec2sat, double *dphi0,
                      double *dphi)
    {

        int j, n;
        double x_f[3], y_f[3], x_j[3], y_j[3], dummy[3], rlength, kusi;
        double k[3], d_r[3], d_s[3];

        double x_l[3] = {0.0, 1.0, 0.0};
        double y_l[3] = {-1.0, 0.0, 0.0};

        unit_vector(3, xrec2sat, k, &rlength);

        for (j = 0; j < 3; j++)
            k[j] = -k[j];

        /*equivalent antenna dipole for both receiver and satellite antenna
         D = x_j - k (k . x_j) - k x y_j
         k is the unit vector of signal transmitting direction

         transfer the unit vector of the antenna dipole unit in local/antenna system to
         inertial system.
         For ground receiver local   => earth fixed      => inertial
         For LEO    receiver antenna => spacecraft fixed => inertial*/
        matmpy((double *)rot_l2f, x_l, x_f, 3, 3, 1);
        matmpy((double *)rot_l2f, y_l, y_f, 3, 3, 1);
        matmpy((double *)rot_f2j, x_f, x_j, 3, 3, 1);
        matmpy((double *)rot_f2j, y_f, y_j, 3, 3, 1);

        // D = x_j - k(k . x_j) + k x y_j
        rlength = dot(3, k, x_j);
        cross(k, y_j, dummy);

        for (j = 0; j < 3; j++)
            d_r[j] = x_j[j] - rlength * k[j] + dummy[j];
        unit_vector(3, d_r, d_r, &rlength);

        // the same for the satellite antenna
        rlength = dot(3, k, xbf);
        cross(k, ybf, dummy);
        for (j = 0; j < 3; j++)
            d_s[j] = xbf[j] - rlength * k[j] - dummy[j];
        unit_vector(3, d_s, d_s, &rlength);

        cross(d_s, d_r, dummy);

        kusi = dot(3, k, dummy);
        *dphi = dot(3, d_s, d_r);

        if ((ABS(*dphi)) > 1.0)
            *dphi = NINT(*dphi) * 1.0;
        *dphi = SIGN(1.0, kusi) * acos(*dphi) / (2 * PI);

        if (*lfirst)
        {
            *lfirst = false;
            n = 0;
        }
        else
            n = NINT(*dphi0 - *dphi);
        *dphi = n + *dphi;

        // save for the next epoch
        (*dphi0) = (*dphi);
    }
    void getfreq(char csys, char *freq, int ifreq, double *val)
    {
        switch (csys)
        {
        case 'G':
            if (strstr(freq, "L1") != NULL)
                *val = GPS_L1;
            else if (strstr(freq, "L2") != NULL)
                *val = GPS_L2;
            else if (strstr(freq, "L5") != NULL)
                *val = GPS_L5;
            else
            {
                LOGE("freq = %s,unknow frequency for GPS", freq);
                LOGPRINT("freq = %s,unknow frequency for GPS", freq);
                exit(1);
            }
            break;
        case 'R':
            if (strstr(freq, "L1") != NULL)
                *val = GLS_L1 + ifreq * GLS_dL1;
            else if (strstr(freq, "L2") != NULL)
                *val = GLS_L2 + ifreq * GLS_dL2;
            else
            {
                LOGE("freq = %s,unknow frequency for GLONASS", freq);
                LOGPRINT("freq = %s,unknow frequency for GLONASS", freq);
                exit(1);
            }
            break;
        case 'E':
            if (strstr(freq, "L1") != NULL)
                *val = GAL_E1;
            else if (strstr(freq, "L8") != NULL)
                *val = GAL_E5;
            else if (strstr(freq, "L6") != NULL)
                *val = GAL_E6;
            else if (strstr(freq, "L5") != NULL)
                *val = GAL_E5a;
            else if (strstr(freq, "L7") != NULL)
                *val = GAL_E5b;
            else
            {
                LOGE("freq = %s,unknow frequency for GALILEO", freq);
                LOGPRINT("freq = %s,unknow frequency for GALILEO", freq);
                exit(1);
            }
            break;
        case 'C':
            if (strstr(freq, "L1") != NULL)
                *val = BDS_B1c;
            else if (strstr(freq, "L2") != NULL)
                *val = BDS_B1;
            else if (strstr(freq, "L7") != NULL)
                *val = BDS_B2b;
            else if (strstr(freq, "L6") != NULL)
                *val = BDS_B3;
            else if (strstr(freq, "L5") != NULL)
                *val = BDS_B2a;
            else if (strstr(freq, "L8") != NULL)
                *val = BDS_B2;
            else
            {
                LOGE("freq = %s,unknow frequency for BDS", freq);
                LOGPRINT("freq = %s,unknow frequency for BDS", freq);
                exit(1);
            }
            break;
        case 'J':
            if (strstr(freq, "L1") != NULL)
                *val = QZS_L1;
            else if (strstr(freq, "L2") != NULL)
                *val = QZS_L2;
            else if (strstr(freq, "L5") != NULL)
                *val = QZS_L5;
            else if (strstr(freq, "L6") != NULL)
                *val = QZS_LEX;
            else
            {
                LOGE("freq = %s,unknow frequency for QZSS", freq);
                LOGPRINT("freq = %s,unknow frequency for QZSS", freq);
                exit(1);
            }
            break;
        default:
            LOGE("freq = %s,unknow frequency", freq);
            LOGPRINT("freq = %s,unknow frequency", freq);
            exit(1);
        }
    }
    int index_freq(char (*cfreq)[LEN_FREQ], int nfreq, const char *freq)
    {
        int rfreq = -1;
        for (int ifreq = 0; ifreq < nfreq; ifreq++)
        {
            if (strstr(cfreq[ifreq], freq))
            {
                rfreq = ifreq;
                break;
            }
        }
        return rfreq;
    }
    void get_freq(Satellite *SAT, const char *type, int nfreq, char (*cfreq)[LEN_FREQ])
    {
        int i, j, iq, jq, isys;
        double tmp;
        char key[16];
        isys = index_string(SYS, SAT->prn_alias[0]);
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        if (nfreq == 0 || (nfreq == 1 && !strstr(type, "SF")))
        {
            LOGE("cprn = %s,nfreq = %d,frequency used for satellite is not selected!", SAT->cprn, nfreq);
            LOGPRINT("cprn = %s,nfreq = %d,frequency used for satellite is not selected!", SAT->cprn, nfreq);
            exit(1);
        }
        for (i = 0; i < nfreq; i++)
        {
            getfreq(SAT->cprn[0], cfreq[i], SAT->ifreq, SAT->freq + i);
            SAT->lamda[i] = VEL_LIGHT / SAT->freq[i];
            SAT->mlam[cfreq[i]] = SAT->lamda[i];
        }
        if (nfreq >= 2) // only update for the first two frequency
        {
            iq = idx_AFIF(0, isys, dly);
            jq = idx_AFIF(1, isys, dly);
            double g = SAT->freq[iq] / SAT->freq[jq];
            double g2 = g * g;
            // SAT->lamdw = VEL_LIGHT / fabs(SAT->freq[iq] - SAT->freq[jq]);
            // SAT->lamdn = VEL_LIGHT / fabs(SAT->freq[iq] + SAT->freq[jq]);
            SAT->fac[0] = g2 / (g2 - 1.0);
            SAT->fac[1] = 1.0 / (g2 - 1.0);
        }
    }
    double freqbysys(int isys, int ifreq, const char *freq)
    {
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        switch (SYS[isys])
        {
        case 'G':
            if (strstr(freq, "L1") != NULL)
                return GPS_L1;
            else if (strstr(freq, "L2") != NULL)
                return GPS_L2;
            else if (strstr(freq, "L5") != NULL)
                return GPS_L5;
            else
            {
                LOGPRINT("freq = %s,unknow frequency for GPS", freq);
                exit(1);
            }
            break;
        case 'B':
        case 'C':
            if (strstr(freq, "L1") != NULL)
                return BDS_B1c;
            else if (strstr(freq, "L2") != NULL)
                return BDS_B1;
            else if (strstr(freq, "L7") != NULL)
                return BDS_B2b;
            else if (strstr(freq, "L6") != NULL)
                return BDS_B3;
            else if (strstr(freq, "L5") != NULL)
                return BDS_B2a;
            else if (strstr(freq, "L8") != NULL)
                return BDS_B2;
            else
            {
                LOGPRINT("freq = %s,unknow frequency for BDS", freq);
                exit(1);
            }
            break;
        case 'R':
            if (strstr(freq, "L1") != NULL)
                return GLS_L1 + ifreq * GLS_dL1;
            else if (strstr(freq, "L2") != NULL)
                return GLS_L2 + ifreq * GLS_dL2;
            else
            {
                LOGPRINT("freq = %s,unknow frequency for GLONASS", freq);
                exit(1);
            }
            break;
        case 'E':
            if (strstr(freq, "L1") != NULL)
                return GAL_E1;
            else if (strstr(freq, "L8") != NULL)
                return GAL_E5;
            else if (strstr(freq, "L6") != NULL)
                return GAL_E6;
            else if (strstr(freq, "L5") != NULL)
                return GAL_E5a;
            else if (strstr(freq, "L7") != NULL)
                return GAL_E5b;
            else
            {
                LOGPRINT("freq = %s,unknow frequency for GALILEO", freq);
                exit(1);
            }
            break;
        case 'J':
            if (strstr(freq, "L1") != NULL)
                return QZS_L1;
            else if (strstr(freq, "L2") != NULL)
                return QZS_L2;
            else if (strstr(freq, "L5") != NULL)
                return QZS_L5;
            else if (strstr(freq, "L6") != NULL)
                return QZS_LEX;
            else
            {
                LOGPRINT("freq = %s,unknow frequency for QZSS", freq);
                exit(1);
            }
            break;
        default:
            LOGPRINT("type = %s,unknow observation type", freq);
            break;
        }
        return 0;
    }
    double freqbytp(int isat, const char *freq)
    {
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        int isys = index_string(SYS, dly->cprn[isat][0]);
        return freqbysys(isys, dly->SAT[isat].ifreq, freq);
    }
    void brdtime(char *cprn, int *mjd, double *sod)
    {
        switch (cprn[0])
        {
        case 'C':
        case 'B':
            timinc(*mjd, *sod, 14.0, mjd, sod);
            break;
        case 'R':
            timinc(*mjd, *sod, Taiutc::s_getInstance()->m_getTaiutc(*mjd) - 19.0, mjd, sod);
            break;
        }
    }
    void m_wtsp3hd(const char *filname, int mjd, double intv)
    {
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        int idoy, iyear, imon, iday, ih, imin, gweek, gd, nsat = dly->nprn, nl_set = 17, nline = ceil(1.0 * nsat / nl_set);
        double dsec, gsow;
        mjd2doy(mjd, &iyear, &idoy);
        mjd2date(mjd, 0, &iyear, &imon, &iday, &ih, &imin, &dsec);
        gpsweek(iyear, imon, iday, &gweek, &gd);
        mjd2wksow(mjd, 0, &gweek, &gsow);
        char line[] = {
            "%c M  cc GPS ccc cccc cccc cccc cccc ccccc ccccc ccccc ccccc\n"
            "%c cc cc ccc ccc cccc cccc cccc cccc ccccc ccccc ccccc ccccc\n"
            "%f  1.2500000  1.025000000  0.00000000000  0.000000000000000\n"
            "%f  0.0000000  0.000000000  0.00000000000  0.000000000000000\n"
            "%i    0    0    0    0      0      0      0      0         0\n"
            "%i    0    0    0    0      0      0      0      0         0\n"};
        Logtrace::s_defaultlogger.m_wtMsg("@%s #dP%04d%3d%3d%3d%3d%12.8lf      96   u+U IGS14 FIT  WHU\n", filname, iyear, imon,
                                          iday, ih, imin, dsec);
        Logtrace::s_defaultlogger.m_wtMsg("@%s ## %4d%16.8lf%15.8lf %d%16.8lf\n", filname, gweek, gsow, intv, mjd, 0.0);
        for (int iline = 0; iline < nline; ++iline)
        {
            if (iline == 0)
            {
                Logtrace::s_defaultlogger.m_wtMsg("@%s +  %03d   ", filname, nsat);
            }
            else
            {
                Logtrace::s_defaultlogger.m_wtMsg("@%s +%8s", filname, " ");
            }
            for (int isat = 0; isat < nl_set; ++isat)
            {
                if (iline * nl_set + isat < nsat)
                    Logtrace::s_defaultlogger.m_wtMsg("@%s %s", filname, dly->cprn[iline * nl_set + isat].c_str());
                else
                    Logtrace::s_defaultlogger.m_wtMsg("@%s %3d", filname, 0);
            }
            Logtrace::s_defaultlogger.m_wtMsg("@%s \n", filname);
        }
        for (int iline = 0; iline < nline; ++iline)
        {
            Logtrace::s_defaultlogger.m_wtMsg("@%s ++%7s", filname, " ");
            for (int isat = 0; isat < nl_set; ++isat)
            {
                if (iline * nl_set + isat < nsat)
                    Logtrace::s_defaultlogger.m_wtMsg("@%s %3d", filname, 10);
                else
                    Logtrace::s_defaultlogger.m_wtMsg("@%s %3d", filname, 0);
            }
            Logtrace::s_defaultlogger.m_wtMsg("@%s \n", filname);
        }
        Logtrace::s_defaultlogger.m_wtMsg("@%s %s", filname, line);
    }
    void m_wtclkhd(const char *filename, int mjd, double intv)
    {
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        int iyear, idoy, isat, i, nt;
        mjd2doy(mjd, &iyear, &idoy);
        Logtrace::s_defaultlogger.m_wtMsg("@%s      3.00           M                                       RINEX VERSION / TYPE\r\n", filename);
        Logtrace::s_defaultlogger.m_wtMsg("@%s     %2d.00                                                   INTERVAL            \r\n", filename, (int)intv);
        Logtrace::s_defaultlogger.m_wtMsg("@%s BAMBOO-INTCLK       WHU                 %04d%03d             PGM / RUN BY / DATE \r\n", filename, iyear, idoy);
        Logtrace::s_defaultlogger.m_wtMsg("@%s      1    AS                                                # / TYPES OF DATA   \r\n", filename);
        Logtrace::s_defaultlogger.m_wtMsg("@%s WHU WUHAN UNIVERSITY                                        ANALYSIS CENTER     \r\n", filename);
        Logtrace::s_defaultlogger.m_wtMsg("@%s    %03d                                                      # OF SOLN SATS      \r\n", filename, dly->nprn);
        for (nt = 0, isat = 0; nt < (int)ceil(dly->nprn / 15.0); nt++)
        {
            for (i = 0; i < 15; i++)
            {
                if (nt * 15 + i < dly->nprn)
                {
                    Logtrace::s_defaultlogger.m_wtMsg("@%s %3s ", filename, dly->cprn[isat++].c_str());
                }
                else
                {
                    Logtrace::s_defaultlogger.m_wtMsg("@%s     ", filename);
                }
            }
            Logtrace::s_defaultlogger.m_wtMsg("@%s PRN LIST            \r\n", filename);
        }
        Logtrace::s_defaultlogger.m_wtMsg("@%s                                                             END OF HEADER       \r\n", filename);
    }
    void oceanload_coef(double lat, double lon, double (*olc)[11])
    {
        int i, j, lfind;
        string line;
        double tol_d, tol_lat, tol_lon;
        double lon_deg, lat_deg, dlon, dlat;
        ifstream in;
        map<string, string>::iterator itr;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        itr = dly->mTable.find(string("ocload"));
        if (itr == dly->mTable.end())
        {
            LOGE("item = %s,file_table not exist key item", "ocload");
            LOGPRINT("item = %s,file_table not exist key item", "ocload");
            exit(1);
        }
        in.open((*itr).second.c_str(), ios::in);
        if (!in.is_open())
        {
            LOGE("file = %s,open configure file", (*itr).second.c_str());
            LOGPRINT("file = %s,open configure file", (*itr).second.c_str());
            exit(1);
        }
        tol_d = 1e4;
        tol_lat = tol_d / E_MAJAXIS;
        tol_lon = tol_lat / cos(lat);
        memset(olc, 0, sizeof(double) * 11 * 6);
        lfind = 0;
        while (getline(in, line))
        {
            if (strstr(line.c_str(), "lon/lat:") != NULL)
            {
                sscanf(line.c_str(), "%*50c%lf%lf", &lon_deg, &lat_deg);
                if (lon_deg < 0)
                    lon_deg += 360;
                dlon = ABS(lon_deg * DEG2RAD - lon);
                dlat = ABS(lat_deg * DEG2RAD - lat);
                if (dlat < tol_lat && dlon < tol_lon)
                {
                    for (i = 0; i < 6; i++)
                    {
                        getline(in, line);
                        sscanf(line.c_str(), "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", olc[i],
                               olc[i] + 1, olc[i] + 2, olc[i] + 3, olc[i] + 4,
                               olc[i] + 5, olc[i] + 6, olc[i] + 7, olc[i] + 8,
                               olc[i] + 9, olc[i] + 10);
                        if (i >= 3)
                        {
                            for (j = 0; j < 11; j++)
                                olc[i][j] *= DEG2RAD;
                        }
                    }
                    lfind = 1;
                }
            }
        }
        if (!lfind)
        {
            LOGI(LEVEL_GNSS, "no oceanload coefficients");
            LOGPRINT("no oceanload coefficients");
        }
        in.close();
        return;
    }

    int all(int *vx, int n, int value)
    {
        int i = 0, ret;
        ret = 1;
        for (i = 0; i < n; i++)
        {
            if (vx[i] != value)
            {
                ret = 0;
                break;
            }
        }
        return ret;
    }
    int linit(int *ldat, char csys, int value)
    {
        int isat, ret;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        ret = 1;
        for (isat = 0; isat < dly->nprn; isat++)
        {
            // if (dly->cprn[isat][0] != csys)
            if (dly->prn_alias[isat][0] != csys)
                continue;
            if (ldat[isat] != value)
            {
                ret = 0;
                break;
            }
        }
        return ret;
    }
    bool chos(int ndel, int imax, int *idel)
    {
        int i, ic;
        if (imax < ndel)
            return false;
        if (idel[0] == -1)
        {
            for (i = 0; i < ndel; i++)
                idel[i] = i;
            return true;
        }
        else
        {
            ic = ndel;
            while (ic > 0)
            {
                idel[ic - 1] = idel[ic - 1] + 1;
                if (idel[ic - 1] >= imax)
                {
                    ic--;
                    continue;
                }
                i = ic + 1;
                while (i <= ndel)
                {
                    idel[i - 1] = idel[i - 2] + 1;
                    if (idel[i - 1] >= imax)
                    {
                        ic--;
                        break;
                    }
                    i++;
                }
                if (i > ndel)
                    break;
            }
            if (ic <= 0)
                return false;
            else
                return true;
        }
        return false;
    }
    int ambslv(int ncad, double *q22, double *bias, double *disall)
    {
        /**********************************************
         * parameter:
         * 	input:  ncad          -- # of candidate ambiguities
         * 		    q22           -- cofactor matrix
         * 	output: bias          -- float / fixed ambiguity estimates
         *          disall        -- norm of optimum & suboptimum solutions
         */
        double *F = (double *)calloc(2 * ncad, sizeof(double));
        double *s = (double *)calloc(2, sizeof(double));
        double dump;
        int info = 0;
        if (ncad > 1)
        {
            if (!(info = lambda(ncad, 2, bias, q22, F, s)))
            {
                if (s[0] < s[1])
                {
                    memcpy(bias, F, sizeof(double) * ncad);
                    disall[0] = s[0];
                    disall[1] = s[1];
                }
                else
                {
                    memcpy(bias, F + ncad, sizeof(double) * ncad);
                    disall[0] = s[1];
                    disall[1] = s[0];
                }
            }
        }
        else
        {
            dump = bias[0];
            bias[0] = NINT(bias[0]) * 1.0;
            dump = bias[0] - dump;
            disall[0] = dump * dump / q22[0];
            dump = 1.0 - fabs(dump);
            disall[1] = dump * dump / q22[0];
        }
        free(F);
        free(s);
        return info;
    }

    //    int ambslv(int ncad, double* q22, double* bias, double* disall) {
    //        /**********************************************
    //         * parameter:
    //         * 	input:  ncad          -- # of candidate ambiguities
    //         * 		    q22           -- cofactor matrix
    //         * 	output: bias          -- float / fixed ambiguity estimates
    //         *          disall        -- norm of optimum & suboptimum solutions
    //         */
    //        double* F = (double*) calloc(2 * ncad, sizeof(double));
    //        double* s = (double*) calloc(2, sizeof(double));
    //        double dump;
    //        int info  = 0;
    //        if (ncad > 1) {
    //            LAMBDA(bias,q22,ncad,1,0,F);
    //            disall[1] = 4.1;
    //            disall[0] = 1.0;
    //            return 0;
    //        }
    //    }
    int ambslv_(int ncad, double *q22, double *bias, double *bias_sec, double *disall)
    {
        /**********************************************
         * parameter:
         * 	input:  ncad          -- # of candidate ambiguities
         * 		    q22           -- cofactor matrix
         * 	output: bias          -- float / fixed ambiguity estimates
         *          disall        -- norm of optimum & suboptimum solutions
         */
        double *F = (double *)calloc(2 * ncad, sizeof(double));
        double *s = (double *)calloc(2, sizeof(double));
        double dump, f;
        int info = 0;
        if (ncad > 1)
        {
            if (!(info = lambda(ncad, 2, bias, q22, F, s)))
            {
                if (s[0] < s[1])
                {
                    memcpy(bias, F, sizeof(double) * ncad);
                    if (bias_sec != NULL)
                        memcpy(bias_sec, F + ncad, sizeof(double) * ncad);
                    disall[0] = s[0];
                    disall[1] = s[1];
                }
                else
                {
                    memcpy(bias, F + ncad, sizeof(double) * ncad);
                    if (bias_sec != NULL)
                        memcpy(bias_sec, F, sizeof(double) * ncad);
                    disall[0] = s[1];
                    disall[1] = s[0];
                }
            }
        }
        else
        {
            f = dump = bias[0];
            bias[0] = NINT(bias[0]) * 1.0;
            dump = bias[0] - dump;
            disall[0] = dump * dump / q22[0];

            dump = 1.0 - fabs(dump);
            disall[1] = dump * dump / q22[0];
            if (bias_sec != NULL)
                bias_sec[0] = fabs(bias[0] + 1.0 - f) < fabs(bias[0] - 1.0 - f) ? bias[0] + 1.0 : bias[0] - 1.0;
        }
        free(F);
        free(s);
        return info;
    }
    int read_siteinfo(string name, double *x)
    {
        int lfound;
        string line;
        map<string, string>::iterator itr;
        ifstream in;
        /// gnss.cfg part
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        itr = dly->mTable.find(string("stacoo"));
        if (itr == dly->mTable.end())
        {
            LOGPRINT("item = %s,file_table not exist key item", "stacoo");
            LOGE("item = %s,file_table not exist key item", "stacoo");
            return 0;
        }
        in.open((*itr).second.c_str(), ios::in);
        if (!in.is_open())
        {
            LOGPRINT("file = %s,can't access site file", (*itr).second.c_str());
            LOGE("file = %s,can't access site file", (*itr).second.c_str());
            return 0;
        }

        lfound = 0;
        while (getline(in, line))
        {
            if (strstr(line.c_str(), (name + "_POS").c_str()))
            {
                lfound = 1;
                break;
            }
        }
        if (!lfound)
        {
            LOGPRINT("staname = %s,file = %s,station is not exist in the site file", name.c_str(), (*itr).second.c_str());
            LOGE("staname = %s,file = %s,station is not exist in the site file", name.c_str(), (*itr).second.c_str());
            in.close();
            return 0;
        }
        sscanf(line.c_str(), "%*s%lf%lf%lf", x, x + 1, x + 2);
        in.close();
        if (x[0] == 0 || x[1] == 0 || x[2] == 0)
        {
            LOGPRINT("staname = %s,file = %s,no coordinate information for station in site file", name.c_str(),
                     (*itr).second.c_str());
            LOGE("staname = %s,file = %s,no coordinate information for station in site file", name.c_str(),
                 (*itr).second.c_str());
        }
        return 1;
    }
    int read_siteinfo(Station *SIT)
    {
        int lfound;
        string line;
        map<string, string>::iterator itr;
        ifstream in;
        /// gnss.cfg part
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        itr = dly->mTable.find(string("stacoo"));
        if (itr == dly->mTable.end())
        {
            LOGPRINT("item = %s,file_table not exist key item", "stacoo");
            LOGE("item = %s,file_table not exist key item", "stacoo");
            return 0;
        }
        in.open((*itr).second.c_str(), ios::in);
        if (!in.is_open())
        {
            LOGPRINT("file = %s,can't access site file", (*itr).second.c_str());
            LOGE("file = %s,can't access site file", (*itr).second.c_str());
            return 0;
        }
        memset(SIT->x, 0, sizeof(double) * 3);
        memset(SIT->qx, 0, sizeof(double) * 3);
        memset(SIT->dx0, 0, sizeof(double) * 3);
        lfound = 0;
        while (getline(in, line))
        {
            if (strstr(line.c_str(), (SIT->name + "_POS").c_str()))
            {
                lfound = 1;
                break;
            }
        }
        if (!lfound)
        {
            LOGPRINT("staname = %s,file = %s,station is not exist in the site file", SIT->name.c_str(), (*itr).second.c_str());
            LOGE("staname = %s,file = %s,station is not exist in the site file", SIT->name.c_str(), (*itr).second.c_str());
            in.close();
            return 0;
        }
        sscanf(line.c_str(), "%*s%lf%lf%lf", SIT->x, SIT->x + 1, SIT->x + 2);
        getline(in, line);
        if (strstr(line.c_str(), (SIT->name + "_SIG").c_str()))
        {
            sscanf(line.c_str(), "%*s%lf%lf%lf%*54c%lf%lf%lf", SIT->dx0, SIT->dx0 + 1,
                   SIT->dx0 + 2, SIT->qx, SIT->qx + 1, SIT->qx + 2);
        }
        else
        {
            LOGPRINT("staname = %s,file = %s,site file format error", SIT->name.c_str(), (*itr).second.c_str());
            LOGE("staname = %s,file = %s,site file format error", SIT->name.c_str(), (*itr).second.c_str());
            in.close();
            return 0;
        }
        getline(in, line);
        if (strstr(line.c_str(), (SIT->name + "_ENU").c_str()))
        {
            sscanf(line.c_str(), "%*s%lf%lf%lf", SIT->enu0, SIT->enu0 + 1, SIT->enu0 + 2);
            SIT->rectyp = line.substr(58, 20);
            SIT->anttyp = line.substr(85, 20);
            if (!strstr(SIT->anttyp.c_str(), "NONE") && len_trim(SIT->anttyp.c_str()) < 13)
                SIT->anttyp = SIT->anttyp.substr(0, 16) + "NONE";
        }
        else
        {
            LOGPRINT("staname = %s,file = %s,site file format error", SIT->name.c_str(), (*itr).second.c_str());
            LOGE("staname = %s,file = %s,site file format error", SIT->name.c_str(), (*itr).second.c_str());
            in.close();
            return 0;
        }
        in.close();
        if (SIT->x[0] == 0 || SIT->x[1] == 0 || SIT->x[2] == 0)
        {
            LOGPRINT("staname = %s,file = %s,no coordinate information for station in site file", SIT->name.c_str(),
                     (*itr).second.c_str());
            LOGE("staname = %s,file = %s,no coordinate information for station in site file", SIT->name.c_str(),
                 (*itr).second.c_str());
        }
        return 1;
    }

    long double GetX(double RadLat, double eSquare, double a)
    {
        long double X;
        long double A0;
        long double A2;
        long double A4;
        long double A6;
        long double A8; ///*  from hsq
        A0 = 1 + 3.0 / 4.0 * eSquare + 45.0 / 64.0 * eSquare * eSquare + 350.0 / 512.0 * eSquare * eSquare * eSquare + 11025.0 / 16384.0 * eSquare * eSquare * eSquare * eSquare;
        A2 =
            -1.0 / 2.0 * (3.0 / 4.0 * eSquare + 60.0 / 64.0 * eSquare * eSquare + 525.0 / 512.0 * eSquare * eSquare * eSquare + 17640.0 / 16384.0 * eSquare * eSquare * eSquare * eSquare);
        A4 = 1.0 / 4.0 * (15.0 / 64.0 * eSquare * eSquare + 210.0 / 512.0 * eSquare * eSquare * eSquare + 8820.0 / 16384.0 * eSquare * eSquare * eSquare * eSquare);
        A6 = -1.0 / 6.0 * (35.0 / 512.0 * eSquare * eSquare * eSquare + 2520.0 / 16384.0 * eSquare * eSquare * eSquare * eSquare);
        A8 = 1.0 / 8.0 * 315.0 / 16384.0 * eSquare * eSquare * eSquare * eSquare;
        X = a * (1 - eSquare) * (A0 * RadLat + A2 * sin(2 * RadLat) + A4 * sin(4 * RadLat) + A6 * sin(6 * RadLat) + A8 * sin(8 * RadLat));
        return X;
    }
    bool bl2Gaussxy(double RadLat, double RadLon, double *x, double *y,
                    double RadLon0, double eSquare, double a, double heightchange)
    {
        long double m0, t, etaSquare, RadDeltaLon;
        long double N, X0;
        long double ePrimeSquare;
        long double aa, da, db; // ykoky2k
        RadDeltaLon = RadLon - RadLon0;
        ePrimeSquare = eSquare / (1 - eSquare);
        N = (a / sqrt(1 - eSquare * sin(RadLat) * sin(RadLat))) + heightchange;
        aa = N * sqrt(1 - eSquare * sin(RadLat) * sin(RadLat));
        da = aa - a;
        db = eSquare * sin(RadLat) * cos(RadLat) * (1 - eSquare * sin(RadLat) * cos(RadLat)) * da / (a * (1 - eSquare));
        RadLat = RadLat + db;
        a = aa;
        m0 = RadDeltaLon * cos(RadLat);
        t = tan(RadLat);
        etaSquare = ePrimeSquare * cos(RadLat) * cos(RadLat);
        X0 = GetX(RadLat, eSquare, aa);
        *x = X0 + 1 / 2.0 * N * t * pow(m0, 2) + 1 / 24.0 * (5.0 - pow(t, 2) + 9 * etaSquare + 4 * pow(etaSquare, 2)) * N * t * pow(m0, 4) + 1 / 720.0 * (61.0 - 58 * pow(t, 2) + pow(t, 4) + 270 * etaSquare - 330 * etaSquare * pow(t, 2)) * N * t * pow(m0, 6) + 1 / 40320 * N * t * pow(m0, 8) * (1385 - 3111 * pow(t, 2) + 543 * pow(t, 4) - pow(t, 6));
        *y = 500000.0 + N * m0 + 1 / 6.0 * (1.0 - pow(t, 2) + etaSquare) * N * pow(m0, 3) + 1 / 120.0 * (5.0 - 18.0 * pow(t, 2) + pow(t, 4) + 14.0 * etaSquare - 58.0 * pow(t, 2) * etaSquare) * N * pow(m0, 5) + 1 / 5040.0 * pow(m0, 7) * (61 - 479 * pow(t, 2) + 179 * pow(t, 4) - pow(t, 6));
        return true;
    }
    unsigned long CRC24(long size, const unsigned char *buf)
    {
        unsigned long crc = 0;
        int ii;
        while (size--)
        {
            crc ^= (*buf++) << (16);
            for (ii = 0; ii < 8; ii++)
            {
                crc <<= 1;
                if (crc & 0x1000000)
                {
                    crc ^= 0x01864cfb;
                }
            }
        }
        return crc;
    }
    bool getTwoIndexOfValidObs(double obs[2 * MAXFREQ], int &i, int &j)
    {
        i = j = -1;
        for (i = 0; i < MAXFREQ; i++)
        {
            if (obs[MAXFREQ + i] == 0.0)
                continue;
            for (j = i + 1; j < MAXFREQ; j++)
            {
                if (obs[MAXFREQ + j] == 0.0)
                    continue;
                break;
            }
            if (j != MAXFREQ)
                break;
        }
        if (i == MAXFREQ || j == MAXFREQ)
        {
            i = j = -1;
            return false;
        }
        return true;
    }
    bool validSatObs(const char *uobs, const char *cobs, double (*obs)[2 * MAXFREQ], int isat, int ifreq)
    {
        int id1, id2, isys;
        bool bvalid = false;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        if (strstr(cobs, "RAW") || strstr(cobs, "SF"))
        {
            if ((strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC")) && strstr(uobs, "CODE"))
            {
                if (obs[isat][ifreq] != 0.0 && obs[isat][MAXFREQ + ifreq] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC"))
            {
                if (obs[isat][ifreq] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "CODE"))
            {
                if (obs[isat][MAXFREQ + ifreq] != 0.0)
                    bvalid = true;
            }
        }
        else if (strstr(cobs, "IF"))
        {

            isys = index_string(SYS, dly->prn_alias[isat][0]);

            id1 = idx_AFIF(0, isys, dly); /// index of ionosphere-free combination
            id2 = idx_AFIF(1, isys, dly); /// index of ionosphere-free combination

            if ((strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC")) && strstr(uobs, "CODE"))
            {
                if (obs[isat][id1] != 0.0 && obs[isat][MAXFREQ + id1] != 0.0 &&
                    obs[isat][id2] != 0.0 && obs[isat][MAXFREQ + id2] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC"))
            {
                if (obs[isat][id1] != 0.0 && obs[isat][id2] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "CODE"))
            {
                if (obs[isat][MAXFREQ + id1] != 0.0 && obs[isat][MAXFREQ + id2] != 0.0)
                    bvalid = true;
            }
        }
        return bvalid;
    }

    bool validSatObs(const char *uobs, const char *cobs, double obs[2 * MAXFREQ], int ifreq)
    {
        int id1, id2;
        bool bvalid = false;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        if (strstr(cobs, "RAW") || strstr(cobs, "SF"))
        {
            if ((strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC")) && strstr(uobs, "CODE"))
            {
                if (obs[ifreq] != 0.0 && obs[MAXFREQ + ifreq] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC"))
            {
                if (obs[ifreq] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "CODE"))
            {
                if (obs[MAXFREQ + ifreq] != 0.0)
                    bvalid = true;
            }
        }
        else if (strstr(cobs, "IF"))
        {
            id1 = 0;
            id2 = 1;
            if ((strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC")) && strstr(uobs, "CODE"))
            {
                if (obs[id1] != 0.0 && obs[MAXFREQ + id1] != 0.0 &&
                    obs[id2] != 0.0 && obs[MAXFREQ + id2] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "PHASE") || strstr(uobs, "GRAPHIC"))
            {
                if (obs[id1] != 0.0 && obs[id2] != 0.0)
                    bvalid = true;
            }
            else if (strstr(uobs, "CODE"))
            {
                if (obs[MAXFREQ + id1] != 0.0 && obs[MAXFREQ + id2] != 0.0)
                    bvalid = true;
            }
        }
        return bvalid;
    }
    unsigned int getbdsiode(GPSEPH &bdseph)
    {
        unsigned char buffer[80];
        int size = 0;
        int numbits = 0;
        long long bitbuffer = 0;
        unsigned char *startbuffer = buffer;

        BDSADDBITSFLOAT(14, bdseph.idot, M_PI / (double)(1 << 30) / (double)(1 << 13));
        BDSADDBITSFLOAT(11, bdseph.a2,
                        1.0 / (double)(1 << 30) / (double)(1 << 30) / (double)(1 << 6));
        BDSADDBITSFLOAT(22, bdseph.a1,
                        1.0 / (double)(1 << 30) / (double)(1 << 20));
        BDSADDBITSFLOAT(24, bdseph.a0, 1.0 / (double)(1 << 30) / (double)(1 << 3));
        BDSADDBITSFLOAT(18, bdseph.crs, 1.0 / (double)(1 << 6));
        BDSADDBITSFLOAT(16, bdseph.dn, M_PI / (double)(1 << 30) / (double)(1 << 13));
        BDSADDBITSFLOAT(32, bdseph.m0, M_PI / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(18, bdseph.cuc,
                        1.0 / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(32, bdseph.e, 1.0 / (double)(1 << 30) / (double)(1 << 3));
        BDSADDBITSFLOAT(18, bdseph.cus,
                        1.0 / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(32, bdseph.roota, 1.0 / (double)(1 << 19));
        BDSADDBITSFLOAT(18, bdseph.cic,
                        1.0 / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(32, bdseph.omega0, M_PI / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(18, bdseph.cis,
                        1.0 / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(32, bdseph.i0, M_PI / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(18, bdseph.crc, 1.0 / (double)(1 << 6));
        BDSADDBITSFLOAT(32, bdseph.omega, M_PI / (double)(1 << 30) / (double)(1 << 1));
        BDSADDBITSFLOAT(24, bdseph.omegadot, M_PI / (double)(1 << 30) / (double)(1 << 13));
        BDSADDBITS(5, 0); // the last byte is filled by 0-bits to obtain a length of an integer multiple of 8

        return CRC24(size, startbuffer);
    }
    int genAode(char csys, int mjd, double sod, double toe, int inade, GPSEPH *eph)
    {
        int mjd_r, ret = -1;
        double sod_r;
        if (csys == 'G' || csys == 'J')
            ret = inade;
        else if (csys == 'R')
        {
            timinc(mjd, sod, 19.0 - Taiutc::s_getInstance()->m_getTaiutc(mjd), &mjd_r, &sod_r);
            //		ret = NINT(sod / 900.0) + 1;
            // IGMAS:
            ret = NINT(fmod(sod_r + 10800.0, 86400.0) / 900.0);
        }
        else if (csys == 'E')
        {
            // ret = NINT(sod / 600.0) + 1;
            // IGMAS:
            ret = inade;
        }
        else if (csys == 'C')
        {
            ret = NINT(fmod(toe, 86400.0) / 1800.0) + 1; // zhbd
            //		 ret = NINT(sod / 1800.0) + 1;
            // IGMAS:
            // ret = NINT(fmod(toe, 86400.0) / 450.0); // geng laoshi
            //		ret = getbdsiode(*eph);
            //        ret = (int)eph->iodc;
        }
        return ret;
    }
    void bdsCodeCorbyElv(int Prn, double elvRad, double *codeCor)
    {
        int i = 0, j = 0;
        static double coef[14][3][3] = {
            {{1.0211, -0.6213, 0.2708}, {1.2526, -0.0896, -0.1790}, {1.3200, -1.1985, 0.5211}},
            {{2.0465, -2.2363, 0.9862}, {0.5262, 0.2255, -0.1576}, {-0.2278, 1.3401, -0.6411}},
            {{1.2494, -1.0582, 0.5139}, {1.1609, -0.7834, 0.2694}, {1.4482, -1.2945, 0.3900}},
            {{1.1879, -0.9541, 0.3953}, {0.1531, 0.8377, -0.4189}, {0.9770, -0.9244, 0.3965}},
            {{2.1465, -2.4363, 0.9862}, {0.5294, -0.0964, -0.0198}, {0.2678, -0.2956, 0.2237}},
            {{1.5271, -1.4497, 0.6116}, {0.1531, 0.8377, -0.4087}, {0.2678, -0.2956, 0.2237}},
            {{2.1465, -2.4363, 0.9862}, {0.5294, -0.0964, -0.0198}, {0.3711, 0.2564, -0.1754}},
            {{0.2859, -0.1965, 0.1490}, {-0.1152, 0.5383, -0.1248}, {-0.2278, 1.3401, -0.6411}},
            {{1.2494, -1.0582, 0.5139}, {1.2526, -0.0896, -0.1790}, {0.9096, -1.5152, 0.8835}},
            {{1.0211, -0.6213, 0.2708}, {1.1609, -0.7834, 0.2768}, {1.4482, -1.2945, 0.3900}},
            {{1.1870, -1.0146, 0.7256}, {0.0837, 0.4875, -0.0303}, {0.1539, 0.1525, 0.0870}},
            {{0.7912, 0.2191, 0.0450}, {0.3797, 0.4546, -0.1035}, {-0.6531, 1.3267, -0.4818}},
            {{0.2770, 0.6165, -0.0564}, {0.2694, 0.8452, -0.3600}, {0.0440, 0.3500, -0.0032}},
            {{-0.0950, 0.9728, -0.0345}, {-0.2918, 1.1718, -003134}, {0.1824, -0.2680, 0.2526}},
        };
        for (i = 0; i < 3; i++)
        {
            codeCor[i] = 0;
            codeCor[i] = coef[Prn - 1][i][0] * elvRad + coef[Prn - 1][i][1] * elvRad * elvRad + coef[Prn - 1][i][2] * elvRad * elvRad * elvRad;
        }
    }
    int getNoZeroCount(double *xl, int n)
    {
        int it, ret = 0;
        for (it = 0; it < n; it++)
            if (xl[it] != 0.0)
                ret++;
        return ret;
    }
    void excludeAnnoValue(char *value, const char *in)
    {
        int num = 0, lstart = false;
        const char *ptr = in;
        while (*ptr != '\0')
        {
            if (lstart == false && (*ptr == ' ' || *ptr == '\n' || *ptr == '\t'))
            {
                ++ptr;
                continue;
            }
            lstart = true;
            if (*ptr != '#' && *ptr != '!')
            {
                value[num++] = *ptr;
                ++ptr;
            }
            else
                break;
        }
        value[num++] = '\0';
        trim(value);
    }
    string zipJson(string json)
    {
        string zipStr = "";
        int isize = 0, jsize = json.size();
        char *zipPtr = new char[jsize];
        const char *ptr = json.c_str();
        memset(zipPtr, 0, sizeof(char) * jsize);
        while (ptr - json.c_str() < json.size())
        {
            if (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')
            {
                ++ptr;
                continue;
            }
            zipPtr[isize++] = *ptr;
            ++ptr;
        }
        zipStr = string(zipPtr);
        delete[] zipPtr;
        return zipStr;
    }
    void cor_BDScodebias(int isat, double *omc, double elev)
    {
        int isys, iq;
        double rcor[MAXFREQ] = {0};
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        isys = index_string(SYS, dly->prn_alias[isat][0]);
        if (dly->cprn[isat][0] == 'C' || atoi(dly->cprn[isat].c_str() + 1) < 19)
        {
            bds_code_cor(dly->SAT[isat].type, dly->nfreq_obs[isys], dly->freq_obs[isys], elev * R2D, rcor);
            for (iq = 0; iq < dly->nfreq_obs[isys]; ++iq)
            {
                if (omc[MAXFREQ + iq] != 0.0 && rcor[iq] != 0.0)
                {
                    // LOGPRINT_EX("[%d %9.1lf] %s %13.3lf(origin) %13.3lf(cor) %13.3lf(now)", dly->mjd, dly->sod, dly->cprn[isat].c_str(), omc[MAXFREQ + iq], rcor[iq],
                    //             omc[MAXFREQ + iq] + rcor[iq]);
                    omc[MAXFREQ + iq] = omc[MAXFREQ + iq] + rcor[iq];
                }
            }
        }
    }
    void bds_code_cor(char *ctype, int nfreq, char cfreq[MAXFREQ][LEN_FREQ],
                      double elev, double *bias)
    {
        int isat, i, idx, ifreq[3] = {0};
        double alpha;
        static double coef[2][3][10] = {-0.11, -0.11, -0.09, -0.07, -0.02, -0.08,
                                        0.13, 0.24, 0.25, 0.33, -0.06, -0.11, -0.07, -0.05, -0.01, 0.06,
                                        0.11, 0.19, 0.26, 0.28, -0.27, -0.23, -0.21, -0.15, -0.11, -0.04,
                                        0.05, 0.14, 0.19, 0.32, -0.13, -0.20, -0.17, -0.12, -0.04, 0.10,
                                        0.32, 0.61, 0.82, 0.92, -0.12, -0.16, -0.12, -0.09, -0.01, 0.10,
                                        0.25, 0.42, 0.56, 0.64, -0.22, -0.15, -0.13, -0.10, -0.04, 0.05,
                                        0.14, 0.27, 0.36, 0.47};
        if (nfreq > 3)
        {
            LOGE("the number of frequency is more than 3");
            printf(
                "***ERROR(bds_code_cor):the number of frequency is more than 3!\n");
            exit(1);
        }
        if (strstr(ctype, "BEIDOU-2I"))
        {
            isat = 0;
        }
        else if (strstr(ctype, "BEIDOU-2M"))
        {
            isat = 1;
        }
        else
        {
            return;
        }
        for (i = 0; i < nfreq; i++)
        {
            if (strstr(cfreq[i], "L2"))
                ifreq[i] = 0;
            if (strstr(cfreq[i], "L7"))
                ifreq[i] = 1;
            if (strstr(cfreq[i], "L6"))
                ifreq[i] = 2;
        }
        if (elev <= 0.0)
        {
            for (i = 0; i < nfreq; i++)
                bias[i] = coef[isat][ifreq[i]][0];
        }
        else if (elev >= 90.0)
        {
            for (i = 0; i < nfreq; i++)
                bias[i] = coef[isat][ifreq[i]][9];
        }
        else
        {
            idx = (int)(elev / 10.0);
            for (i = 0; i < nfreq; i++)
            {
                alpha = (coef[isat][ifreq[i]][idx + 1] - coef[isat][ifreq[i]][idx]) / 10.0;
                bias[i] = alpha * (elev - idx * 10.0) + coef[isat][ifreq[i]][idx];
            }
        }
    }
    int pointer_station(vector<Station> &stalist, string staname)
    {
        vector<Station>::iterator staItr;
        int ret = -1, isit;
        for (staItr = stalist.begin(), isit = 0; staItr != stalist.end(); ++staItr, ++isit)
        {
            if ((*staItr).name == staname)
            {
                ret = isit;
                break;
            }
        }
        return ret;
    }

    void get_compile_date_base(uint8_t *Year, uint8_t *Month, uint8_t *Day)
    {
        const char *pMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        const char Date[12] = __DATE__; // 取编译时间
        uint8_t i;
        for (i = 0; i < 12; i++)
            if (memcmp(Date, pMonth[i], 3) == 0)
                *Month = i + 1, i = 12;
        *Year = (uint8_t)atoi(Date + 9); // Date[9]为２位年份，Date[7]为完整年份
        *Day = (uint8_t)atoi(Date + 4);
    }
    char *get_compile_date(char *g_date_buf)
    {
        uint8_t Year, Month, Day;
        get_compile_date_base(&Year, &Month, &Day);            // 取编译时间
        sprintf(g_date_buf, "%02d%02d%02d", Year, Month, Day); // 任意格式化
        return g_date_buf;
    }
    /*
     * purpose:chi-square test
     * parameter:
     * 		input:flag -- 0:dual-side test, H0 nsig^2==osig^2  H1 nsig^2!=osig^2
     * 					  1:right side test,H0 nsig^2<=osig^2  H1 nsig^2 > osig^2
     * 					 -1:left side test, H0 nsig^2>=osig^2  H1 nsig^2 < osig^2
     * 			  ndof -- degree of freedom
     * 			  osig -- reference sigma
     * 			  nsig -- new sigma
     * 			  zone -- belifef zone 0.95 0.99
     * 	     output:true then accept H0,flase then reject H0,accept H1
     *
     */
    bool chitst(int flag, int ndof, double osig, double nsig, double zone)
    {
        double stat, afa, chi2l, chi2r;
        bool result = false;
        stat = ndof * pow(nsig / osig, 2);
        if (!flag)
        {
            afa = (1.0 - zone) / 2.0;
            pchi2(ndof, afa, 1, &chi2r);
            afa = 1 - afa;
            pchi2(ndof, afa, 1, &chi2l);
            if (stat > chi2l && stat < chi2r)
                result = true;
        }
        else if (flag > 0)
        {
            afa = 1 - zone;
            pchi2(ndof, afa, 1, &chi2r);
            if (stat < chi2r)
                result = true;
        }
        else
        {
            afa = zone;
            pchi2(ndof, afa, 1, &chi2l);
            if (stat > chi2l)
                result = true;
        }
        return result;
    }
    void pchi2(int N, double Q, int L, double *XX)
    {
        double X, P, W, X0, K, PP, D;
        if (Q <= 0)
        {
            printf("ERROR(pchi2):INPUT AF=%lf <=0\n", Q);
            exit(1);
        }
        if (N == 1)
        {
            PNORMAL(Q / 2.0, &X);
            *XX = X * X;
            return;
        }
        if (N == 2)
        {
            *XX = -2.0 * log(Q);
            return;
        }
        P = 1.0 - Q;
        PNORMAL(Q, &X);
        W = 2.0 / (9.0 * N);
        if (W < 0)
            W = 0.0;
        X0 = N * pow(1.0 - W + X * sqrt(W), 3);
        if (L == 0)
        {
            *XX = X0;
            return;
        }

        K = 0;
        while (true)
        {
            CHI2(N, X0, &PP, &D);
            if (ABS(D) < 1e-20)
            {
                *XX = X0;
                return;
            }
            *XX = X0 - (PP - P) / D;
            if (ABS(X0 - (*XX)) <= (1e-4 * ABS(*XX)))
                return;
            K++;
            if (K >= 30)
                return;
            X0 = (*XX);
        }
    }
    void PNORMAL(double Q, double *U)
    {
        double P, Y, B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10;
        if (Q <= 0.0 || Q >= 1.0)
        {
            printf("ERROR(pnormal):INPUT FRA CAN NOT >=1 OR <=0\n");
            exit(1);
        }
        if (Q == 0.5)
        {
            *U = 0.0;
            return;
        }
        if (Q < 0.5 && Q > 0.0)
            P = Q;
        else if (Q > 0.5)
            P = 1.0 - Q;

        Y = -1.0 * log(4.0 * P * (1.0 - P));
        B0 = 1.570796288E0;
        B1 = .3706987906E-1;
        B2 = -.8364353598E-3;
        B3 = -.2250947176E-3;
        B4 = .6841218299E-5;
        B5 = .5824238515E-5;
        B6 = -.1045274970E-5;
        B7 = .8360937017E-7;
        B8 = -.3231081277E-8;
        B9 = .3657763036E-10;
        B10 = .6936233982E-12;
        Y = Y * (B0 + Y * (B1 + Y * (B2 + Y * (B3 + Y * (B4 + Y * (B5 + Y * (B6 + Y * (B7 + Y * (B8 + Y * (B9 + Y * B10))))))))));

        *U = sqrt(Y);
        if (Q > 0.5)
            *U = -1.0 * (*U);
    }
    /*
     * parameters : in : u:normal offset point
     * 				out: p:down satat
     */
    void NORMAL(double U, double *P)
    {
        double Y, A1, A2, A3, A4, A5, A6, ER, Q;
        if (U < -5.0)
        {
            *P = 0.0;
            return;
        }
        if (U > 5.0)
        {
            *P = 1.0;
            return;
        }
        Y = ABS(U) / sqrt(2.0);
        A1 = 0.0705230784e0;
        A2 = 0.0422820123e0;
        A3 = 0.0092705272e0;
        A4 = 0.0001520143e0;
        A5 = 0.0002765672e0;
        A6 = 0.0000430638e0;
        ER = 1.e0 - pow((1.e0 + Y * (A1 + Y * (A2 + Y * (A3 + Y * (A4 + Y * (A5 + Y * A6)))))), (-16));
        Q = 0.5 * ER;
        if (U < 0)
            *P = 0.5 - Q;
        else
            *P = 0.5 + Q;
        return;
    }
    /*
     * purpose: compute normal function
     *
     * parameters:
     * 			IN: N: degree of freedom
     * 				CH:chi2 value
     * 				P: down satat. F(CHI2,N)
     * 			OUT:D: value of dencity function
     *
     *
     */
    void CHI2(int N, double CH, double *P, double *D)
    {
        // double PI=atan(1.0)*4.0;
        double PIS = sqrt(PI);
        double X = CH / 2.0;
        double CHS = sqrt(CH);
        double LU, PP, U;
        int N2, i, IAI;

        if (N % 2 == 0)
        {
            LU = log(X) - X;
            *P = 1.0 - exp(-X);
            IAI = 2;
        }
        else
        {
            LU = log(sqrt(X)) - X - log(PIS);
            NORMAL(CHS, &PP);
            *P = 2.0 * (PP - 0.5);
            IAI = 1;
        }

        U = exp(LU);
        if (IAI == N)
        {
            *D = U / CH;
            return;
        }
        N2 = N - 2;
        for (i = IAI; i <= N2; i += 2)
        {
            *P = *P - 2.0 * U / i;
            LU = log(CH / i) + LU;
            U = exp(LU);
        }
        *D = U / CH;
        return;
    }
    void filesync_unlock(def_dev_t lockfd, int locksize)
    {
#ifdef _WIN32
        UnlockFile(lockfd, 0, 0, locksize, 0);
        CloseHandle(lockfd);
#else
        int iret = flock(lockfd, LOCK_UN);
        close(lockfd);
#endif
    }
    int filesync_lock(const char *orbmutex, def_dev_t &lockfd, int &locksize)
    {
#ifdef _WIN32
        lockfd = CreateFile(orbmutex,
                            GENERIC_READ,          // open for reading
                            0,                     // do not share
                            NULL,                  // no security
                            OPEN_EXISTING,         // existing file only
                            FILE_ATTRIBUTE_NORMAL, // normal file
                            NULL);                 // no attr. template
        if (lockfd == INVALID_HANDLE_VALUE)
        {
            LOGE("file = %s,can't open mutex file", orbmutex);
            cout << "Cant open mutex file  to write!" << endl;
            exit(1);
        }
        locksize = GetFileSize(lockfd, NULL); // get the file size for use in the  lockfile function
        if (locksize == INVALID_FILE_SIZE)
        {
            LOGE("file = %s,acquire file size error", orbmutex);
            cout << "file = " << orbmutex << ",acquire file size error!" << endl;
            exit(1);
        }
        return LockFile(lockfd, 0, 0, locksize, 0);
#else
        lockfd = open(orbmutex, O_RDWR | O_CREAT, 0777);
        if (lockfd == -1)
        {
            LOGE("file = %s,can't open mutex file", orbmutex);
            LOGPRINT("file = %s,can't open mutex file", orbmutex);
            exit(1);
        }
        // int iret = flock(*lockfd,LOCK_EX | LOCK_NB);
        int iret = flock(lockfd, LOCK_EX);
        if (iret == -1)
        {
            LOGE("file = %s,can't lock mutex file", orbmutex);
            LOGPRINT("file = %s,can't open mutex file", orbmutex);
            return 0;
        }
#endif
        return 1;
    }
    int filesync_lock_nosync(const char *orbmutex, def_dev_t &lockfd, int &locksize)
    {
#ifdef _WIN32
        lockfd = CreateFile(orbmutex,
                            GENERIC_READ,          // open for reading
                            0,                     // do not share
                            NULL,                  // no security
                            OPEN_EXISTING,         // existing file only
                            FILE_ATTRIBUTE_NORMAL, // normal file
                            NULL);                 // no attr. template
        if (lockfd == INVALID_HANDLE_VALUE)
        {
            LOGE("file = %s,can't open mutex file", orbmutex);
            cout << "Cant open mutex file  to write!" << endl;
            exit(1);
        }
        locksize = GetFileSize(lockfd, NULL); // get the file size for use in the  lockfile function
        if (locksize == INVALID_FILE_SIZE)
        {
            LOGE("file = %s,acquire file size error", orbmutex);
            cout << "file = " << orbmutex << ",acquire file size error!" << endl;
            exit(1);
        }
        return LockFile(lockfd, 0, 0, locksize, 0);
#else
        lockfd = open(orbmutex, O_RDWR | O_CREAT, 0777);
        if (lockfd == -1)
        {
            LOGE("file = %s,can't open mutex file", orbmutex);
            LOGPRINT("file = %s,can't open mutex file", orbmutex);
            exit(1);
        }
        int iret = flock(lockfd, LOCK_EX | LOCK_NB);
        if (iret == -1)
        {
            LOGE("file = %s,can't lock mutex file", orbmutex);
            LOGPRINT("file = %s,can't  mutex file", orbmutex);
            return 0;
        }
#endif
        return 1;
    }
    double stec2m(double stec, int isat, const char *L1)
    {
        double fq = freqbytp(isat, L1);
        return stec * 40.28 * 1e16 / fq / fq;
    }
    double m2stec(double delay, double fq)
    {
        return delay * fq * fq / 40.28 / 1e16;
    }
    int getIndexOfStation(vector<Station> &stalist, string name)
    {
        int isit;
        for (isit = 0; isit < stalist.size(); isit++)
        {
            if (stalist[isit].name == name)
                return isit;
        }
        return -1;
    }
    bool validConfig(const char *config)
    {
        return !strstr(config, "NONE");
    }
    char csys_bytype(const char *type)
    {
        if (strstr(type, "GPS"))
            return 'G';
        if (strstr(type, "GLS"))
            return 'R';
        if (strstr(type, "QZS"))
            return 'J';
        if (strstr(type, "GAL"))
            return 'E';
        if (strstr(type, "BD2"))
            return 'B';
        if (strstr(type, "BD3") || strstr(type, "BDS"))
            return 'C';
        if (strstr(type, "LEO"))
            return 'L';
        return ' ';
    }
    string get_wl_tag_forinput(char csys)
    {
        char wltag[256] = {0};
        const char *p = NULL;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        if (!(p = strstr(dly->m_addargs, "-usemwwl")))
            return "wl";
        else
        {
            sscanf(p, "%s", wltag);
            return index_string(wltag, csys) == -1 ? "wl" : "wl_mw";
        }
    }

    string getcprn_bytype(string cprn, char csys)
    {
        if (csys == 'L')
            return toString("L") + cprn;
        else if (csys == 'B')
            return toString("B") + (cprn.c_str() + 1);
        else if (cprn[0] == csys)
            return cprn;
        else
        {
            /// mixed with other gnss systems, often is the QZSS with GPS
            int offset = 0;
            switch (cprn[0])
            {
            case 'J':
                offset = 100;
                break;
            }
            int iprn = offset + atoi(cprn.c_str() + 1);
            stringstream prnstr;
            prnstr << toString(csys) << iprn << setw(3);
            return toString(prnstr.str());
        }
    }
    bool check_sys(int bunionBDS, int isat, int isys)
    {
        /// check the isat whether belongs to the sys
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        if (bunionBDS && dly->cprn[isat][0] == 'C')
            return dly->cprn[isat][0] == SYS[isys];
        else
            return dly->prn_alias[isat][0] == SYS[isys];
    }

    int index_sys(bool bunionBDS, int isat)
    {
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        if (bunionBDS)
            return index_string(SYS, dly->cprn[isat][0]);
        else
            return index_string(SYS, dly->prn_alias[isat][0]);
    }

    bool bbo_hasobs(int isys, double *obs)
    {
        bool bgotobs = false;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        for (int ifreq = 0; ifreq < dly->nfreq_obs[isys]; ++ifreq)
        {
            if (obs[ifreq] != 0.0)
            {
                bgotobs = true;
                break;
            }
        }
        return bgotobs;
    }
    int bbo_sel_avaobs(int isys, double *obs)
    {
        int idx = -1;
        Deploy *dly = Controller::s_getInstance()->m_getConfigure();
        for (int ifreq = 0; ifreq < dly->nfreq_obs[isys]; ++ifreq)
        {
            if (obs[ifreq] != 0.0)
            {
                idx = ifreq;
                break;
            }
        }
        return idx;
    }
    string s_validCmd(map<string, map<string, string>> &maps, string node, string key)
    {
        if (maps.find(node) == maps.end())
            return "";
        map<string, string> &kv = maps[node];
        if (kv.find(key) == kv.end())
            return "";
        return kv[key];
    }
    /* astronomical arguments: f={l,l',F,D,OMG} (rad) ----------------------------*/
    static void ast_args(double t, double *f)
    {
        static const double fc[][5] = {/* coefficients for iau 1980 nutation */
                                       {134.96340251, 1717915923.2178, 31.8792, 0.051635, -0.00024470},
                                       {357.52910918, 129596581.0481, -0.5532, 0.000136, -0.00001149},
                                       {93.27209062, 1739527262.8478, -12.7512, -0.001037, 0.00000417},
                                       {297.85019547, 1602961601.2090, -6.3706, 0.006593, -0.00003169},
                                       {125.04455501, -6962890.2665, 7.4722, 0.007702, -0.00005939}};
        double tt[4];
        int i, j;

        for (tt[0] = t, i = 1; i < 4; i++)
            tt[i] = tt[i - 1] * t;
        for (i = 0; i < 5; i++)
        {
            f[i] = fc[i][0] * 3600.0;
            for (j = 0; j < 4; j++)
                f[i] += fc[i][j + 1] * tt[j];
            f[i] = fmod(f[i] * AS2R, 2.0 * PI);
        }
    }
    /* sun and moon position in eci (ref [4] 5.1.1, 5.2.1) -----------------------*/
    extern void sunmoonpos_eci(KPL_IO::gtime_t tut, double *rsun, double *rmoon)
    {
        const double ep2000[] = {2000, 1, 1, 12, 0, 0};
        double t, f[5], eps, Ms, ls, rs, lm, pm, rm, sine, cose, sinp, cosp, sinl, cosl;

        trace(4, "sunmoonpos_eci: tut=%s\n", time_str(tut, 3));

        t = KPL_IO::timediff(tut, KPL_IO::epoch2time(ep2000)) / 86400.0 / 36525.0;

        /* astronomical arguments */
        ast_args(t, f);

        /* obliquity of the ecliptic */
        eps = 23.439291 - 0.0130042 * t;
        sine = sin(eps * D2R);
        cose = cos(eps * D2R);

        /* sun position in eci */
        if (rsun)
        {
            Ms = 357.5277233 + 35999.05034 * t;
            ls = 280.460 + 36000.770 * t + 1.914666471 * sin(Ms * D2R) + 0.019994643 * sin(2.0 * Ms * D2R);
            rs = AU * (1.000140612 - 0.016708617 * cos(Ms * D2R) - 0.000139589 * cos(2.0 * Ms * D2R));
            sinl = sin(ls * D2R);
            cosl = cos(ls * D2R);
            rsun[0] = rs * cosl;
            rsun[1] = rs * cose * sinl;
            rsun[2] = rs * sine * sinl;

            trace(5, "rsun =%.3f %.3f %.3f\n", rsun[0], rsun[1], rsun[2]);
        }
        /* moon position in eci */
        if (rmoon)
        {
            lm = 218.32 + 481267.883 * t + 6.29 * sin(f[0]) - 1.27 * sin(f[0] - 2.0 * f[3]) +
                 0.66 * sin(2.0 * f[3]) + 0.21 * sin(2.0 * f[0]) - 0.19 * sin(f[1]) - 0.11 * sin(2.0 * f[2]);
            pm = 5.13 * sin(f[2]) + 0.28 * sin(f[0] + f[2]) - 0.28 * sin(f[2] - f[0]) -
                 0.17 * sin(f[2] - 2.0 * f[3]);
            rm = RE_WGS84 / sin((0.9508 + 0.0518 * cos(f[0]) + 0.0095 * cos(f[0] - 2.0 * f[3]) +
                                 0.0078 * cos(2.0 * f[3]) + 0.0028 * cos(2.0 * f[0])) *
                                D2R);
            sinl = sin(lm * D2R);
            cosl = cos(lm * D2R);
            sinp = sin(pm * D2R);
            cosp = cos(pm * D2R);
            rmoon[0] = rm * cosp * cosl;
            rmoon[1] = rm * (cose * cosp * sinl - sine * sinp);
            rmoon[2] = rm * (sine * cosp * sinl + cose * sinp);

            trace(5, "rmoon=%.3f %.3f %.3f\n", rmoon[0], rmoon[1], rmoon[2]);
        }
    }

} // namespace bamboo
