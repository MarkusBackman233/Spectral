#pragma once
#include <memory>
#include "Component.h"
struct Light;

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

