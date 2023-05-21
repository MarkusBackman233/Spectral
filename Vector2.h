#pragma once
#include "Vector3.h"
namespace Math
{

	class Vector2
	{
	public:
		Vector2(void) {}

		Vector2(float _x, float _y);
		Vector2(float _x, float _y, float _w);

		Vector2(const Math::Vector3& A);

		inline friend Vector2 operator*(const float& lhs, const Vector2& rhs) {
			return Vector2(lhs * rhs.x, lhs * rhs.y);
		}		
		inline friend Vector2 operator/(const float& lhs, const Vector2& rhs) {
			return Vector2(lhs / rhs.x, lhs / rhs.y);
		}		
		inline friend Vector2 operator+(const float& lhs, const Vector2& rhs) {
			return Vector2(lhs + rhs.x, lhs + rhs.y);
		}		
		inline friend Vector2 operator-(const float& lhs, const Vector2& rhs) {
			return Vector2(lhs - rhs.x, lhs - rhs.y);
		}

		inline Vector2 operator * (const float A) const
		{
			return Vector2(x * A, y * A);
		}	
		inline Vector2 operator / (const float A) const
		{
			return Vector2(x / A, y / A);
		}
		inline Vector2 operator - (const float A) const
		{
			return Vector2(x - A, y - A);
		}		
		inline Vector2 operator + (const float A) const
		{
			return Vector2(x + A, y + A);
		}			
		
		inline Vector2 operator * (const Vector2& A) const
		{
			return Vector2(x * A.x, y * A.y);
		}	
		inline Vector2 operator / (const Vector2& A) const
		{
			return Vector2(x / A.x, y / A.y);
		}
		inline Vector2 operator - (const Vector2& A) const
		{
			return Vector2(x - A.x, y - A.y);
		}		
		inline Vector2 operator + (const Vector2& A) const
		{
			return Vector2(x + A.x, y + A.y);
		}		


		inline float Dot(const Math::Vector2& A) const
		{ 
			return x * A.x + y * A.y;
		}		
		
		inline float Cross(const Math::Vector2& A) const
		{ 
			return (x * A.y) - (y * A.x);
		}
		Math::Vector2 GetNormal()
		{

			float lengthSquared = x * x + y * y;
			float length = sqrt(lengthSquared);
			return Math::Vector2(x / length, y / length);
		}
		inline float Length() const
		{
			return x * x + y * y;
		}

		float x = 0.0f, y = 0.0f, w = 1.0f;
	};	
	class Vector2i
	{
	public:
		Vector2i(void) {}

		Vector2i(int _x, int _y);

		Vector2i(const Math::Vector3& A);
		int x = 0, y = 0;
	};

}