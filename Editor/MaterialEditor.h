#pragma once
#include <memory>
class DefaultMaterial;

class MaterialEditor
{
public:

	static bool RenderGUI(std::shared_ptr<DefaultMaterial> material);

};

