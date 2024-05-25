#include "LightComponent.h"
#include "ModelManager.h"
#include "Spectral.h"

LightComponent::LightComponent(GameObject* owner)
	: Component(owner)
{
	m_componentName = "LightComponent";
	m_componentType = ComponentType_LightComponent;
	m_light = std::make_shared<Light>();
	m_light->color.r = 255;
	m_light->color.g = 255;
	m_light->color.b = 255;
	m_light->color.a = 255;
}


LightComponent::LightComponent(GameObject* owner, LightComponent* LightComponent)
	: Component(owner)
{
	m_componentName = "LightComponent";
	m_componentType = ComponentType_LightComponent;
	SetLight(LightComponent->GetLight());
}

void LightComponent::Update(float deltaTime)
{

}

void LightComponent::Render()
{
	m_light->position = m_owner->GetMatrix().GetPosition();
	Spectral::GetInstance()->GetLights().push_back(m_light);
}


void LightComponent::SetLight(std::shared_ptr<Light> light)
{

	m_light = std::make_shared<Light>();
	m_light->color.r = light->color.r;
	m_light->color.g = light->color.g;
	m_light->color.b = light->color.b;
	m_light->color.a = light->color.a;
}

#ifdef EDITOR
void LightComponent::ComponentEditor()
{
	float colorRGBA[4]{ (float)GetLight()->color.r / 255,(float)GetLight()->color.g / 255 ,(float)GetLight()->color.b / 255,(float)GetLight()->color.a / 255 };
	if (ImGui::ColorPicker4("Light Color", colorRGBA))
	{
		GetLight()->color.r = static_cast<int>(colorRGBA[0] * 255);
		GetLight()->color.g = static_cast<int>(colorRGBA[1] * 255);
		GetLight()->color.b = static_cast<int>(colorRGBA[2] * 255);
		GetLight()->color.a = static_cast<int>(colorRGBA[3] * 255);
	}
	const char* items[] = { "Point","Directional" };
	static int selectedLightType = 0;
	int oldSelectedItem = selectedLightType;
	if (ImGui::Combo("Light Type", &selectedLightType, items, IM_ARRAYSIZE(items)))
	{
		GetLight()->type = selectedLightType;
	}
}
#endif