#include "CloudGenerator.h"
#include "iRender.h"
#include "Mesh.h"
#include "SkyboxManager.h"
#include "DeviceResources.h"

CloudGenerator::CloudGenerator()
{
	
}

void CloudGenerator::CreateResources(ID3D11Device* device)
{
	const int width = 1024;
	const int height = 1024;
	std::vector<Math::Vector4> cloudData(width * height);

	Perlin perlin;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Math::Vector4& data = cloudData[y * width + x];

			data.w = (float)perlin.noise(x * 0.1, y * 0.1);
			data.w += (float)perlin.noise(x * 0.6, y * 0.6)*0.3f;
			data.w = std::max(data.w, 0.0f);
		}
	}
	float heightScale = 10.0f;

	Math::Vector3 viewer(static_cast<float>(height) * 0.5f,0.0f, static_cast<float>(height) * 0.5f);
	/*
	for (int y = 1; y < height - 1; ++y) {
		for (int x = 1; x < width - 1; ++x) {

			float up = cloudData[(y + 1) * width + x].w;
			float down = cloudData[(y - 1) * width + x].w;
			float left = cloudData[y * width + x - 1].w;
			float right = cloudData[y * width + x + 1].w;

			Math::Vector4& data = cloudData[y * width + x];

			float dx = (right - left) * heightScale;
			float dy = (down - up) * heightScale;

			Math::Vector3 normal = Math::Vector3(-dx, -1.0f, -dy).GetNormal();


			Math::Vector3 cloudPosition(static_cast<float>(x), 30.0f, static_cast<float>(y));
			Math::Vector3 directionToViewer = (viewer - cloudPosition).GetNormal();

			float distanceToCloud = (viewer - cloudPosition).Length();

			float distanceWeight = (1.0f + distanceToCloud * 0.1f);


			normal = (normal + directionToViewer * distanceWeight).GetNormal();

			data.x = directionToViewer.x;
			data.y = directionToViewer.y;
			data.z = directionToViewer.z;
		}
	}
	*/
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = cloudData.data();
	initData.SysMemPitch = width * sizeof(Math::Vector4);

	device->CreateTexture2D(&texDesc, &initData, m_texture.GetAddressOf());


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_resource.GetAddressOf());

	float scale = 100.0f;

    m_cloudMesh = std::make_shared<Mesh>();
    m_cloudMesh->vertexes = {
        { Math::Vector3(scale,  0.5f,  scale) },
        { Math::Vector3(scale,  0.5f, -scale) },
        { Math::Vector3(-scale,  0.5f, -scale) },
        { Math::Vector3(-scale,  0.5f,  scale) },
    };
    m_cloudMesh->indices32 = { 0,1,2,0,2,3 };
    m_cloudMesh->CreateVertexAndIndexBuffer(device);

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


    //m_pixelConstantBuffer.skyboxColor = Math::Vector4(0.6f, 0.6f, 0.6f, 1.0f);
    //Render::CreateConstantBuffer(device, sizeof(VertexConstantBuffer), m_pVertexConstantBufferData);
    //Render::CreateConstantBuffer(device, sizeof(PixelConstantBuffer), m_pPixelConstantBufferData);
    Render::CreatePixelShader(device, "Cloud_PS.cso", &m_cloudPixelShader);
    Render::CreateVertexShader(device, "Cloud_VS.cso", &m_cloudVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout);

}

void CloudGenerator::Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, const SkyboxManager& skyboxManager)
{
	UINT stride = sizeof(Mesh::Vertex);
	UINT offset = 0;

	Render::SetShaders(m_cloudPixelShader, m_cloudVertexShader, m_pInputLayout, context);

	auto cubeMap = skyboxManager.GetIrradianceCubemap();
	float blendFactor[4] = { 0, 0, 0, 0 }; 
	UINT sampleMask = 0xffffffff;
	context->OMSetBlendState(deviceResources.GetTransparentBlendState(), blendFactor, sampleMask);
	context->PSSetShaderResources(0, 1, &cubeMap);
	context->PSSetShaderResources(1, 1, m_resource.GetAddressOf());
	context->IASetVertexBuffers(0, 1, m_cloudMesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_cloudMesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(static_cast<UINT>(m_cloudMesh->indices32.size()),0,0);


	context->OMSetBlendState(deviceResources.GetDefaultBlendState(), blendFactor, sampleMask);
}