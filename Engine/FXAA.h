#pragma once
#include "PostProcessing.h"
class FXAA :
    public PostProcessing
{

public:
	FXAA();
	void CreateResources(ID3D11Device* device);
	void Process(ID3D11DeviceContext* context, const DeviceResources& deviceResources);
};

