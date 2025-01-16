#pragma once
#include <cmath>
#include "Vector3.h"
namespace Math
{
	constexpr float PI = 3.141592654f;
	constexpr float HalfPI = PI * 0.5f;
	constexpr float TwoPI = PI * 2.0f;

	inline constexpr float ConvertToRadians(float degrees) noexcept { return degrees * (PI / 180.0f); };
	inline constexpr float ConvertToDegrees(float radians) noexcept { return radians * (180.0f / PI); };

	inline float SnapToNearest(float value, float step) noexcept { return std::round(value * 1.0f / step) * step; }
	inline Vector3 SnapToNearest(Vector3 value, float step) noexcept {
		return Vector3(SnapToNearest(value.x,step), SnapToNearest(value.y, step), SnapToNearest(value.z, step));
	}
};

