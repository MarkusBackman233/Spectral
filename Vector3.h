#pragma once
#include "pch.h"
#include <PxPhysicsAPI.h>
namespace Math
{


	class Vector3
	{
		
	public:
		Vector3(void) {}

		Vector3(physx::PxVec3 physxVector);
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
		inline  Vector3 operator / (const Vector3& A) const
		{
			return Vector3(x / A.x, y / A.y, z / A.z);
		}			
		inline Vector3 operator / (const float A) const
		{
			return Vector3(x / A, y / A, z / A, w / A);
		}	



		inline Vector3& operator += (const float A) 
		{
			x += A; 
			y += A;
			z += A;

			return *this;
		}
		inline Vector3& operator += (const Vector3& A) 
		{
			x += A.x; 
			y += A.y; 
			z += A.z;

			return *this;
		}
		inline Vector3& operator -= (const Vector3& A) 
		{
			x -= A.x; 
			y -= A.y;
			z -= A.z;

			return *this;
		}
		inline Vector3& operator -= (const float& A) 
		{
			x -= A;
			y -= A;
			z -= A;

			return *this;
		}
		inline Vector3& operator *= (const Vector3& A) 
		{
			x *= A.x;
			y *= A.y;
			z *= A.z;

			return *this;
		}
		inline Vector3& operator *= (const float A) 
		{
			x *= A;
			y *= A;
			z *= A;

			return *this;
		}
		inline  Vector3& operator /= (const Vector3& A) 
		{
			x /= A.x;
			y /= A.y;
			z /= A.z;
			return *this;
		}
		inline Vector3& operator /= (const float A) 
		{
			x /= A;
			y /= A;
			z /= A;
			w /= A;
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


		inline float Dot(const Vector3& A) const
		{
			return A.x * x + A.y * y + A.z * z;
		}		
		inline float Length() const // this is length squared in reality
		{
			return x * x + y * y + z * z;
		}		

		Vector3 reflect(const Vector3& normal) const {
			float dotProduct = this->Dot(normal);
			Vector3 scaledNormal = normal * (2.0f * dotProduct);
			return *this - scaledNormal;
		}

		Math::Vector3 Interpolate(const Vector3& A, float fraction);
		Math::Vector3 GetNormal();
		Math::Vector3 GetNegative();
		Math::Vector3 Cross(const Math::Vector3& in) const;
		Math::Vector3 IntersectPlane(const Math::Vector3& planeNormal, const Math::Vector3& lineStart, const Math::Vector3& lineEnd, float& t) const;

		Math::Vector3 EulerToQuat() const;



	};

}