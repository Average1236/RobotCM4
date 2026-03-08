#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <typeinfo>

namespace matrix
{
	using namespace std;

	const double epsilon = 1e-12; // 小于该数判断为0

	// 创建 h行l列的矩阵
	template <typename T1, typename T2>
	vector<vector<T1>> creatMatrix(int h, int l, T2 initialValue)
	{
		vector<vector<T1>> v;
		for (int i = 0; i < h; i++)
		{
			vector<T1> v1(l, initialValue);
			v.push_back(v1);
		}
		return v;
	}

	// 矩阵A+矩阵B=矩阵C，并返回
	template <typename T>
	vector<vector<T>> matrixSum(const vector<vector<T>> &A, const vector<vector<T>> &B)
	{
		if ((typeid(T) != typeid(double)) && (typeid(T) != typeid(float)) && (typeid(T) != typeid(int)))
		{
			cout << "输入数据类型不合法" << endl;
			exit(0);
		}

		int h = A.size();
		int l = A[0].size();
		vector<vector<T>> C;
		C = creatMatrix<T>(h, l, 0);

		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < l; j++)
			{
				C[i][j] = A[i][j] + B[i][j];
				if (abs(C[i][j]) < epsilon)
				{
					C[i][j] = 0.0;
				}
			}
		}
		return C;
	}

	// 矩阵A-矩阵B=矩阵C，并返回
	template <typename T>
	vector<vector<T>> matrixMinus(const vector<vector<T>> &A, const vector<vector<T>> &B)
	{
		if ((typeid(T) != typeid(double)) && (typeid(T) != typeid(float)) && (typeid(T) != typeid(int)))
		{
			cout << "输入数据类型不合法" << endl;
			exit(0);
		}
		int h = A.size();
		int l = A[0].size();
		vector<vector<T>> C;
		C = creatMatrix<T>(h, l, 0);

		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < l; j++)
			{
				C[i][j] = A[i][j] - B[i][j];
				if (abs(C[i][j]) < epsilon)
				{
					C[i][j] = 0.0;
				}
			}
		}
		return C;
	}

	// 矩阵A*矩阵B=矩阵C，并返回
	template <typename T>
	vector<vector<T>> matrixMultiply(const vector<vector<T>> &A, const vector<vector<T>> &B)
	{
		if ((typeid(T) != typeid(double)) && (typeid(T) != typeid(float)) && (typeid(T) != typeid(int)))
		{
			cout << "输入数据类型不合法" << endl;
			exit(0);
		}
		int A_h = A.size();
		int A_l = A[0].size();
		int B_h = B.size();
		int B_l = B[0].size();
		if (A_l != B_h)
		{
			cout << "两矩阵维数无法相乘" << endl;
			exit(0);
		}
		vector<vector<T>> C = creatMatrix<T>(A_h, B_l, 0);
		for (int i = 0; i < A_h; i++)
		{
			for (int j = 0; j < B_l; j++)
			{
				C[i][j] = 0;
				for (int k = 0; k < A_l; k++)
				{
					C[i][j] += A[i][k] * B[k][j];
				}
				if (abs(C[i][j]) < epsilon)
				{
					C[i][j] = 0.0;
				}
				// cout<<C[i][j]<<"\t";
			}
			// cout<<endl;
		}
		return C;
	}

	// 矩阵A*num=矩阵B，并返回
	template <typename T>
	vector<vector<T>> matrixMultiplyNum(const vector<vector<T>> &A, float num)
	{
		if ((typeid(T) != typeid(double)) && (typeid(T) != typeid(float)) && (typeid(T) != typeid(int)))
		{
			cout << "输入数据类型不合法" << endl;
			exit(0);
		}
		int A_h = A.size();
		int A_l = A[0].size();
		vector<vector<T>> B = creatMatrix<T>(A_h, A_l, 0);
		for (int i = 0; i < A_h; i++)
		{
			for (int j = 0; j < A_l; j++)
			{
				B[i][j] = num * A[i][j];
			}
		}
		return B;
	}

	// 输入矩阵A，输出矩阵A的转置矩阵AT
	template <typename T>
	vector<vector<T>> matrixTrans(const vector<vector<T>> &A)
	{
		vector<vector<T>> AT = creatMatrix<T>(A[0].size(), A.size(), 0);
		int h = AT.size();
		int l = AT[0].size();
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < l; j++)
			{
				AT[i][j] = A[j][i];
			}
		}
		return AT;
	}

	// 输入矩阵A,输出矩阵A的逆矩阵inv_A
	template <typename T>
	vector<vector<T>> matrixInverse(const vector<vector<T>> &A)
	{
		if ((typeid(T) != typeid(double)) && (typeid(T) != typeid(float)) && (typeid(T) != typeid(int)))
		{
			cout << "输入数据类型不合法" << endl;
			exit(0);
		}
		if (A.size() != A[0].size())
		{
			cout << "输入矩阵维数不合法" << endl;
			exit(0);
		}
		int n = A.size();
		vector<vector<T>> inv_A = creatMatrix<T>(n, n, 0);
		vector<vector<T>> L = creatMatrix<T>(n, n, 0);
		vector<vector<T>> U = creatMatrix<T>(n, n, 0);
		vector<vector<T>> inv_L = creatMatrix<T>(n, n, 0);
		vector<vector<T>> inv_U = creatMatrix<T>(n, n, 0);
		// LU分解
		// L矩阵对角元素为1
		for (int i = 0; i < n; i++)
		{
			L[i][i] = 1;
		}
		// U矩阵第一行
		for (int i = 0; i < n; i++)
		{
			U[0][i] = A[0][i];
		}
		// L矩阵第一列
		for (int i = 1; i < n; i++)
		{
			L[i][0] = 1.0 * A[i][0] / A[0][0];
		}

		// 计算LU上下三角
		for (int i = 1; i < n; i++)
		{
			// 计算U（i行j列）
			for (int j = i; j < n; j++)
			{
				double tem = 0;
				for (int k = 0; k < i; k++)
				{
					tem += L[i][k] * U[k][j];
				}
				U[i][j] = A[i][j] - tem;
				if (abs(U[i][j]) < epsilon)
				{
					U[i][j] = 0.0;
				}
			}
			// 计算L（j行i列）
			for (int j = i; j < n; j++)
			{
				double tem = 0;
				for (int k = 0; k < i; k++)
				{
					tem += L[j][k] * U[k][i];
				}
				L[j][i] = 1.0 * (A[j][i] - tem) / U[i][i];
				if (abs(L[i][j]) < epsilon)
				{
					L[i][j] = 0.0;
				}
			}
		}
		// L U剩余位置设为0
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				if (i > j)
				{
					U[i][j] = 0.0;
				}
				else if (i < j)
				{
					L[i][j] = 0.0;
				}
			}
		}
		// LU求逆
		// 求矩阵U的逆
		for (int i = 0; i < n; i++)
		{
			inv_U[i][i] = 1 / U[i][i]; // U对角元素的值，直接取倒数
			for (int k = i - 1; k >= 0; k--)
			{
				double s = 0;
				for (int j = k + 1; j <= i; j++)
				{
					s = s + U[k][j] * inv_U[j][i];
				}
				inv_U[k][i] = -s / U[k][k]; // 迭代计算，按列倒序依次得到每一个值，
				if (abs(inv_U[k][i]) < epsilon)
				{
					inv_U[k][i] = 0.0;
				}
			}
		}
		// 求矩阵L的逆
		for (int i = 0; i < n; i++)
		{
			inv_L[i][i] = 1; // L对角元素的值，直接取倒数，这里为1
			for (int k = i + 1; k < n; k++)
			{
				for (int j = i; j <= k - 1; j++)
				{
					inv_L[k][i] = inv_L[k][i] - L[k][j] * inv_L[j][i];
					if (abs(inv_L[k][i]) < epsilon)
					{
						inv_L[k][i] = 0.0;
					}
				}
			}
		}
		inv_A = matrixMultiply(inv_U, inv_L);
		return inv_A;
	}

	template <typename T>
	void showMatrix(const vector<vector<T>> &A)
	{
		int h = A.size();
		int l = A[0].size();
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < l; j++)
			{
				cout << A[i][j] << "\t";
			}
			cout << endl;
		}
	}
}

#endif /*__MATRIX_H__*/
