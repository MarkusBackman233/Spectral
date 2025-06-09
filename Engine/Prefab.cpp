#include "Prefab.h"
#include "Logger.h"
#include "IOManager.h"
#include <filesystem>
#include "GameObject.h"
#include "ComponentFactory.h"
#include <fstream>
#include <sstream>
bool Prefab::Load(const std::filesystem::path& file)
{
    std::string jsonData;
    try {
        std::ifstream ifile(file);
        std::stringstream buffer;
        buffer << ifile.rdbuf();

        jsonData = buffer.str();

    }
    catch (const std::exception& e) {
        Logger::Error(e.what());
        return 1;
    }

    rapidjson::Document object;
    if (object.Parse(jsonData.c_str()).HasParseError()) {
        Logger::Error("Error parsing JSON");
        return 1;
    }

    LoadGameObject(object, nullptr);
	return true;
}

void Prefab::Reload(const std::filesystem::path& file)
{
    if (m_prefabRoot)
    {
        DestroyGameObject(m_prefabRoot);
    }
    Load(file);
}

size_t Prefab::GetPrefabId() const
{
    return m_prefabId;
}

GameObject* Prefab::GetPrefabRoot()
{
    return m_prefabRoot;
}

void Prefab::SetPrefabId(size_t id)
{
    m_prefabId = id;
}

void Prefab::LoadGameObject(const rapidjson::Value& object, GameObject* parent) 
{
    GameObject* gameObject = new GameObject();
    if (parent != nullptr)
    {
        gameObject->SetParent(parent);
    }
    else
    {
        m_prefabRoot = gameObject;
    }
    gameObject->SetName(object["Name"].GetString());
    gameObject->SetWorldMatrixNoUpdate(JsonToMatrix(object["Matrix"]));
    gameObject->SetLocalMatrixNoUpdate(JsonToMatrix(object["LocalMatrix"]));


    const rapidjson::Value& components = object["Components"];
    for (rapidjson::SizeType i = 0; i < components.Size(); i++) 
    {
        const rapidjson::Value& componentObject = components[i];

        auto compName = componentObject["Name"].GetString();

        auto component = ComponentFactory::CreateComponent(
            gameObject,
            ComponentFactory::ComponentNames.find(compName)->second,
            nullptr,
            false
        );
        component->LoadComponent(componentObject);
        gameObject->AddComponent(component);
    }

    if (object.HasMember("Children") && object["Children"].IsArray()) 
    {
        const rapidjson::Value& Children = object["Children"];
        for (rapidjson::SizeType i = 0; i < Children.Size(); i++) 
        {
            LoadGameObject(Children[i], gameObject);
        }
    }
}

Math::Matrix Prefab::JsonToMatrix(const rapidjson::Value& array)
{
    Math::Matrix matrix;
    int r = 0;
    int c = 0;
    for (rapidjson::SizeType i = 0; i < array.Size(); i++)
    {
        matrix.data[c][r] = array[i].GetFloat();
        r++;
        if (r >= 4)
        {
            r = 0;
            c++;
        }
    }
    return matrix;
}

void Prefab::DestroyGameObject(GameObject* gameObject)
{
    for (size_t i = 0; i < gameObject->GetChildren().size(); i++)
    {
        DestroyGameObject(gameObject->GetChildren()[i]);
    }

    delete gameObject;
}
