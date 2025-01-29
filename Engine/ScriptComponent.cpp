#include "ScriptComponent.h"
#include "src/IMGUI/imgui.h"
#include "PropertyWindowFactory.h"
#include "ScriptManager.h"
#include <Windows.h>
#include "IOManager.h"
#include <shellapi.h>

ScriptComponent::ScriptComponent(GameObject* owner)
	: Component(owner)
{
}

ScriptComponent::ScriptComponent(GameObject* owner, ScriptComponent* scriptComponent)
    : Component(owner)
{
    SetScript(scriptComponent->GetScript());
}


void ScriptComponent::Start()
{
    m_script->Start(m_owner);
}

void ScriptComponent::Update(float deltaTime)
{
    m_script->Update(deltaTime);
}

void ScriptComponent::SaveComponent(rapidjson::Value& object, rapidjson::Document::AllocatorType& allocator)
{
    if (m_script.get() != nullptr)
    {
        object.AddMember("Script", rapidjson::Value(m_script->GetFilename().c_str(), allocator), allocator);
    }
    else
    {
        object.AddMember("Script", "null", allocator);
    }
}

void ScriptComponent::LoadComponent(const rapidjson::Value& object)
{
    if (object.HasMember("Script"))
    {
        auto scriptName = object["Script"].GetString();
        if (scriptName != "null")
        {
            SetScript(ScriptManager::GetInstance()->GetScript(object["Script"].GetString()));
        }
    }
}
#ifdef EDITOR
void ScriptComponent::ComponentEditor()
{
    std::string scriptName = m_script ? m_script->GetFilename() : "Null";
    ImGui::PushID(this);
    ImGui::Text(std::string("Script: " + scriptName).c_str());
    if (ImGui::Button("Select Script##Script"))
    {
        PropertyWindowFactory::SelectScript(this);
    }    
    
    if (ImGui::Button("Edit##Script"))
    {
        ShellExecute(0, 0, (IOManager::ProjectDirectory / (m_script->GetFilename() + std::string(".lua"))).wstring().c_str(), 0, 0, SW_SHOW);
    }

    

    if (m_script)
    {
        for (const auto& variableName : m_script->GetExposedVariables())
        {
            //ImGui::Text(variableName.c_str());
        
            std::string value = m_script->GetFloatVariable(variableName);
        
            auto cstrText = (char*)value.c_str();
            ImGui::InputText(variableName.c_str(), cstrText, 255);
        }
    }

    ImGui::PopID();
}
#endif // EDITOR


#define IF_NULL_RETURN(nullcheck, returnValue) if (nullcheck == nullptr) return returnValue

/*
std::string ScriptComponent::GetComponentName()
{
    IF_NULL_RETURN(m_script,"Script Component");

    const std::string& input = m_script->GetFilename();
    std::string result;

    for (size_t i = 0; i < input.length(); ++i) {
        // Add a space if transitioning from lowercase to uppercase
        if (i > 0 && std::islower(input[i - 1]) && std::isupper(input[i])) {
            result += ' ';
        }
        result += input[i];
    }
    result += " Lua##script";
    return result;
}
*/
void ScriptComponent::SetScript(std::shared_ptr<Script> script)
{
    m_script = std::make_shared<Script>(script->GetFilename(),false);
}
