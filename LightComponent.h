#pragma once
#include "pch.h"
#include "Component.h"
#include <string>

struct ColorFLOAT {
	float r;
	float g;
	float b;
	float a;
};


struct Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;

	ColorFLOAT GetNormalizedColor() const
	{
		ColorFLOAT colorFloat{};
		colorFloat.r = static_cast<float>(r) / 255.0f;
		colorFloat.g = static_cast<float>(g) / 255.0f;
		colorFloat.b = static_cast<float>(b) / 255.0f;
		colorFloat.a = static_cast<float>(a) / 255.0f;
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
	int type;
	bool enabled;
	Color color;
	float attenuation;
};

class LightComponent : public Component
{
public:
	LightComponent(GameObject* owner);
	LightComponent(GameObject* owner, LightComponent* meshComponent);

	void Update(float deltaTime) override;

	std::shared_ptr<Light> GetLight() { return m_light; }
	void SetLight(std::shared_ptr<Light> light);

#ifdef EDITOR
	void ComponentEditor() override;
#endif // EDITOR

private:
	std::shared_ptr<Light> m_light;
};

