#pragma once
#include <optional>
#include <cmath>
#include <PxPhysicsAPI.h>
#include <foundation/PxVecMath.h>
namespace Math
{

	class Vector3
	{
		
	public:
		Vector3(void) {}

		Vector3(float _x, float _y, float _z);
		Vector3(float _x, float _y, float _z, float _w);
		float x = 0, y = 0, z = 0, w = 1;

		inline Vector3 operator + (const float A) const
		{
			return Vector3(x + A, y + A, z + A);
		}			
		inline Vector3 operator + (const Vector3& A) const
		{
			return Vector3(x + A.x, y + A.y, z + A.z);
		}		
		inline Vector3 operator - (const Vector3& A) const
		{
			return Vector3(x - A.x, y - A.y, z - A.z);
		}				
		inline Vector3 operator - (const float& A) const
		{
			return Vector3(x - A, y - A, z - A);
		}			
		inline Vector3 operator * (const Vector3& A) const
		{
			return Vector3(x * A.x, y * A.y, z * A.z);
		}				
		inline Vector3 operator * (const float A) const
		{
			return Vector3(x * A, y * A, z * A);
		}				
		inline Vector3 operator / (const Vector3& A) const
		{
			return Vector3(x / A.x, y / A.y, z / A.z);
		}			
		inline Vector3 operator / (const float A) const
		{
			return Vector3(x / A, y / A, z / A, w / A);
		}		


		inline float Dot(const Vector3& A) const
		{
			return A.x * x + A.y * y + A.z * z;
		}		
		inline float Length() const
		{
			return x * x + y * y + z * z;
		}

		Math::Vector3 GetNormal();
		Math::Vector3 Cross(Math::Vector3& in);
		Math::Vector3 IntersectPlane(Math::Vector3& planeNormal, Math::Vector3& lineStart, Math::Vector3& lineEnd, float& t);

	};

}