#pragma once
#include "pch.h"
#include "Component.h"
#include "Vector3.h"
#include "Vector4.h"



struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	Math::Vector4 GetNormalizedColor() const
	{
		Math::Vector4 colorFloat{};
		colorFloat.x = static_cast<float>(r) / 255.0f;
		colorFloat.y = static_cast<float>(g) / 255.0f;
		colorFloat.z = static_cast<float>(b) / 255.0f;
		colorFloat.w = static_cast<float>(a) / 255.0f;
		return colorFloat;
	}

	float* GetNormalizedColorArray() const
	{
		float returnLightArray[4]{};
		returnLightArray[0] = static_cast<float>(r) / 255.0f;
		returnLightArray[1] = static_cast<float>(g) / 255.0f;
		returnLightArray[2] = static_cast<float>(b) / 255.0f;
		returnLightArray[3] = static_cast<float>(a) / 255.0f;
		return returnLightArray;
	}
};

struct Light
{
	enum class LightType
	{
		Point,
		Directional,
	};

	bool Enabled;
	LightType Type;

	Color Color;
	float Attenuation;

	Math::Vector3 Position;
	Math::Vector3 Direction;
};

class LightComponent : public Component
{
public:
	LightComponent(GameObject* owner);
	LightComponent(GameObject* owner, LightComponent* meshComponent);

	Component::Type GetComponentType() override { return Component::Type::Light; };
	void Update(float deltaTime) override;
	void Render() override;

	std::shared_ptr<Light> GetLight() { return m_light; }
	void SetLight(std::shared_ptr<Light> light);

	Json::Object SaveComponent() override;

	void LoadComponent(const rapidjson::Value& object) override;

#ifdef EDITOR
	void ComponentEditor() override;
	void DisplayComponentIcon() override;
#endif // EDITOR

private:
	std::shared_ptr<Light> m_light;
};

