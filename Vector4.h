#pragma once
#include "cmath"
//#define OperatorOverload(op) \
//inline Vector4 operator op=(const Vector4& other) { x op= other.x; y op= other.y; z op= other.z; return *this; } \
//inline Vector4 operator op=(const float other) { x op= other; y op= other; z op= other; return *this; } \
//inline Vector4 operator op(const float other) const { return Vector4(x op other, y op other, z op other); } \
//friend Vector4 operator op(float other, const Vector4& vec) { return Vector4(vec.x op other, vec.y op other, vec.z op other); } \
//inline Vector4 operator op(const Vector4& other) { return Vector4(x op other.x, y op other.y, z op other.z); } \
//friend Vector4 operator op(const Vector4& other, const Vector4& vec) { return Vector4(vec.x op other.x, vec.y op other.y, vec.z op other.z); }
namespace DirectX
{
	struct XMFLOAT4;
}

namespace Math
{

	class Matrix;
	class Vector3;
	class Vector4
	{
		
	public:
		Vector4();
		Vector4(const DirectX::XMFLOAT4& dxVector);
		Vector4(const Vector3& vector3, float _w);
		Vector4(float scalar);
		Vector4(float _x, float _y, float _z);
		Vector4(float _x, float _y, float _z, float _w);
		float x, y, z, w;

		inline Vector4 operator +=(const Vector4& other) {
			x += other.x; y += other.y; z += other.z; return *this;
		} inline Vector4 operator +=(const float other) {
			x += other; y += other; z += other; return *this;
		} inline Vector4 operator + (const float other) const {
			return Vector4(x + other, y + other, z + other);
		} friend Vector4 operator +(float other, const Vector4& vec) {
			return Vector4(vec.x + other, vec.y + other, vec.z + other);
		} inline Vector4 operator +(const Vector4& other) {
			return Vector4(x + other.x, y + other.y, z + other.z);
		} friend Vector4 operator +(const Vector4& other, const Vector4& vec) {
			return Vector4(vec.x + other.x, vec.y + other.y, vec.z + other.z);
		};
		inline Vector4 operator -=(const Vector4& other) {
			x -= other.x; y -= other.y; z -= other.z; return *this;
		} inline Vector4 operator -=(const float other) {
			x -= other; y -= other; z -= other; return *this;
		} inline Vector4 operator - (const float other) const {
			return Vector4(x - other, y - other, z - other);
		} friend Vector4 operator -(float other, const Vector4& vec) {
			return Vector4(vec.x - other, vec.y - other, vec.z - other);
		} inline Vector4 operator -(const Vector4& other) {
			return Vector4(x - other.x, y - other.y, z - other.z);
		} friend Vector4 operator -(const Vector4& other, const Vector4& vec) {
			return Vector4(vec.x - other.x, vec.y - other.y, vec.z - other.z);
		};
		inline Vector4 operator *=(const Vector4& other) {
			x *= other.x; y *= other.y; z *= other.z; return *this;
		} inline Vector4 operator *=(const float other) {
			x *= other; y *= other; z *= other; return *this;
		} inline Vector4 operator * (const float other) const {
			return Vector4(x * other, y * other, z * other);
		} friend Vector4 operator *(float other, const Vector4& vec) {
			return Vector4(vec.x * other, vec.y * other, vec.z * other);
		} inline Vector4 operator *(const Vector4& other) {
			return Vector4(x * other.x, y * other.y, z * other.z);
		} friend Vector4 operator *(const Vector4& other, const Vector4& vec) {
			return Vector4(vec.x * other.x, vec.y * other.y, vec.z * other.z);
		};
		inline Vector4 operator /=(const Vector4& other) {
			x /= other.x; y /= other.y; z /= other.z; return *this;
		} inline Vector4 operator /=(const float other) {
			x /= other; y /= other; z /= other; return *this;
		} inline Vector4 operator / (const float other) const {
			return Vector4(x / other, y / other, z / other);
		} friend Vector4 operator /(float other, const Vector4& vec) {
			return Vector4(vec.x / other, vec.y / other, vec.z / other);
		} inline Vector4 operator /(const Vector4& other) {
			return Vector4(x / other.x, y / other.y, z / other.z);
		} friend Vector4 operator /(const Vector4& other, const Vector4& vec) {
			return Vector4(vec.x / other.x, vec.y / other.y, vec.z / other.z);
		};
		inline Vector4 operator -()
		{				
			x = -x;
			y = -y;
			z = -z;
			w = -w;
			return *this;	
		}
		inline bool operator == (const Vector4& A) const
		{
			return x == A.x && y == A.y && z == A.z && w == A.w;
		}
		inline bool operator != (const Vector4& A) const
		{
			return x != A.x || y != A.y || z != A.z|| w != A.w;
		}
		float* Data() const;


		float Dot(const Vector4& other) const;
		float Length() const;
		float LengthSquared() const;
		Vector4 Transform(const Matrix& matrix) const;
		bool IsFinite() const;

	};
}