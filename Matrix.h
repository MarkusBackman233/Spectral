#pragma once
#include "Vector3.h"
#include <immintrin.h>
#include <immintrin.h> 
#include <algorithm>
#include <cmath>


namespace Math
{
	class Matrix
	{
	public:

		

		float m_matrix[4][4] = {0};
		Matrix(void) {}

		//Matrix(float[4][4]);
		Matrix(float* matrix);

		Math::Matrix Inverse();
		Matrix MakeIdentity();
		Matrix MakeRotationX(float angleRad);
		Matrix MakeRotationY(float angleRad);
		Matrix MakeRotationZ(float angleRad);

		Matrix PointAt(Math::Vector3 &pos, Math::Vector3& target, Math::Vector3& up);

		Matrix MakeTranslation(Math::Vector3 & pos);
		Matrix MakeRotationQuaternion(const Math::Vector3& quat);
		Matrix MakeRotationAA(Math::Vector3 axis, float angle);

		Vector3 GetPosition() { return Vector3(m_matrix[0][3], m_matrix[1][3], m_matrix[2][3]); }

		inline Matrix operator * (const Matrix& A) const
		{
			Matrix matrix;
			for (int c = 0; c < 4; c++)
				for (int r = 0; r < 4; r++)
					matrix.m_matrix[r][c] = m_matrix[r][0] * A.m_matrix[0][c] + m_matrix[r][1] * A.m_matrix[1][c] + m_matrix[r][2] * A.m_matrix[2][c] + m_matrix[r][3] * A.m_matrix[3][c];
			return matrix;
		}			

		//inline Matrix operator * (const Matrix& A) const
		//{
		//	Matrix matrix;
		//
		//	// Load the A matrix into SIMD registers
		//	__m128 a_row0 = _mm_loadu_ps(&A.m_matrix[0][0]);
		//	__m128 a_row1 = _mm_loadu_ps(&A.m_matrix[1][0]);
		//	__m128 a_row2 = _mm_loadu_ps(&A.m_matrix[2][0]);
		//	__m128 a_row3 = _mm_loadu_ps(&A.m_matrix[3][0]);
		//
		//	// Transpose the current matrix
		//	__m128 r0 = _mm_loadu_ps(&m_matrix[0][0]);
		//	__m128 r1 = _mm_loadu_ps(&m_matrix[1][0]);
		//	__m128 r2 = _mm_loadu_ps(&m_matrix[2][0]);
		//	__m128 r3 = _mm_loadu_ps(&m_matrix[3][0]);
		//	_MM_TRANSPOSE4_PS(r0, r1, r2, r3);
		//
		//	// Multiply the two matrices using SIMD instructions
		//	__m128 c0 = _mm_add_ps( _mm_add_ps(_mm_dp_ps(r0, a_row0, 0xF1) , _mm_dp_ps(r1, a_row0, 0xF2)) , _mm_add_ps(_mm_dp_ps(r2, a_row0, 0xF4) , _mm_dp_ps(r3, a_row0, 0xF8)));
		//	__m128 c1 = _mm_add_ps( _mm_add_ps(_mm_dp_ps(r0, a_row1, 0xF1) , _mm_dp_ps(r1, a_row1, 0xF2)) , _mm_add_ps(_mm_dp_ps(r2, a_row1, 0xF4) , _mm_dp_ps(r3, a_row1, 0xF8)));
		//	__m128 c2 = _mm_add_ps( _mm_add_ps(_mm_dp_ps(r0, a_row2, 0xF1) , _mm_dp_ps(r1, a_row2, 0xF2)) , _mm_add_ps(_mm_dp_ps(r2, a_row2, 0xF4) , _mm_dp_ps(r3, a_row2, 0xF8)));
		//	__m128 c3 = _mm_add_ps( _mm_add_ps(_mm_dp_ps(r0, a_row3, 0xF1) , _mm_dp_ps(r1, a_row3, 0xF2)) , _mm_add_ps(_mm_dp_ps(r2, a_row3, 0xF4) , _mm_dp_ps(r3, a_row3, 0xF8)));
		//
		//	// Transpose the result
		//	_MM_TRANSPOSE4_PS(c0, c1, c2, c3);
		//
		//	// Store the result back into the matrix
		//	_mm_storeu_ps(&matrix.m_matrix[0][0], c0);
		//	_mm_storeu_ps(&matrix.m_matrix[1][0], c1);
		//	_mm_storeu_ps(&matrix.m_matrix[2][0], c2);
		//	_mm_storeu_ps(&matrix.m_matrix[3][0], c3);
		//
		//	return matrix;
		//}


		inline Math::Vector3 operator * (const Math::Vector3& A) const
		{
			Math::Vector3 vector;
			vector.x = A.x * m_matrix[0][0] + A.y * m_matrix[1][0] + A.z * m_matrix[2][0] + A.w * m_matrix[3][0];
			vector.y = A.x * m_matrix[0][1] + A.y * m_matrix[1][1] + A.z * m_matrix[2][1] + A.w * m_matrix[3][1];
			vector.z = A.x * m_matrix[0][2] + A.y * m_matrix[1][2] + A.z * m_matrix[2][2] + A.w * m_matrix[3][2];
			vector.w = A.x * m_matrix[0][3] + A.y * m_matrix[1][3] + A.z * m_matrix[2][3] + A.w * m_matrix[3][3];
			return vector;
		}		


		//inline Math::Vector3 operator * (const Math::Vector3& A) const
		//{
		//	Math::Vector3 vector;
		//
		//	// Load the matrix into SIMD registers
		//	__m128 row0 = _mm_loadu_ps(&m_matrix[0][0]);
		//	__m128 row1 = _mm_loadu_ps(&m_matrix[1][0]);
		//	__m128 row2 = _mm_loadu_ps(&m_matrix[2][0]);
		//	__m128 row3 = _mm_loadu_ps(&m_matrix[3][0]);
		//
		//	// Broadcast the vector into a SIMD register
		//	__m128 vec = _mm_set_ps(A.x, A.y, A.z, A.w);
		//
		//	// Perform the matrix-vector multiplication using SIMD instructions
		//	__m128 dot0 = _mm_dp_ps(row0, vec, 0xF1);
		//	__m128 dot1 = _mm_dp_ps(row1, vec, 0xF2);
		//	__m128 dot2 = _mm_dp_ps(row2, vec, 0xF4);
		//	__m128 dot3 = _mm_dp_ps(row3, vec, 0xF8);
		//
		//	// Store the result back into the vector
		//	__m128 dot4 = _mm_add_ps(dot0, dot1);
		//	__m128 dot5 = _mm_add_ps(dot2, dot3);
		//	__m128 dot6 = _mm_add_ps(dot4, dot5);
		//	_mm_storeu_ps(&vector.x, dot6);
		//
		//
		//	return vector;
		//}

	};

}