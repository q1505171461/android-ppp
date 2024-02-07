/*
 * CMat.h
 *
 *  Created on: 2018/3/12
 *      Author: juntao, at wuhan university
 */

#ifndef INCLUDE_COM_CMAT_H_
#define INCLUDE_COM_CMAT_H_

#include <iostream>
#include <string>
using namespace std;
namespace bamboo
{
	class CMat
	{
	public:
		//// single precision part
		static void CMat_sHouseholder(float *A, int lda, int itrans, int m, int n, bool lsav);
		static void CMat_sMatmul(const char *tr, int m, int n, int k, float alpha, float *a, int lda, float *b, int ldb, float beta, float *c, int ldc);
		static int CMat_sInverse(float *A, int lda, int n);
		static void CMat_sCholesky(char uplo, float *a, int lad, int n);
		static int CMat_sHougenu(float *A, int lda, int index, int row, int col, float *u, float *beta);
		static void CMat_sSolveLinear(int n, float *A, int lda, float *est, int lde);
		static void CMat_sPrintMatrix(float *mat, int lda, int row, int col, const char *premsg);
		//// double precision part
		static int CMat_Hougenu_old(double *A, int lda, int index, int row, int col, double *u, double *beta);
		static void CMat_Householder_old(double *A, int lda, int itrans, int m, int n, bool lsav);
		static int CMat_Hougenu(double *A, int lda, int index, int row, int col, double *u, double *beta);
		static void CMat_Householder(double *A, int lda, int itrans, int m, int n, bool lsav);
		static void CMat_Matmul(const char *tr, int m, int n, int k, double alpha, double *a, int lda, double *b, int ldb, double beta, double *c, int ldc);
		static void CMat_Cholesky(char uplo, double *a, int lad, int n);
		static int CMat_Inverse(double *A, int lda, int n);
		static void CMat_PrintMatrix(double *mat, int lda, int row, int col, const char *premsg);
		static void CMat_PrintMatrix_file(double *mat, int lda, int row, int col, const char *premsg, const char *pfile);
		static int CMat_InverUU(double *U, double *inv, int lda, int n, char uctl);
		static void CMat_SolveLinear(int n, double *A, int lda, double *est, int lde);
		static void CMat_PrintMatrix_file_add(double *mat, int lda, int row, int col, FILE *fp);

	protected:
		static double *mat(int n, int m);
		static int *imat(int n, int m);
		static void lubksb(const double *A, int n, const int *indx, double *b);
		static int ludcmp(double *A, int n, int *indx, double *d);
	};
}
#endif /* INCLUDE_COM_CMAT_H_ */
