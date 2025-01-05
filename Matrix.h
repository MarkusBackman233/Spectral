#pragma once
#include "pch.h"
#include "Vector3.h"
#include "Vector4.h"
#include <algorithm>
#include <cmath>


namespace Math
{
	class Matrix
	{
	public:
		float data[4][4] = {};

		Matrix();
		Matrix(float* matrix);

		float* Data() const;
		Matrix GetInverse() const;
		static Matrix MakeIdentity();
		static Matrix MakeRotationX(float angleRad);
		static Matrix MakeRotationY(float angleRad);
		static Matrix MakeRotationZ(float angleRad);
		static Matrix MakeRotationXYZ(const Vector3& xyzAngleInRad);
		static Matrix MakeRotationXYZAndTranslate(const Vector3& xyzAngleInRad, const Vector3& position);

		static Matrix MakePerspective(float fov, float aspectRatio, float nearClip, float farClip);
		static Matrix MakeTranslation(const Vector3& pos);
		static Matrix MakeRotationFromQuaternion(const Vector4& quat);
		static Matrix MakeRotationAA(const Vector3& axis, float angle);
		static Matrix MakeScale(const Vector3& scale);

		void Transpose();
		void LookAt(const Vector3& pos, const Vector3& target, const Vector3& up = Vector3(0.0f, 1.0f, 0.0f));

		Vector3 GetPosition() const;
		Vector3 GetScale() const;
		Vector4 GetQuaternion() const;
		Vector3 GetRotationXYZInRads() const;

		Matrix  operator * (const Matrix&  A) const;
		Vector4 operator * (const Vector4& A) const;
		Vector3 operator * (const Vector3& A) const;


		 Vector3 GetRight() const;
		 Vector3 GetUp() const;
		 Vector3 GetFront() const;				 
		 float RightLength() const;
		 float UpLength() const;
		 float FrontLength() const;

		 void SetPosition(const Vector3& pos);
		 void SetRight(const Vector3& direction);
		 void SetUp(const Vector3& direction);
		 void SetFront(const Vector3& direction);

		 void OrthoNormalize();
	};
}