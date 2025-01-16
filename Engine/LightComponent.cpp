#include "LightComponent.h"
#include "iRender.h"
#include "TextureManager.h"
#include "Editor.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "TextureManager.h"
#include "Texture.h"
#include "RenderManager.h"

LightComponent::LightComponent(GameObject* owner)
	: Component(owner)
{
	m_light = std::make_shared<Light>();
	m_light->Enabled = true;
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
	RenderManager::GetInstance()->GetPbrRenderer()->RenderLight(m_light);


#ifdef EDITOR
	static auto texture = TextureManager::GetInstance()->GetTexture("Light.bmp");
	Render::DrawGuizmo(m_owner->GetWorldMatrix().GetPosition(), texture, m_light->Color.GetNormalizedColor());
	if (m_light->Type == Light::LightType::Directional && Editor::GetInstance()->IsStarted() == false)
	{
		Render::DrawLine(m_owner->GetWorldMatrix().GetPosition(), m_owner->GetWorldMatrix().GetPosition() + m_owner->GetWorldMatrix().GetFront()*2);
	}
#endif // EDITOR
}


void LightComponent::SetLight(std::shared_ptr<Light> light)
{
	m_light = std::make_shared<Light>(*light.get());
}

void LightComponent::SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator)
{

	object.AddMember("Enabled", m_light->Enabled,allocator);
	object.AddMember("Type", (int)m_light->Type,allocator);

	rapidjson::Value colorArray(rapidjson::kArrayType);
	colorArray.PushBack(m_light->Color.r,allocator);
	colorArray.PushBack(m_light->Color.g,allocator);
	colorArray.PushBack(m_light->Color.b,allocator);
	colorArray.PushBack(m_light->Color.a,allocator);

	object.AddMember("Color", colorArray,allocator);
	object.AddMember("Attenuation", m_light->Attenuation,allocator);
}

void LightComponent::LoadComponent(const rapidjson::Value& object)
{
	if (object.HasMember("Enabled"))
	{
		m_light->Enabled = object["Enabled"].GetBool();
	}
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
	float colorRGBA[4]{ (float)GetLight()->Color.r / 255,(float)GetLight()->Color.g / 255 ,(float)GetLight()->Color.b / 255,(float)GetLight()->Color.a / 255 };
	if (ImGui::ColorPicker4("Light Color", colorRGBA, Editor::ColorPickerMask))
	{
		GetLight()->Color.r = static_cast<int>(colorRGBA[0] * 255);
		GetLight()->Color.g = static_cast<int>(colorRGBA[1] * 255);
		GetLight()->Color.b = static_cast<int>(colorRGBA[2] * 255);
		GetLight()->Color.a = static_cast<int>(colorRGBA[3] * 255);
	}

	ImGui::DragFloat("Attenuation", &m_light->Attenuation,1.0f,0.0f,100000.0f);
	
	const char* items[] = { "Point","Directional" };
	static int selectedLightType = 0;
	int oldSelectedItem = selectedLightType;
	if (ImGui::Combo("Light Type", &selectedLightType, items, IM_ARRAYSIZE(items)))
	{
		GetLight()->Type = static_cast<Light::LightType>(selectedLightType);
	}
}
void LightComponent::DisplayComponentIcon()
{
	auto color = GetLight()->Color.GetNormalizedColor();
	ImGui::SameLine();
	ImGui::Image(TextureManager::GetInstance()->GetTexture("Light.bmp")->GetResourceView().Get(), ImVec2(15, 15), ImVec2(0, 0), ImVec2(1, 1), ImVec4(color.x, color.y, color.z, 1.0f));
}
#endif