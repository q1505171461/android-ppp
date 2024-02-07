/*
 * CMat.cpp
 *
 *  Created on: 2018年3月12日
 *      Author: juntao, at wuhan university
 */
#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include "include/Utils/Com/CMat.h"
#include "include/Controller/Const.h"
#ifdef MKL
#include "include/Utils/Com/mkl.h"
#include "include/Utils/Com/mkl_cblas.h"
#endif
using namespace std;
using namespace bamboo;
double *CMat::mat(int n, int m)
{
	double *p = NULL;
	if (m < 0 || n < 0)
	{
		cout << "***ERROR(mat):cant be zero!" << endl;
		return NULL;
	}
	p = (double *)malloc(m * n * sizeof(double));
	if (!p)
	{
		cout << "***ERROR(mat):allocate error!" << endl;
		return NULL;
	}
	memset(p, 0, sizeof(double) * m * n);
	return p;
}
int *CMat::imat(int n, int m)
{
	int *p;
	if (n <= 0 || m <= 0)
		return NULL;
	if (!(p = (int *)malloc(sizeof(int) * n * m)))
	{
		cout << "***ERROR(mat):allocate error!" << endl;
		return NULL;
	}
	return p;
}
int CMat::ludcmp(double *A, int n, int *indx, double *d)
{
	double big, s, tmp, *vv = mat(n, 1);
	int i, imax = 0, j, k;
	*d = 1.0;
	for (i = 0; i < n; i++)
	{
		big = 0.0;
		for (j = 0; j < n; j++)
			if ((tmp = fabs(A[i + j * n])) > big)
				big = tmp;
		if (big > 0.0)
			vv[i] = 1.0 / big;
		else
		{
			free(vv);
			return -1;
		}
	}
	for (j = 0; j < n; j++)
	{
		for (i = 0; i < j; i++)
		{
			s = A[i + j * n];
			for (k = 0; k < i; k++)
				s -= A[i + k * n] * A[k + j * n];
			A[i + j * n] = s;
		}
		big = 0.0;
		for (i = j; i < n; i++)
		{
			s = A[i + j * n];
			for (k = 0; k < j; k++)
				s -= A[i + k * n] * A[k + j * n];
			A[i + j * n] = s;
			if ((tmp = vv[i] * fabs(s)) >= big)
			{
				big = tmp;
				imax = i;
			}
		}
		if (j != imax)
		{
			for (k = 0; k < n; k++)
			{
				tmp = A[imax + k * n];
				A[imax + k * n] = A[j + k * n];
				A[j + k * n] = tmp;
			}
			*d = -(*d);
			vv[imax] = vv[j];
		}
		indx[j] = imax;
		if (A[j + j * n] == 0.0)
		{
			free(vv);
			return -1;
		}
		if (j != n - 1)
		{
			tmp = 1.0 / A[j + j * n];
			for (i = j + 1; i < n; i++)
				A[i + j * n] *= tmp;
		}
	}
	free(vv);
	return 0;
}
void CMat::lubksb(const double *A, int n, const int *indx, double *b)
{
	double s;
	int i, ii = -1, ip, j;
	for (i = 0; i < n; i++)
	{
		ip = indx[i];
		s = b[ip];
		b[ip] = b[i];
		if (ii >= 0)
			for (j = ii; j < i; j++)
				s -= A[i + j * n] * b[j];
		else if (s)
			ii = i;
		b[i] = s;
	}
	for (i = n - 1; i >= 0; i--)
	{
		s = b[i];
		for (j = i + 1; j < n; j++)
			s -= A[i + j * n] * b[j];
		b[i] = s / A[i + i * n];
	}
}
int CMat::CMat_Hougenu_old(double *A, int lda, int index, int row, int col, double *u, double *beta)
{
	int i;
	double yty = 0, sigma = 0;
	int iptx = lda * col;
	for (i = index; i < row; i++)
	{
		if (A[i + iptx] == 0)
			continue;
		yty += A[i + iptx] * A[i + iptx];
	}
	sigma = SIGN(1.0, A[index + iptx]) * sqrt(yty);
	for (i = 0; i < row - index; i++)
	{
		if (i == 0)
		{
			u[i] = A[i + index + iptx] + sigma;
			A[i + index + iptx] = -1 * sigma;
		}
		else
		{
			u[i] = A[i + index + iptx];
			A[i + index + iptx] = 0;
		}
	}
	if (sigma == 0 || u[0] == 0)
	{
		*beta = 0.0;
		return 1;
	}
	*beta = -1.0 / (sigma * u[0]);
	return 1;
}

