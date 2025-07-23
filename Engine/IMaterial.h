#pragma once
#include <memory>
#include "LoadableResource.h"
#include "InstanceManager.h"

struct ID3D11DeviceContext;
class DeviceResources;
class Mesh;

class IMaterial : public Resource
{
public:
	virtual void Render(ID3D11DeviceContext* context, const DeviceResources& deviceResources, std::shared_ptr<Mesh> mesh, InstanceManager::InstanceData instanceData) = 0;
};

