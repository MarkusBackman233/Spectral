#include "LightComponent.h"
#include "iRender.h"
#include "ResourceManager.h"
#include "Editor.h"
#include "GameObject.h"
#include "Texture.h"
#include "Light.h"



LightComponent::LightComponent(GameObject* owner)
	: Component(owner)
{
	m_light = std::make_shared<Light>();
	m_light->Type = Light::LightType::Point;
	m_light->Color.r = 255;
	m_light->Color.g = 255;
	m_light->Color.b = 255;
	m_light->Color.a = 255;
	m_light->Attenuation = 40.0f;
}


LightComponent::LightComponent(GameObject* owner, LightComponent* LightComponent)
	: Component(owner)
{
	SetLight(LightComponent->GetLight());
}

void LightComponent::Update(float deltaTime)
{

}

void LightComponent::Render()
{
	m_light->Position = m_owner->GetWorldMatrix().GetPosition();
	m_light->Direction = m_owner->GetWorldMatrix().GetFront();
	Render::DrawLight(m_light.get());

#ifdef EDITOR
	if(!Editor::GetInstance()->IsStarted())
	{
		static auto lightTexture = ResourceManager::GetInstance()->GetResource<Texture>("Light.png");
		static auto sunTexture = ResourceManager::GetInstance()->GetResource<Texture>("Sun.png");
		if (Editor::GetInstance()->IsStarted() == false)
		{
			if (m_light->Type == Light::LightType::Directional)
			{
				Render::DrawGuizmo(m_owner->GetWorldMatrix().GetPosition(), lightTexture, m_light->Color.GetNormalizedColor());
				Render::DrawLine(m_owner->GetWorldMatrix().GetPosition(), m_owner->GetWorldMatrix().GetPosition() + m_owner->GetWorldMatrix().GetFront()*2);
			}
			else
			{
				Render::DrawGuizmo(m_owner->GetWorldMatrix().GetPosition(), sunTexture, m_light->Color.GetNormalizedColor());
			}
		}
	}
#endif // EDITOR
}


void LightComponent::SetLight(std::shared_ptr<Light> light)
{
	m_light = std::make_shared<Light>(*light.get());
}

Json::Object LightComponent::SaveComponent()
{
	Json::Object object;
	object.emplace("Type", static_cast<int>(m_light->Type));
	object.emplace("Color", Json::Array{ m_light->Color.r, m_light->Color.g ,m_light->Color.b ,m_light->Color.a });
	object.emplace("Attenuation", m_light->Attenuation);

	return std::move(object);
}

void LightComponent::LoadComponent(const rapidjson::Value& object)
{
	if (object.HasMember("Type"))
	{
		m_light->Type = static_cast<Light::LightType>(object["Type"].GetInt());
	}
	const rapidjson::Value& jsonColor = object["Color"];
	m_light->Color.r = jsonColor[0].GetInt();
	m_light->Color.g = jsonColor[1].GetInt();
	m_light->Color.b = jsonColor[2].GetInt();
	m_light->Color.a = jsonColor[3].GetInt();
	m_light->Attenuation = object["Attenuation"].GetFloat();
}

#ifdef EDITOR
void LightComponent::ComponentEditor()
{
	const char* items[] = { "Point","Directional" };
	int selectedLightType = static_cast<int>(m_light->Type);
	if (ImGui::Combo("Light Type", &selectedLightType, items, IM_ARRAYSIZE(items)))
	{
		GetLight()->Type = static_cast<Light::LightType>(selectedLightType);
	}

	if (m_light->Type == Light::LightType::Point)
	{
		float colorRGBA[4]{ (float)GetLight()->Color.r / 255,(float)GetLight()->Color.g / 255 ,(float)GetLight()->Color.b / 255,(float)GetLight()->Color.a / 255 };
		if (ImGui::ColorPicker4("Light Color", colorRGBA, Editor::ColorPickerMask))
		{
			GetLight()->Color.r = static_cast<int>(colorRGBA[0] * 255);
			GetLight()->Color.g = static_cast<int>(colorRGBA[1] * 255);
			GetLight()->Color.b = static_cast<int>(colorRGBA[2] * 255);
			GetLight()->Color.a = static_cast<int>(colorRGBA[3] * 255);
		}

		ImGui::DragFloat("Attenuation", &m_light->Attenuation, 1.0f, 0.0f, 100000.0f);
	}
}
void LightComponent::DisplayComponentIcon()
{
	ImGui::SameLine();

	if (m_light->Type != Light::LightType::Directional)
	{
		auto color = GetLight()->Color.GetNormalizedColor();
		ImGui::ImageWithBg(ResourceManager::GetInstance()->GetResource<Texture>("Sun.png")->GetResourceView().Get(), ImVec2(15, 15), ImVec2(0, 0), ImVec2(1, 1), ImVec4(color.x, color.y, color.z, 1.0f));
	}
	else
	{
		ImGui::Image(ResourceManager::GetInstance()->GetResource<Texture>("Light.png")->GetResourceView().Get(), ImVec2(15, 15), ImVec2(0, 0), ImVec2(1, 1));
	}
}
#endif