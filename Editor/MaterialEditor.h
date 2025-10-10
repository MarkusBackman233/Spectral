#pragma once
#include <memory>
class DefaultMaterial;

class MaterialEditor
{
public:

	static void RenderGUI(std::shared_ptr<DefaultMaterial> material);

};

