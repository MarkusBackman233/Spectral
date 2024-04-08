#pragma once
#include "pch.h"
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

		

		float m_matrix[4][4] = {};
		Matrix(void) {
			m_matrix[0][0] = 1.0f;
			m_matrix[1][1] = 1.0f;
			m_matrix[2][2] = 1.0f;
			m_matrix[3][3] = 1.0f;
		}
		Matrix(float* matrix);


		Math::Matrix Inverse();
		Matrix MakeIdentity();
		static Matrix MakeRotationX(float angleRad);
		static Matrix MakeRotationY(float angleRad);
		static Matrix MakeRotationZ(float angleRad);

		void PointAt(const Math::Vector3& pos, const Math::Vector3& target, const Math::Vector3& up);

		static Matrix MakeTranslation(const Math::Vector3& pos);
		static Matrix MakeRotationQuaternion(const Math::Vector3& quat);
		static Matrix MakeRotationAA(Math::Vector3 axis, float angle);

		static Matrix Scale(const Math::Vector3& scale);


		Vector3 GetPosition() const { return Vector3(m_matrix[3][0], m_matrix[3][1], m_matrix[3][2]); }

		void SetPosition(const Math::Vector3& pos)  {
			m_matrix[3][0] = pos.x;			
			m_matrix[3][1] = pos.y;			
			m_matrix[3][2] = pos.z;
			m_matrix[3][3] = 1.0f;
		}

		Math::Vector3 GetScale();

		Vector3 MatrixToQuaternion() const;
		
		inline Matrix operator * (const Matrix& A) const
		{
			Matrix matrix;
			for (int c = 0; c < 4; c++)
				for (int r = 0; r < 4; r++)
					matrix.m_matrix[r][c] = m_matrix[r][0] * A.m_matrix[0][c] + m_matrix[r][1] * A.m_matrix[1][c] + m_matrix[r][2] * A.m_matrix[2][c] + m_matrix[r][3] * A.m_matrix[3][c];
			return matrix;
		}	

		inline Math::Vector3 operator * (const Math::Vector3& A) const
		{
			Math::Vector3 vector;
			vector.x = A.x * m_matrix[0][0] + A.y * m_matrix[1][0] + A.z * m_matrix[2][0] + A.w * m_matrix[3][0];
			vector.y = A.x * m_matrix[0][1] + A.y * m_matrix[1][1] + A.z * m_matrix[2][1] + A.w * m_matrix[3][1];
			vector.z = A.x * m_matrix[0][2] + A.y * m_matrix[1][2] + A.z * m_matrix[2][2] + A.w * m_matrix[3][2];
			vector.w = A.x * m_matrix[0][3] + A.y * m_matrix[1][3] + A.z * m_matrix[2][3] + A.w * m_matrix[3][3];
			return vector;
		}		

		 Math::Vector3 Right() const;
		 Math::Vector3 Up() const;
		 Math::Vector3 Front() const;				 
		 float RightLength() const;
		 float UpLength() const;
		 float FrontLength() const;
		 
		 void SetRight(const Math::Vector3& direction);
		 void SetUp(const Math::Vector3& direction);
		 void SetFront(const Math::Vector3& direction);

		 void OrthoNormalize();

	};

}