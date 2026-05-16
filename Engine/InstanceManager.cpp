#include "InstanceManager.h"
#include "iRender.h"
#include "ProfilerManager.h"
#include "Mesh.h"
#include "IMaterial.h"

constexpr uint32_t InitialMaxInstances = 8;

void InstanceManager::AddInstance(DrawableInstance drawable, const Math::Matrix& matrix)
{
    m_pendingInstances[drawable].push_back(matrix);
}

void InstanceManager::AddInstance(DrawableInstance drawable, const InstanceData& staticInstanceData)
{
    m_pendingStaticInstances.push_back(std::make_pair(drawable, staticInstanceData));
}


void InstanceManager::AddInstance(TerrainComponent* terrain)
{
    m_terrainsToRender.push_back(terrain);
}


std::vector<std::pair<DrawableInstance, InstanceManager::InstanceData>> InstanceManager::GetInstances() const
{
    std::vector<std::pair<DrawableInstance, InstanceManager::InstanceData>> buffers;

    for (auto& [drawable, data] : m_instanceBuffers)
    {
        buffers.emplace_back(drawable, data);
    }

    for (auto& [drawable, data] : m_pendingStaticInstances)
    {
        buffers.emplace_back(drawable, data);
    }


    return buffers;
}

void InstanceManager::Clear()
{
    m_pendingStaticInstances.clear();
    m_terrainsToRender.clear();

}

void InstanceManager::Map(ID3D11DeviceContext* context, ID3D11Device* device)
{
    ProfileFunction
    for (auto& [mesh, instanceData] : m_instanceBuffers)
    {
        instanceData.CurrentInstanceCount = 0;
    }

    for (auto& [mesh, matrices] : m_pendingInstances)
    {
        auto it = m_instanceBuffers.find(mesh);
        if (it == m_instanceBuffers.end())
        {

            uint32_t maxInstnaces = InitialMaxInstances;
            while (maxInstnaces < matrices.size())
            {
                maxInstnaces *= 2;
            }

            CreateInstanceBuffer(device, mesh, matrices, maxInstnaces);
            continue;
        }

        auto& instanceData = it->second;

        if (matrices.size() >= it->second.MaxInstanceCount)
        {
            instanceData.Buffer.Reset();

            uint32_t newMaxInstances = instanceData.MaxInstanceCount;
            while (newMaxInstances <= matrices.size())
            {
                newMaxInstances *= 2;
            }

            CreateInstanceBuffer(device, mesh, matrices, newMaxInstances);
            continue;
        }

        instanceData.CurrentInstanceCount = static_cast<uint32_t>(matrices.size());
        Render::MapVectorIntoBuffer(context, instanceData.Buffer.Get(), matrices);
    }
    m_pendingInstances.clear();
}


void InstanceManager::CreateInstanceBuffer(
    ID3D11Device* device, 
    const DrawableInstance& drawable,
    const std::vector<Math::Matrix>& initialMatrices, 
    uint32_t maxInstances
)
{
    D3D11_BUFFER_DESC bufferDesc{};
    D3D11_SUBRESOURCE_DATA bufferData{};

    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(Math::Matrix) * maxInstances;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;



    std::vector<Math::Matrix> copyMatrices;
    copyMatrices.resize(maxInstances);
    memcpy(copyMatrices.data(), initialMatrices.data(), sizeof(Math::Matrix) * initialMatrices.size());
    bufferData.pSysMem = copyMatrices.data();
    bufferData.SysMemPitch = 0;
    bufferData.SysMemSlicePitch = 0;
    InstanceData instanceData{};

    ThrowIfFailed(device->CreateBuffer(&bufferDesc, &bufferData, &instanceData.Buffer));
    instanceData.MaxInstanceCount = maxInstances;
    instanceData.CurrentInstanceCount = static_cast<uint32_t>(initialMatrices.size());

    m_instanceBuffers[drawable] = instanceData;
}

InstanceManager::InstanceData InstanceManager::CreateStaticInstanceBuffer(const std::vector<Math::Matrix>& matrices)
{
    D3D11_BUFFER_DESC bufferDesc{};
    D3D11_SUBRESOURCE_DATA bufferData{};

    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Math::Matrix) * matrices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = sizeof(Math::Matrix);

    bufferData.pSysMem = matrices.data();

    InstanceData instanceData{};

    ThrowIfFailed(Render::GetDevice()->CreateBuffer(&bufferDesc, &bufferData, &instanceData.Buffer));

    instanceData.MaxInstanceCount = static_cast<uint32_t>(matrices.size());
    instanceData.CurrentInstanceCount = static_cast<uint32_t>(matrices.size());

    return instanceData;
}