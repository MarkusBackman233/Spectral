#include "VegetationEditor.h"
#include <src/IMGUI/imgui.h>


VegetationEditor::VegetationEditor(VegetationComponent* vegetationComponent)
	: m_vegetationComponent(vegetationComponent)
{
}

bool VegetationEditor::Update()
{
    bool stayOpen = true;

	if (ImGui::Begin("Vegetation Editor", &stayOpen))
	{

        ImGui::SeparatorText("Brush");


        if (ImGui::Button("Close Vegetation Editor"))
        {
            stayOpen = false;
        }

		ImGui::End();
	}
    return stayOpen;
}