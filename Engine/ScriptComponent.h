#pragma once
#include "pch.h"

#include "Component.h"
#include "Script.h"
class GameObject;


class ScriptComponent :
    public Component
{

public:
    ScriptComponent(GameObject* owner);
    ScriptComponent(GameObject* owner, ScriptComponent* scriptComponent);
    Component::Type GetComponentType() override { return Component::Type::Script; };

    void Start();

    void Update(float deltaTime);


    Json::Object SaveComponent() override;
    void LoadComponent(const rapidjson::Value& object) override;

#ifdef EDITOR
    void ComponentEditor() override;
#endif // EDITOR
    
    std::string GetComponentName() override;
    
    void SetScript(std::shared_ptr<Script> script);
    std::shared_ptr<Script> GetScript() { return m_script; }

private:
    std::shared_ptr<Script> m_script;
};

