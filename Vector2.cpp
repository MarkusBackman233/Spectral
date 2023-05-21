#include "Vector2.h"

Math::Vector2::Vector2(float _x, float _y)
{
	x = _x;
	y = _y;
}
Math::Vector2::Vector2(float _x, float _y, float _w)
{
	x = _x;
	y = _y;
	w = _w;
}

Math::Vector2::Vector2(const Math::Vector3& A)
{
	x = A.x;
	y = A.y;
	w = A.w;
}



Math::Vector2i::Vector2i(int _x, int _y)
{
	x = _x;
	y = _y;
}

Math::Vector2i::Vector2i(const Math::Vector3& A)
{
	x = (int)A.x;
	y = (int)A.y;
}
