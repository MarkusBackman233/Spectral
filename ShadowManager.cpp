#include "ShadowManager.h"
#include "RenderManager.h"
#include "iRender.h"
#include "ProfilerManager.h"

ShadowManager::ShadowManager()
    : m_ShadowMapSize(4096) 
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

    D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc;
    ZeroMemory(&DSVDesc, sizeof(DSVDesc));
    DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DSVDesc.Texture2D.MipSlice = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC gbDepthTexDesc;
    ZeroMemory(&gbDepthTexDesc, sizeof(gbDepthTexDesc));
    gbDepthTexDesc.Format = DXGI_FORMAT_R32_FLOAT;
    gbDepthTexDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    gbDepthTexDesc.Texture2D.MipLevels = 1;
    gbDepthTexDesc.Texture2D.MostDetailedMip = 0;
    ThrowIfFailed(Render::GetDevice()->CreateTexture2D(&depthTexDesc, nullptr, m_shadowDepthTexture.GetAddressOf()));
    ThrowIfFailed(Render::GetDevice()->CreateDepthStencilView(m_shadowDepthTexture.Get(), &DSVDesc, m_shadowDepthView.GetAddressOf()));
    ThrowIfFailed(Render::GetDevice()->CreateShaderResourceView(m_shadowDepthTexture.Get(), &gbDepthTexDesc, m_shadowResourceView.GetAddressOf()));  

    D3D11_INPUT_ELEMENT_DESC vertexLayout[] = {
    { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT      , 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
    { "Transform",  0, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "Transform",  1, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "Transform",  2, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "Transform",  3, DXGI_FORMAT_R32G32B32A32_FLOAT   , 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    ThrowIfFailed(Render::CreateVertexShader("C:/Projects/Spectral/x64/Debug/ShadowDepthVertexShader.cso", &m_pVertexShader, vertexLayout, ARRAYSIZE(vertexLayout), &m_pInputLayout));
    ThrowIfFailed(Render::CreatePixelShader("C:/Projects/Spectral/x64/Debug/SimplePixelShader.cso", &m_pPixelShader));
    ThrowIfFailed(Render::CreateConstantBuffer(sizeof(VertexConstantBuffer), m_pVertexConstantBufferData));  


    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Linear filtering
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;


    ThrowIfFailed(Render::GetDevice()->CreateSamplerState(&samplerDesc, &samplerState));
    samplerState.As(&m_comparisonSamplerState);
}

void ShadowManager::DrawShadowDepth(std::unordered_map<std::shared_ptr<Mesh>, std::vector<DirectX::XMFLOAT4X4>>& drawList)
{
    auto timer = ProfilerManager::GetInstance()->GetProfileObject(__func__);
    timer.StartTimer();
    SetupLightMatrix();


    auto deviceRes = RenderManager::GetInstance()->GetDeviceResources();
    {
        auto lockedContext = deviceRes->GetLockedDeviceContext();
        ID3D11DeviceContext* context = lockedContext.GetContext();

        D3D11_VIEWPORT viewport{};
        viewport.Width = static_cast<float>(4096);
        viewport.Height = static_cast<float>(4096);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;

        context->RSSetViewports(1, &viewport);
        ID3D11ShaderResourceView*  pSRV = nullptr;
        context->PSSetShaderResources(5, 1, &pSRV);
        //ID3D11RenderTargetView* renderTarget = deviceRes->GetRenderTarget();
        ID3D11DepthStencilView* depthStencil = m_shadowDepthView.Get();

        //const float dark[] = { 0.098f, 0.098f, 0.098f, 1.000f };
        //context->ClearRenderTargetView(renderTarget, dark);
        context->OMSetRenderTargets(0, nullptr, depthStencil);
        context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    Render::SetShaders(m_pPixelShader, m_pVertexShader, m_pInputLayout);
    {
        auto lockedContext = Render::GetContext();
        lockedContext.GetContext()->PSSetShader(nullptr, nullptr, 0);
    }
    m_vertexConstantBuffer.viewProjection = m_viewProjectionMatrix;
    Render::UpdateConstantBuffer(Render::SHADER_TYPE_VERTEX, 0, m_pVertexConstantBufferData, &m_vertexConstantBuffer);

    UINT stride = sizeof(Mesh::VertexStruct);
    UINT offset = 0;

    for (auto& [mesh, matrixes] : drawList)
    {
        auto lockedContext = deviceRes->GetLockedDeviceContext();
        ID3D11DeviceContext* context = lockedContext.GetContext();

        context->IASetVertexBuffers(0, 1, mesh->m_pVertexBuffer.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(mesh->m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        auto [instanceBufferDesc, instanceBufferData] = Render::CreateVertexBuffer(matrixes);
        deviceRes->GetDevice()->CreateBuffer(&instanceBufferDesc, &instanceBufferData, &m_pInstanceBuffer);

        UINT strideInstance = sizeof(DirectX::XMFLOAT4X4);
        context->IASetVertexBuffers(1, 1, m_pInstanceBuffer.GetAddressOf(), &strideInstance, &offset);

        context->DrawIndexedInstanced(static_cast<UINT>(mesh->indices32.size()), static_cast<UINT>(matrixes.size()), 0, 0, 0);

        DrawedShadows = true;
    }
}


void ShadowManager::SetupLightMatrix()
{
    const float nearPlane = 25.f;
    const float farPlane = 150.0f;

    Math::Vector3 lightDirection(0.1f, -0.6f, -0.9f);
    lightDirection = lightDirection.GetNormal();

    //Math::Vector3 cameraPos(Render::GetCameraPosition().x, 0.0f, Render::GetCameraPosition().z);
    Math::Vector3 cameraPos(-24.839f, 0.0f, 5.0f);

    auto lightPosPre = cameraPos - lightDirection * 100.0f;
    auto lightTargetPre = lightPosPre - lightDirection;

    using namespace DirectX;
    XMVECTOR lightPos = XMLoadFloat3((XMFLOAT3*)(void*)&lightPosPre);
    XMVECTOR targetPos = XMLoadFloat3((XMFLOAT3*)(void*)&lightTargetPre);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Assuming upward is positive Y-axis
    m_viewMatrix = XMMatrixLookAtLH(lightPos, targetPos, up);

    // Calculate the projection matrix
    Math::Vector2 size(100, 100);

    m_projectionMatrix = XMMatrixOrthographicRH(size.x, size.y, nearPlane, farPlane);

    DirectX::XMStoreFloat4x4(&m_viewProjectionMatrix, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(m_viewMatrix, m_projectionMatrix)));
}
