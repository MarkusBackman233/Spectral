#include "Vector3.h"


Math::Vector3::Vector3(float _x, float _y, float _z)
	: x(_x), y(_y), z(_z)
{
}
Math::Vector3::Vector3(float _x, float _y, float _z, float _w)
	: x(_x), y(_y), z(_z), w(_w)
{
}

Math::Vector3 Math::Vector3::GetNormal()
{
	float lengthSquared = x * x + y * y + z * z;
	float length = sqrt(lengthSquared);
	return Math::Vector3(x / length, y / length, z / length);
}

Math::Vector3 Math::Vector3::Cross(Math::Vector3& in)
{
	return Math::Vector3(
		y * in.z - z * in.y,
		z * in.x - x * in.z,
		x * in.y - y * in.x);
}

Math::Vector3 Math::Vector3::IntersectPlane(Math::Vector3& planeNormal,Math::Vector3& lineStart, Math::Vector3& lineEnd, float& t)
{
	float ad = lineStart.Dot(planeNormal);
	float bd = lineEnd.Dot(planeNormal);
	t = (planeNormal.Dot(Math::Vector3(x,y,z)) - ad) / (bd - ad);
	return lineStart + (lineEnd - lineStart) * t;
}