void CMat::CMat_Householder_old(double *A, int lda, int itrans, int m, int n, bool lsav)
{
	int i, j, col;
	double beta, sum;
	double *u = new double[m];
	if (itrans > n || lda < m)
	{
		cout << "***ERROR(CMat_Householder_old):Wrong input arguments!" << endl;
		exit(1);
	}
	/*loop for the whole column*/
	for (col = 0; col < itrans; col++)
	{
		/*get the corresponding u vector*/
		memset(u, 0, sizeof(double) * m);
		if (CMat_Hougenu_old(A, lda, col, m, col, u, &beta))
		{
			/*save the corresponding u*/
			if (lsav)
			{
				if (lda < m + 2)
				{
					cout << "***WARNING(CMat_Householder_old):no more space to save u! " << endl;
				}
				else
				{
					for (i = col; i < m; i++)
					{
						A[lda * col + i + 1] = u[i - col];
					}
					A[lda * col + m + 1] = beta;
				}
			}
			/*apply to other column*/
			for (i = col + 1; i < n; i++)
			{
				/*compute the ut*y*/
				sum = 0.0;
				for (j = col; j < m; j++)
					sum += u[j - col] * A[j + lda * i];
				for (j = col; j < m; j++)
					A[j + lda * i] += beta * sum * u[j - col];
			}
		}
	}
	delete[] u;
}
int CMat::CMat_sHougenu(float *A, int lda, int index, int row, int col, float *u, float *beta)
{
	int i;
	float yty = 0, sigma = 0, u0;
	int iptx = lda * col;
	for (i = index; i < row; i++)
	{
		if (A[i + iptx] == 0)
			continue;
		yty += A[i + iptx] * A[i + iptx];
	}
	sigma = SIGN(1.0, A[index + iptx]) * sqrt(yty);
	for (i = 0; i < row - index; i++)
	{
		if (i == 0)
		{
			u0 = A[i + index + iptx] + sigma;
			A[i + index + iptx] = -1 * sigma;
			u[0] = 1;
		}
		else
		{
			if (u0 != 0)
				u[i] = A[i + index + iptx] / u0;
			A[i + index + iptx] = 0;
		}
	}
	if (sigma == 0)
	{
		*beta = 0.0;
		return 1;
	}
	*beta = -1.0 / sigma * u0;
	return 1;
}
int CMat::CMat_Hougenu(double *A, int lda, int index, int row, int col, double *u, double *beta)
{
	int i;
	double yty = 0, sigma = 0, u0;
	int iptx = lda * col;
	for (i = index; i < row; i++)
	{
		if (A[i + iptx] == 0)
			continue;
		yty += A[i + iptx] * A[i + iptx];
	}
	sigma = SIGN(1.0, A[index + iptx]) * sqrt(yty);
	for (i = 0; i < row - index; i++)
	{
		if (i == 0)
		{
			u0 = A[i + index + iptx] + sigma;
			A[i + index + iptx] = -1 * sigma;
			u[0] = 1;
		}
		else
		{
			if (u0 != 0)
				u[i] = A[i + index + iptx] / u0;
			A[i + index + iptx] = 0;
		}
	}
	if (sigma == 0)
	{
		*beta = 0.0;
		return 1;
	}
	*beta = -1.0 / sigma * u0;
	return 1;
}
void CMat::CMat_sHouseholder(float *A, int lda, int itrans, int m, int n, bool lsav)
{
#ifdef MKL
	float *tau = new float[lda];
	int col, i, j;
	int la = 0, ja = 0;
	int lwork = -1;
	float sum;
	// mkl_set_num_threads(8);
	LAPACKE_sgeqrf(LAPACK_COL_MAJOR, m, itrans, A, lda, tau);
	/***************Apply to next following Columns **********************/
	for (col = itrans; col < n; col++)
	{
		for (i = 0; i < itrans; i++)
		{
			sum = A[lda * col + i];
			for (j = i + 1; j < m; j++)
			{
				sum += A[lda * col + j] * A[i * lda + j];
			}
			A[lda * col + i] -= tau[i] * sum;
			for (j = i + 1; j < m; j++)
			{
				A[lda * col + j] -= tau[i] * sum * A[i * lda + j];
			}
		}
	}
	if (lsav)
	{
		/*****************Save Tau *****************************************/
		if (lda < m + 1)
		{
			cout << "***ERROR(CMat_Householder):TransFormation No More Space To Save Tau!" << endl;
			exit(1);
		}
		for (i = 0; i < itrans; i++)
		{
			A[lda * i + m] = -1 * tau[i];
		}
	}
	else
	{
		/***************Clean The Lower Part*****************************************************/
		for (i = 0; i < itrans; i++)
		{
			for (j = i + 1; j < m; j++)
			{
				A[i * lda + j] = 0.0;
			}
		}
	}
	delete[] tau;
#endif
}
void CMat::CMat_Householder(double *A, int lda, int itrans, int m, int n, bool lsav)
{
#ifdef MKL
	double *tau = new double[lda];
	int col, i, j;
	int la = 0, ja = 0;
	int lwork = -1;
	double sum;
	mkl_set_num_threads(10);
	LAPACKE_dgeqrf(LAPACK_COL_MAJOR, m, itrans, A, lda, tau);
	/***************Apply to next following Columns **********************/
	for (col = itrans; col < n; col++)
	{
		for (i = 0; i < itrans; i++)
		{
			sum = A[lda * col + i];
			for (j = i + 1; j < m; j++)
			{
				sum += A[lda * col + j] * A[i * lda + j];
			}
			A[lda * col + i] -= tau[i] * sum;
			for (j = i + 1; j < m; j++)
			{
				A[lda * col + j] -= tau[i] * sum * A[i * lda + j];
			}
		}
	}
	if (lsav)
	{
		/*****************Save Tau *****************************************/
		if (lda < m + 1)
		{
			cout << "***ERROR(CMat_Householder):TransFormation No More Space To Save Tau!" << endl;
			exit(1);
		}
		for (i = 0; i < itrans; i++)
		{
			A[lda * i + m] = -1 * tau[i];
		}
	}
	else
	{
		/***************Clean The Lower Part*****************************************************/
		for (i = 0; i < itrans; i++)
		{
			for (j = i + 1; j < m; j++)
			{
				A[i * lda + j] = 0.0;
			}
		}
	}
	delete[] tau;
#else
	int i, j, col;
	double *u = new double[m];
	double beta, sum;
	if (itrans > n || lda < m)
	{
		cout << "***ERROR(CMat_Householder):Wrong input arguments!" << endl;
		exit(1);
	}
	/*loop for the whole column*/
	for (col = 0; col < itrans; col++)
	{
		/*get the corresponding u vector*/
		memset(u, 0, sizeof(double) * m);
		if (CMat_Hougenu(A, lda, col, m, col, u, &beta))
		{
			/*save the corresponding u*/
			if (lsav)
			{
				if (lda < m + 1)
				{
					cout << "***WARNING(CMat_Householder):no more space to save u!" << endl;
				}
				else
				{
					for (i = col; i < m - 1; i++)
					{
						A[lda * col + i + 1] = u[i - col + 1];
					}
					A[lda * col + m] = beta;
				}
			}
			/*apply to other column*/
			for (i = col + 1; i < n; i++)
			{
				/*compute the ut*y*/
				sum = 0.0;
				for (j = col; j < m; j++)
					sum += u[j - col] * A[j + lda * i];
				for (j = col; j < m; j++)
					A[j + lda * i] += beta * sum * u[j - col];
			}
		}
	}
	delete[] u;
#endif
}
void CMat::CMat_sCholesky(char uplo, float *a, int lda, int n)
{
#ifdef MKL
	if (LAPACKE_spotrf(CblasColMajor, uplo, n, a, lda))
	{
		LOGPRINT("cholesky function have illegal value!");
		exit(1);
	}
#else
	LOGPRINT("cholesky function is not implement,will exit!");
	exit(1);
#endif
}
void CMat::CMat_Cholesky(char uplo, double *a, int lda, int n)
{
#ifdef MKL
	if (LAPACKE_dpotrf(CblasColMajor, uplo, n, a, lda))
	{
		LOGPRINT("cholesky function have illegal value!");
		exit(1);
	}
#else
	LOGPRINT("cholesky function is not implement,will exit!");
	exit(1);
#endif
	if (uplo == 'L')
	{
		for (int i = 1; i < n; ++i)
			for (int j = 0; j < i; ++j)
				a[i * n + j] = 0.0;
	}
	if (uplo == 'U')
	{
		for (int i = 1; i < n; ++i)
			for (int j = 0; j < i; ++j)
				a[j * n + i] = 0.0;
	}
}

