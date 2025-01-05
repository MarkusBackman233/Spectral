#include "DxMathUtils.h"

using namespace Spectral;


DirectX::XMVECTOR Spectral::DxMathUtils::ToDx(const Math::Vector4& vector)
{
	return DirectX::XMVectorSet(vector.x, vector.y, vector.z, vector.w);
}

DirectX::XMVECTOR Spectral::DxMathUtils::ToDx(const Math::Vector3& vector)
{
	return DirectX::XMVectorSet(vector.x, vector.y, vector.z, 1.0f);
}

DirectX::XMMATRIX Spectral::DxMathUtils::ToDx(const Math::Matrix& matrix)
{
	DirectX::XMFLOAT4X4 dxMatrix(matrix.Data());
	return DirectX::XMLoadFloat4x4(&dxMatrix);
}

Math::Vector4 Spectral::DxMathUtils::ToSp(const DirectX::XMVECTOR& vector)
{
	return Math::Vector4(vector.m128_f32[0], vector.m128_f32[1], vector.m128_f32[2], vector.m128_f32[3]);
}
Math::Vector3 Spectral::DxMathUtils::ToSp(const DirectX::XMFLOAT3& vector)
{
	return Math::Vector3(vector.x, vector.y, vector.z);
}

Math::Matrix Spectral::DxMathUtils::ToSp(const DirectX::XMMATRIX& vector)
{
	Math::Matrix matrix = *static_cast<Math::Matrix*>((void*)&vector);

	return matrix;
}
