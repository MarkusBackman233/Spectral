#include "Texture.h"
#include <iostream>
#include "Spectral.h"

#include "TextureManager.h"
#include "Vector2.h"
#include <d3d11.h>
#include <DirectXMath.h>

#include "iRender.h"
#define STB_IMAGE_IMPLEMENTATION
#include "src/External/stb_image.h"
#include "src/External/DDSTextureLoader/DDSTextureLoader11.h"
#include "Logger.h"
#include <mutex>

#include "IOManager.h"


Texture::Texture()
{
}

void Texture::LoadTexture(const std::string& filename)
{
	
	LogMessage("Loading Texture: " + filename);
	if (filename.find(".dds") != std::string::npos)
	{
		ID3D11Resource* texture; // Need to clean up??
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
			Render::GetDevice(),
			Render::GetContext().GetContext(),
			std::wstring(IOManager::ProjectDirectoryWide + std::wstring(filename.begin(), filename.end())).c_str(),
			&texture,
			&m_textureSRV));
		texture->Release();
	}
	else
	{
		int width, height, channels;
		unsigned char* imageData = stbi_load(std::string(IOManager::ProjectDirectory + filename).c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!imageData) {
			LogMessage("ERROR: Could not not load texture: " + filename);
			return;
		}

		Render::CreateTexture(imageData, Math::Vector2i(width, height), m_textureSRV, m_texture);
		stbi_image_free(imageData);
		GenerateMips();
	}

}

bool Texture::GenerateMips()
{
	ID3D11Texture2D* tex = nullptr; // FIRST generate mipmap texture________________________________
	D3D11_TEXTURE2D_DESC texDesc2;
	m_texture->GetDesc(&texDesc2);
	texDesc2.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc2.CPUAccessFlags = 0;
	texDesc2.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc2.MipLevels = static_cast<int>(log2(std::max(texDesc2.Width, texDesc2.Height))) + 1;
	texDesc2.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	texDesc2.Usage = D3D11_USAGE_DEFAULT;
	ThrowIfFailed(Render::GetDevice()->CreateTexture2D(&texDesc2, nullptr, &tex));
	
	ID3D11Texture2D* stagTex; // create Staging texture_____________________________________
	D3D11_TEXTURE2D_DESC stagDesc;
	m_texture->GetDesc(&stagDesc);
	stagDesc.ArraySize = 1;
	stagDesc.BindFlags = 0;
	stagDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagDesc.MipLevels = 1;
	stagDesc.MiscFlags = 0;
	stagDesc.Usage = D3D11_USAGE_STAGING;
	ThrowIfFailed(Render::GetDevice()->CreateTexture2D(&stagDesc, nullptr, &stagTex));
	
	// read from image_________________________________________________________________________

	auto lockedContext = Render::GetContext();

	lockedContext.GetContext()->CopyResource(stagTex, m_texture.Get());
	D3D11_MAPPED_SUBRESOURCE mapped;
	lockedContext.GetContext()->Map(stagTex, 0, D3D11_MAP_READ, 0, &mapped);
	UINT* arr = new UINT[(size_t)((float)mapped.RowPitch / (float)sizeof(UINT)) * stagDesc.Height];
	ZeroMemory(arr, mapped.RowPitch * stagDesc.Height);
	CopyMemory(arr, mapped.pData, mapped.RowPitch * stagDesc.Height);
	lockedContext.GetContext()->Unmap(stagTex, 0);
	
	// copy image data into mipmap texture_______________________________________________________________
	auto rect = CD3D11_BOX(0, 0, 0, stagDesc.Width, stagDesc.Height, 1);
	lockedContext.GetContext()->UpdateSubresource(tex, 0, &rect, arr, mapped.RowPitch, mapped.DepthPitch);
	
	// create SRV of mipmap texture______________________________________________________________
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc2.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc2.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	ThrowIfFailed(Render::GetDevice()->CreateShaderResourceView(tex, &srvDesc, &m_textureSRV));
	lockedContext.GetContext()->GenerateMips(m_textureSRV.Get());

	return true;
}
