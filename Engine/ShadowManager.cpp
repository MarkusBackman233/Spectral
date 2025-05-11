#include "ShadowManager.h"
#include "RenderManager.h"
#include "iRender.h"
#include "ProfilerManager.h"
#include "Mesh.h"
#include "SceneManager.h"
#include "DxMathUtils.h"
#include "MathFunctions.h"

ShadowManager::ShadowManager()
    : m_ShadowMapSize(4096) 
{}

void ShadowManager::CreateResources(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC depthTexDesc;
    ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
    depthTexDesc.Width = m_ShadowMapSize;
    depthTexDesc.Height = m_ShadowMapSize;
    depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
    DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DSVDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC gbDepthTexDesc{};
    gbDepthTexDesc.Format = DXGI_FORMAT_R32_FLOAT;
    gbDepthTexDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gbDepthTexDesc.Texture2D.MipLevels = 1;
    gbDepthTexDesc.Texture2D.MostDetailedMip = 0;
    ThrowIfFailed(device->CreateTexture2D(&depthTexDesc, nullptr, m_shadowDepthTexture.GetAddressOf()));
    ThrowIfFailed(device->CreateDepthStencilView(m_shadowDepthTexture.Get(), &DSVDesc, m_shadowDepthView.GetAddressOf()));
    ThrowIfFailed(device->CreateShaderResourceView(m_shadowDepthTexture.Get(), &gbDepthTexDesc, m_shadowResourceView.GetAddressOf()));

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
        { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "Transform",  0, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform",  1, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform",  2, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "Transform",  3, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    ThrowIfFailed(Render::CreateVertexShader(device, "ShadowDepth_VS.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout));
    ThrowIfFailed(Render::CreatePixelShader(device, "PureWhite_PS.cso", &m_pPixelShader));
    ThrowIfFailed(Render::CreateConstantBuffer(device, sizeof(Math::Matrix), m_pVertexConstantBufferData));


    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ThrowIfFailed(device->CreateSamplerState(&samplerDesc, m_comparisonSamplerState.GetAddressOf()));

    m_viewPort.Width = static_cast<float>(m_ShadowMapSize);
    m_viewPort.Height = static_cast<float>(m_ShadowMapSize);
    m_viewPort.MinDepth = 0.0f;
    m_viewPort.MaxDepth = 1.0f;
    m_viewPort.TopLeftX = 0;
    m_viewPort.TopLeftY = 0;

    const auto& lightingSettings = SceneManager::GetInstance()->GetCurrentScene().GetLightingSettings();
    m_camera = std::make_unique<OrthographicCamera>(Math::Vector2(lightingSettings.ShadowCameraSize, lightingSettings.ShadowCameraSize), lightingSettings.NearDepth, lightingSettings.FarDepth);
}

void ShadowManager::DrawShadowDepth(ID3D11DeviceContext* context, const InstanceManager& instanceManager)
{
    ProfileFunction

    if (SceneManager::GetInstance()->GetCurrentScene().GetSun() == nullptr)
    {
        return;
    }

    SetupLightMatrix();

    context->RSSetViewports(1, &m_viewPort);
    ID3D11ShaderResourceView*  pSRV = nullptr;
    context->PSSetShaderResources(4, 1, &pSRV);
    ID3D11DepthStencilView* depthStencil = m_shadowDepthView.Get();

    context->OMSetRenderTargets(0, nullptr, depthStencil);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout, context);
    context->PSSetShader(nullptr, nullptr, 0);
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_camera->GetViewProjectionMatrix(), context);

    UINT stride = sizeof(Mesh::Vertex);
    UINT offset = 0;
    UINT strideInstance = sizeof(Math::Matrix);

    const auto& instances = instanceManager.GetInstances();
    for (const auto& [instance, matrixes] : instances)
    {
        context->IASetVertexBuffers(0, 1, instance.Mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(instance.Mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetVertexBuffers(1, 1, matrixes.Buffer.GetAddressOf(), &strideInstance, &offset);
        context->DrawIndexedInstanced(static_cast<UINT>(instance.Mesh->indices32.size()), matrixes.CurrentInstanceCount, 0, 0, 0);
    }
}


void ShadowManager::SetupLightMatrix()
{
    Scene& scene = SceneManager::GetInstance()->GetCurrentScene();
    Light* sun = scene.GetSun();
    Math::Vector3 cameraPos = Math::SnapToNearest(Render::GetCameraPosition().Swizzle("xoz"), 100.0f);
    Math::Vector3 lightPos = cameraPos - sun->Direction * scene.GetLightingSettings().CameraDistance;
    Math::Vector3 lightTarget = lightPos - sun->Direction;
    m_camera->SetFarClip(scene.GetLightingSettings().FarDepth);
    m_camera->SetNearClip(scene.GetLightingSettings().NearDepth);
    m_camera->SetSize(Math::Vector2(scene.GetLightingSettings().ShadowCameraSize, scene.GetLightingSettings().ShadowCameraSize));
    m_camera->GetWorldMatrix().LookAt(lightPos, lightTarget);
    m_camera->CreateViewAndPerspective();
}
