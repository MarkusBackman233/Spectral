#include "Texture.h"
#include "Vector2.h"
#include "iRender.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_HDR_IMPLEMENTATION
#include "src/External/stb_image.h"
#include "src/External/DDSTextureLoader/DDSTextureLoader11.h"
#include "Logger.h"
#include "DeviceResources.h"


bool Texture::LoadTexture(unsigned char* bytes, const Math::Vector2i& size)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Render::CreateTexture(bytes, size, texture, DXGI_FORMAT_R8G8B8A8_UNORM);
	GenerateMips(texture.Get());
	return true;
}


bool Texture::Load(const std::filesystem::path& file)
{


	auto stringFilename = file.string();

	Logger::Info("Loading Texture: " + stringFilename);
	if (file.wstring().find(L".dds") != std::wstring::npos)
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> resource;

		ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
			Render::GetDevice(),
			Render::GetContext().GetContext(),
			file.wstring().c_str(),
			resource.GetAddressOf(),
			m_textureSRV.GetAddressOf()));
		ThrowIfFailed(resource.As(&m_texture));
	}
	else
	{
		int width, height, channels;



		if (file.extension().string() == ".hdr")
		{
			float* imageData = stbi_loadf(stringFilename.c_str(), &width, &height, &channels, 3);
			if (!imageData)
			{
				Logger::Error("Could not load HDR texture: " + stringFilename);
				return false;
			}

			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R32G32B32_FLOAT; // or RGBA version
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = 0;

			ThrowIfFailed(Render::GetDevice()->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf()));

			UINT pitch = width * 12;

			Render::GetContext().GetContext()->UpdateSubresource(m_texture.Get(), 0, nullptr, imageData, pitch, 0);

			ThrowIfFailed(Render::GetDevice()->CreateShaderResourceView(m_texture.Get(), nullptr, m_textureSRV.GetAddressOf()));

			stbi_image_free(imageData);
		}
		else
		{
			unsigned char* imageData = stbi_load(stringFilename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
			if (!imageData) {
				Logger::Error("Could not not load texture: " + stringFilename);
				return false;
			}

			Render::CreateTexture(imageData, Math::Vector2i(width, height), m_texture, DXGI_FORMAT_R8G8B8A8_UNORM);
			GenerateMips(m_texture.Get());
			stbi_image_free(imageData);
		}
	}
	return true;
}

bool Texture::LoadFromResource(unsigned char* bytes, size_t size)
{
	int width, height, channels;

	unsigned char* image = stbi_load_from_memory(
		bytes,
		static_cast<int>(size),
		&width, &height, &channels, 0
	);
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	Render::CreateTexture(image, Math::Vector2i(width, height), texture, DXGI_FORMAT_R8G8B8A8_UNORM);
	stbi_image_free(image);
	GenerateMips(texture.Get());
	return true;
}




void Texture::GenerateMips(ID3D11Texture2D* texture)
{
	auto device = Render::GetDevice();
	auto lockedContext = Render::GetContext();

	D3D11_TEXTURE2D_DESC texDesc{};
	texture->GetDesc(&texDesc);

	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.MipLevels = static_cast<UINT>(log2(std::max(texDesc.Width, texDesc.Height))) + 1;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	texDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mipTexture;
	ThrowIfFailed(device->CreateTexture2D(&texDesc, nullptr, mipTexture.GetAddressOf()));

	lockedContext.GetContext()->CopySubresourceRegion(mipTexture.Get(), 0, 0, 0, 0, texture, 0, nullptr);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	ThrowIfFailed(device->CreateShaderResourceView(mipTexture.Get(), &srvDesc, m_textureSRV.GetAddressOf()));

	lockedContext.GetContext()->GenerateMips(m_textureSRV.Get());
}
