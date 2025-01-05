#pragma once
#include "iRender.h"

class ConstantBuffer
{
public:
	ConstantBuffer(Render::SHADER_TYPE shaderType);

	virtual void* GetConstantBuffer() = 0;

private:
	Render::SHADER_TYPE m_shaderType;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_pConstantBufferData;
};

