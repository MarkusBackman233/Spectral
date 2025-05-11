#pragma once
#include "cmath"
//#define OperatorOverload(op) \
//inline Vector3 operator op=(const Vector3& other) { x op= other.x; y op= other.y; z op= other.z; return *this; } \
//inline Vector3 operator op=(const float other) { x op= other; y op= other; z op= other; return *this; } \
//inline Vector3 operator op(const float other) const { return Vector3(x op other, y op other, z op other); } \
//friend Vector3 operator op(float other, const Vector3& vec) { return Vector3(vec.x op other, vec.y op other, vec.z op other); } \
//inline Vector3 operator op(const Vector3& other) { return Vector3(x op other.x, y op other.y, z op other.z); } \
//friend Vector3 operator op(const Vector3& other, const Vector3& vec) { return Vector3(vec.x op other.x, vec.y op other.y, vec.z op other.z); }

namespace DirectX
{
	struct XMFLOAT3;
}

namespace Math
{

	class Matrix;
	class Vector4;

	class Vector3
	{
		
	public:
		inline Vector3();
		Vector3(const DirectX::XMFLOAT3& dxVector);
		Vector3(const Vector4& vector4);
		Vector3(float _x, float _y, float _z);
		float x, y, z;

		inline Vector3 operator +=(const Vector3& other) {
			x += other.x; y += other.y; z += other.z; return *this;
		} inline Vector3 operator +=(const float other) {
			x += other; y += other; z += other; return *this;
		} inline Vector3 operator + (const float other) const {
			return Vector3(x + other, y + other, z + other);
		} friend Vector3 operator +(float other, const Vector3& vec) {
			return Vector3(vec.x + other, vec.y + other, vec.z + other);
		} inline Vector3 operator +(const Vector3& other) {
			return Vector3(x + other.x, y + other.y, z + other.z);
		} friend Vector3 operator +(const Vector3& other, const Vector3& vec) {
			return Vector3(vec.x + other.x, vec.y + other.y, vec.z + other.z);
		};
		inline Vector3 operator -=(const Vector3& other) {
			x -= other.x; y -= other.y; z -= other.z; return *this;
		} inline Vector3 operator -=(const float other) {
			x -= other; y -= other; z -= other; return *this;
		} inline Vector3 operator - (const float other) const {
			return Vector3(x - other, y - other, z - other);
		} friend Vector3 operator -(float other, const Vector3& vec) {
			return Vector3(vec.x - other, vec.y - other, vec.z - other);
		} inline Vector3 operator -(const Vector3& other) {
			return Vector3(x - other.x, y - other.y, z - other.z);
		} friend Vector3 operator -(const Vector3& other, const Vector3& vec) {
			return Vector3(vec.x - other.x, vec.y - other.y, vec.z - other.z);
		};
		inline Vector3 operator *=(const Vector3& other) {
			x *= other.x; y *= other.y; z *= other.z; return *this;
		} inline Vector3 operator *=(const float other) {
			x *= other; y *= other; z *= other; return *this;
		} inline Vector3 operator * (const float other) const {
			return Vector3(x * other, y * other, z * other);
		} friend Vector3 operator *(float other, const Vector3& vec) {
			return Vector3(vec.x * other, vec.y * other, vec.z * other);
		} inline Vector3 operator *(const Vector3& other) {
			return Vector3(x * other.x, y * other.y, z * other.z);
		} friend Vector3 operator *(const Vector3& other, const Vector3& vec) {
			return Vector3(vec.x * other.x, vec.y * other.y, vec.z * other.z);
		};
		inline Vector3 operator /=(const Vector3& other) {
			x /= other.x; y /= other.y; z /= other.z; return *this;
		} inline Vector3 operator /=(const float other) {
			x /= other; y /= other; z /= other; return *this;
		} inline Vector3 operator / (const float other) const {
			return Vector3(x / other, y / other, z / other);
		} friend Vector3 operator /(float other, const Vector3& vec) {
			return Vector3(vec.x / other, vec.y / other, vec.z / other);
		} inline Vector3 operator /(const Vector3& other) {
			return Vector3(x / other.x, y / other.y, z / other.z);
		} friend Vector3 operator /(const Vector3& other, const Vector3& vec) {
			return Vector3(vec.x / other.x, vec.y / other.y, vec.z / other.z);
		};
		inline Vector3 operator -()
		{				
			x = -x;
			y = -y;
			z = -z;
			return *this;	
		}
		inline bool operator == (const Vector3& A) const
		{
			return x == A.x && y == A.y && z == A.z;
		}
		inline bool operator != (const Vector3& A) const
		{
			return x != A.x || y != A.y || z != A.z;
		}


		Math::Vector3 Swizzle(const char order[3]) const;

		float Dot(const Vector3& other) const;
		float Length() const;
		float LengthSquared() const;

		float* Data() const;


		Vector3 Reflect(const Vector3& normal) const;
		Vector3 Transform(const Math::Matrix& matrix) const;
		Vector3 TransformNormal(const Math::Matrix& matrix) const;

		Vector3 GetInterpolate(const Vector3& A, float fraction) const;
		Vector3 GetNormal() const;
		Vector3& Normalize();
		Vector3 Cross(const Math::Vector3& in) const;

		bool IsFinite() const;
	};
}