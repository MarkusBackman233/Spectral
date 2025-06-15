#pragma once
#include "DirectXMath.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"

namespace Spectral
{
	class DxMathUtils
	{
	public:
		static DirectX::XMVECTOR ToDx(const Math::Vector4& vector);
		static DirectX::XMVECTOR ToDx(const Math::Vector3& vector);
		static DirectX::XMMATRIX ToDx(const Math::Matrix& vector);
		static Math::Vector4 ToSp(const DirectX::XMVECTOR& vector);
		static Math::Vector3 ToSp(const DirectX::XMFLOAT3& vector);
		static Math::Matrix ToSp(const DirectX::XMMATRIX& matrix);
	};
}

