#include "SkyboxManager.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include "iRender.h"
#include "Mesh.h"

SkyboxManager::SkyboxManager()
{

}

void SkyboxManager::RenderSkybox()
{
    UINT stride = sizeof(Mesh::VertexStruct);
    UINT offset = 0;

    auto lockedContext = Render::GetContext();
    ID3D11DeviceContext* context = lockedContext.GetContext();
    context->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}