void CMat::CMat_sMatmul(const char *tr, int m, int n, int k, float alpha, float *a, int lda, float *b, int ldb, float beta, float *c, int ldc)
{
#ifdef MKL
	CBLAS_TRANSPOSE trsa, trsb;
	trsa = tr[0] == 'N' ? CblasNoTrans : CblasTrans;
	trsb = tr[1] == 'N' ? CblasNoTrans : CblasTrans;
	cblas_sgemm(CblasColMajor, trsa, trsb, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc);
#else
	double d;
	int i, j, x;
	int opr = tr[0] == 'N' ? (tr[1] == 'N' ? 1 : 2) : (tr[1] == 'N' ? 3 : 4);
	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
		{
			d = 0;
			switch (opr)
			{
			case 1:
				for (x = 0; x < k; x++)
					d += a[lda * x + i] * b[ldb * j + x];
				break;
			case 2:
				for (x = 0; x < k; x++)
					d += a[lda * x + i] * b[ldb * x + j];
				break;
			case 3:
				for (x = 0; x < k; x++)
					d += a[lda * i + x] * b[ldb * j + x];
				break;
			case 4:
				for (x = 0; x < k; x++)
					d += a[lda * i + x] * b[ldb * x + j];
				break;
			}
			c[ldc * j + i] = alpha * d + beta * c[ldc * j + i];
		}
	}
#endif
}
void CMat::CMat_Matmul(const char *tr, int m, int n, int k, double alpha, double *a, int lda, double *b, int ldb, double beta, double *c, int ldc)
{
#ifdef MKL
	CBLAS_TRANSPOSE trsa, trsb;
	trsa = tr[0] == 'N' ? CblasNoTrans : CblasTrans;
	trsb = tr[1] == 'N' ? CblasNoTrans : CblasTrans;
	cblas_dgemm(CblasColMajor, trsa, trsb, m, n, k, alpha, a, lda, b, ldb, beta, c, ldc);
#else
	double d;
	int i, j, x;
	int opr = tr[0] == 'N' ? (tr[1] == 'N' ? 1 : 2) : (tr[1] == 'N' ? 3 : 4);
	for (i = 0; i < m; i++)
	{
		for (j = 0; j < n; j++)
		{
			d = 0;
			switch (opr)
			{
			case 1:
				for (x = 0; x < k; x++)
					d += a[lda * x + i] * b[ldb * j + x];
				break;
			case 2:
				for (x = 0; x < k; x++)
					d += a[lda * x + i] * b[ldb * x + j];
				break;
			case 3:
				for (x = 0; x < k; x++)
					d += a[lda * i + x] * b[ldb * j + x];
				break;
			case 4:
				for (x = 0; x < k; x++)
					d += a[lda * i + x] * b[ldb * x + j];
				break;
			}
			c[ldc * j + i] = alpha * d + beta * c[ldc * j + i];
		}
	}
#endif
}
int CMat::CMat_sInverse(float *A, int lda, int n)
{
#ifdef MKL
	int ret;
	lapack_int *ipiv = (int *)calloc(MAX(1, n), sizeof(lapack_int));
	ret = LAPACKE_sgetrf(LAPACK_COL_MAJOR, n, n, A, lda, ipiv);
	if (ret != 0)
	{
		free(ipiv);
		return 0;
	}
	ret = LAPACKE_sgetri(LAPACK_COL_MAJOR, n, A, lda, ipiv);
	free(ipiv);
	return !ret;
#else
	//    double d,*B;
	//    int i,j,*indx;
	//    indx = imat(n,1);
	//    B = mat(n,n);
	//    for(i = 0;i < n; i++)
	//    	memcpy(B + n * i ,A + lda * i,sizeof(double) * n);
	//    if (ludcmp(B,n,indx,&d)) {free(indx); free(B); return -1;}
	//    for (j=0;j<n;j++) {
	//        for (i=0;i<n;i++) A[i+j*lda]=0.0; A[j+j*lda]=1.0;
	//        lubksb(B,n,indx,A+j*lda);
	//    }
	//    free(indx); free(B);
	return 0;
#endif
}
int CMat::CMat_Inverse(double *A, int lda, int n)
{
#ifdef MKL
	int ret;
	// mkl_set_num_threads(14);
	lapack_int *ipiv = (int *)calloc(MAX(1, n), sizeof(lapack_int));
	ret = LAPACKE_dgetrf(LAPACK_COL_MAJOR, n, n, A, lda, ipiv);
	if (ret != 0)
	{
		free(ipiv);
		return 0;
	}
	ret = LAPACKE_dgetri(LAPACK_COL_MAJOR, n, A, lda, ipiv);
	free(ipiv);
	return !ret;
#else
	double d, *B;
	int i, j, *indx;
	indx = imat(n, 1);
	B = mat(n, n);
	for (i = 0; i < n; i++)
		memcpy(B + n * i, A + lda * i, sizeof(double) * n);
	if (ludcmp(B, n, indx, &d))
	{
		free(indx);
		free(B);
		return -1;
	}
	for (j = 0; j < n; j++)
	{
		for (i = 0; i < n; i++)
			A[i + j * lda] = 0.0;
		A[j + j * lda] = 1.0;
		lubksb(B, n, indx, A + j * lda);
	}
	free(indx);
	free(B);
	return 0;
#endif
}
int CMat::CMat_InverUU(double *U, double *inv, int lda, int n, char uctl)
{
	int col, row, k;
	int *iptx = new int[n];
	double sum;
	if (n < 0)
	{
		cout << "***ERROR(CMat_InverUU):ndim can't be negative!" << endl;
		exit(1);
	}
	for (col = 0; col < n; col++)
	{
		iptx[col] = (col + 1) * col / 2;
	}
	for (col = 0; col < n; col++)
	{
		for (row = 0; row < col; row++)
		{
			sum = 0.0;
			for (k = row; k < col; k++)
			{
				if (uctl == 'H')
					sum += inv[row + iptx[k]] * U[k + iptx[col]];
				else
					sum += inv[row + iptx[k]] * U[k + lda * col];
			}
			if (uctl == 'H')
				inv[row + iptx[col]] = -sum / U[col + iptx[col]];
			else
				inv[row + iptx[col]] = -sum / U[col + lda * col];
		}
		if (uctl == 'H')
			inv[col + iptx[col]] = 1.0 / U[col + iptx[col]];
		else
			inv[col + iptx[col]] = 1.0 / U[col + lda * col];
	}
	delete[] iptx;
	return 0;
}
void CMat::CMat_sSolveLinear(int n, float *A, int lda, float *est, int lde)
{
#ifdef MKL
	LAPACKE_strtrs(LAPACK_COL_MAJOR, 'U', 'N', 'N', n, 1, A, lda, est, lde);
#else
	//	double* invUu, *val;
	//	int i, j;
	//	invUu = (double*) calloc(n * (n + 1) / 2, sizeof(double));
	//	val = (double*) calloc(n, sizeof(double));
	//	CMat_InverUU(A, invUu, lda, n, 'W');
	//	memcpy(val, est, sizeof(double) * n);
	//	/******************Backward to get the solve***********************************************/
	//	for (i = 0; i < n; i++) {
	//		est[i] = 0.0;
	//		for (j = i; j < n; j++) {
	//			est[i] += invUu[j * (j + 1) / 2 + i] * val[j];
	//		}
	//	}
	//	free(invUu);
	//	free(val);
#endif
}
void CMat::CMat_SolveLinear(int n, double *A, int lda, double *est, int lde)
{
#ifdef MKL
	LAPACKE_dtrtrs(LAPACK_COL_MAJOR, 'U', 'N', 'N', n, 1, A, lda, est, lde);
#else
	double *invUu, *val;
	int i, j;
	invUu = (double *)calloc(n * (n + 1) / 2, sizeof(double));
	val = (double *)calloc(n, sizeof(double));
	CMat_InverUU(A, invUu, lda, n, 'W');
	memcpy(val, est, sizeof(double) * n);
	/******************Backward to get the solve***********************************************/
	for (i = 0; i < n; i++)
	{
		est[i] = 0.0;
		for (j = i; j < n; j++)
		{
			est[i] += invUu[j * (j + 1) / 2 + i] * val[j];
		}
	}
	free(invUu);
	free(val);
#endif
}
void CMat::CMat_sPrintMatrix(float *mat, int lda, int row, int col, const char *premsg)
{
	if (row < 0 || col < 0)
	{
		printf("***print_matrix wrong input arguments for the row and col ***");
		return;
	}
	int i, j;
	if (strlen(premsg) != 0)
	{
		printf("%s\n", premsg);
	}
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			printf("%13.7f ", mat[i + j * lda]);
		}
		printf("\n");
	}
}
void CMat::CMat_PrintMatrix(double *mat, int lda, int row, int col, const char *premsg)
{
	if (row < 0 || col < 0)
	{
		printf("***print_matrix wrong input arguments for the row and col ***");
		return;
	}
	int i, j;
	if (strlen(premsg) != 0)
	{
		printf("%s\n", premsg);
	}
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			printf("%8.4lf ", mat[i + j * lda]);
		}
		printf("\n");
	}
}
void CMat::CMat_PrintMatrix_file(double *mat, int lda, int row, int col, const char *premsg, const char *pfile_name)
{
	static FILE *fp = NULL;
	if (fp == NULL)
	{
		fp = fopen(pfile_name, "w");
	}
	if (row < 0 || col < 0)
	{
		printf("***print_matrix wrong input arguments for the row and col ***");
		return;
	}
	int i, j;
	if (strlen(premsg) != 0)
	{
		printf("%s\n", premsg);
	}
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			fprintf(fp, "%9.5lf ", mat[i + j * lda]);
		}
		fprintf(fp, "\n");
	}
	fflush(fp);
	fclose(fp);
	fp = NULL;
}

void CMat::CMat_PrintMatrix_file_add(double *mat, int lda, int row, int col, FILE *fp)
{
	if (row < 0 || col < 0)
	{
		printf("***print_matrix wrong input arguments for the row and col ***");
		return;
	}
	int i, j;
	for (i = 0; i < row; i++)
	{
		for (j = 0; j < col; j++)
		{
			fprintf(fp, "%16.8lf", mat[i + j * lda]);
		}
		fprintf(fp, "\n");
	}
	fflush(fp);
}