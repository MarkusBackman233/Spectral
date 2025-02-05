#include "Script.h"
#include "IOManager.h"
#include "SpectralAssert.h"
#include "GameObject.h"
#include "ObjectManager.h"
#include "iRender.h"
#include "InputManager.h"
#include "Editor.h"
#include "Vector2.h"
#include "CharacterControllerComponent.h"
#include "ComponentFactory.h"
#include "MeshComponent.h"
#include "iPhysics.h"
#include "RigidbodyComponent.h"

Script::Script(const std::string& filename, bool newScript)
	: m_filename(filename)
{
	if (newScript)
	{
        auto file = IOManager::ProjectDirectory / (filename + std::string(".lua"));
        std::ofstream lua_file(file);

        lua_file << "\n";
        lua_file << "\n";
        lua_file << "function Start()\n";
        lua_file << "\n";
        lua_file << "end\n";
        lua_file << "\n";
        lua_file << "\n";
        lua_file << "function Update(deltaTime)\n";
        lua_file << "\n";
        lua_file << "end\n";

        lua_file.close();
	}
    else
    {
        try {

            for (int i = 0; i < static_cast<int>(sol::lib::count); i++)
            {
                m_lua.open_libraries(static_cast<sol::lib>(i));
            }


            m_lua.new_usertype<GameObject>("GameObject",
                "GetCharacterControllerComponent", &GameObject::GetCharacterControllerComponent,
                "GetTransform", &GameObject::GetWorldMatrix,
                "SetTransform", &GameObject::SetWorldMatrix,
                "GetPosition", &GameObject::GetPosition,
                "SetPosition", &GameObject::SetPosition,
                "GetRotation", &GameObject::GetRotation,
                "SetRotation", &GameObject::SetRotation,
                "Rotate", &GameObject::Rotate
            );

            m_lua.new_usertype<CharacterControllerComponent>("CharacterControllerComponent",
                "Move", &CharacterControllerComponent::Move,
                "IsOnGround", &CharacterControllerComponent::IsOnGround
            );

            m_lua.new_usertype<Math::Vector3>("Vector3",
                sol::constructors<Math::Vector3(), Math::Vector3(float, float, float)>(),
                "x", &Math::Vector3::x,
                "y", &Math::Vector3::y,
                "z", &Math::Vector3::z,
                sol::meta_function::addition, sol::overload(
                    [](const Math::Vector3& vec, Math::Vector3 vec2) { return vec + vec2; },
                    [](const Math::Vector3& vec, float scalar) { return vec + scalar; },
                    [](float scalar, const Math::Vector3& vec) { return vec + scalar; }
                ),
                sol::meta_function::subtraction, sol::overload(
                    //[](const Math::Vector3& vec, Math::Vector3 vec2) { return vec - vec2; },
                    [](const Math::Vector3& vec, float scalar) { return vec - scalar; },
                    [](float scalar, const Math::Vector3& vec) { return vec - scalar; }
                ),       
                sol::meta_function::subtraction, sol::resolve<Math::Vector3(const Math::Vector3&)>(&Math::Vector3::operator-),
        
                sol::meta_function::division, sol::overload(
                    [](const Math::Vector3& vec, Math::Vector3 vec2) { return vec / vec2; },
                    [](const Math::Vector3& vec, float scalar) { return vec / scalar; },
                    [](float scalar, const Math::Vector3& vec) { return vec / scalar; }
                ),
                sol::meta_function::multiplication, sol::overload(
                    [](const Math::Vector3& vec, Math::Vector3 vec2) { return vec * vec2; },
                    [](const Math::Vector3& vec, float scalar) { return vec * scalar; },
                    [](float scalar, const Math::Vector3& vec) { return vec * scalar; }
                ),

                "Dot", &Math::Vector3::Dot,
                "Cross", &Math::Vector3::Cross,
                "Length", &Math::Vector3::Length,
                "Normalize", &Math::Vector3::GetNormal
            );


            m_lua.new_usertype<Math::Vector2>("Vector2",
                sol::constructors<Math::Vector2(), Math::Vector2(float, float)>(),
                "x", &Math::Vector2::x,
                "y", &Math::Vector2::y,
                sol::meta_function::addition, sol::overload(
                    [](const Math::Vector2& vec, Math::Vector2 vec2) { return vec + vec2; },
                    [](const Math::Vector2& vec, float scalar) { return vec + scalar; },
                    [](float scalar, const Math::Vector2& vec) { return vec + scalar; }
                ),
                sol::meta_function::subtraction, sol::overload(
                    [](const Math::Vector2& vec, const Math::Vector2& vec2) { return vec - vec2; },
                    [](const Math::Vector2& vec, float scalar) { return vec - scalar; },
                    [](float scalar, const Math::Vector2& vec) { return vec - scalar; }
                ),

                sol::meta_function::division, sol::overload(
                    [](const Math::Vector2& vec, Math::Vector2 vec2) { return vec / vec2; },
                    [](const Math::Vector2& vec, float scalar) { return vec / scalar; },
                    [](float scalar, const Math::Vector2& vec) { return vec / scalar; }
                ),
                sol::meta_function::multiplication, sol::overload(
                    [](const Math::Vector2& vec, Math::Vector2 vec2) { return vec * vec2; },
                    [](const Math::Vector2& vec, float scalar) { return vec * scalar; },
                    [](float scalar, const Math::Vector2& vec) { return vec * scalar; }
                ),
                "Dot", &Math::Vector2::Dot,
                "Cross", &Math::Vector2::Cross,
                "Length", &Math::Vector2::Length,
                "Normalize", &Math::Vector2::GetNormal
            );

            m_lua.new_usertype<Math::Matrix>("Transform",
                "GetPosition", &Math::Matrix::GetPosition,
                "SetPosition", &Math::Matrix::SetPosition,
                "LookAt", &Math::Matrix::LookAt,
                "Front", &Math::Matrix::GetFront,
                "Right", &Math::Matrix::GetRight,
                "Up", &Math::Matrix::GetUp
            );            
            

            m_lua.new_usertype<Physics::RaycastHit>("RaycastHit",
                "hasHit", &Physics::RaycastHit::hasHit,
                "position", &Physics::RaycastHit::position,
                "normal", &Physics::RaycastHit::normal,
                "distance", &Physics::RaycastHit::distance
            );

            m_lua.set_function("GetGameObjectWithName", &GetGameObjectWithName);
            m_lua.set_function("DrawLine", &DrawLine);
            m_lua.set_function("DrawText", &Render::DrawText2D);
            m_lua.set_function("CreateObject", &CreateGameObject);
            m_lua.set_function("AddComponentToGameObject", &AddComponentToGameObject);
            m_lua.set_function("Raycast", &Physics::Raycast);


            m_lua.new_usertype<InputManager>("Input",
                "GetKeyPressed", &InputManager::GetKeyPressed,
                "GetKeyReleased", &InputManager::GetKeyReleased,
                "GetKeyHeld", &InputManager::GetKeyHeld,
                "GetMouseMovement", &InputManager::GetMouseMovement
            );

        }
        catch (const sol::error& e) {
            Assert(false, std::string("Unable to create file " + std::string(e.what())).c_str());
        }
    }
}

void Script::Start(GameObject* GameObject)
{

    ReloadScript();

    m_lua["gameObject"] = GameObject;
    m_lua["Input"] = InputManager::GetInstance();

    sol::function startFunction = m_lua["Start"];
    if (startFunction) {
        try
        {
            startFunction();
        }
        catch (const std::exception&)
        {
#ifdef EDITOR
            Editor::GetInstance()->StopSimulation();
#endif // EDITOR
        }
    }
}

void Script::Update(float deltaTime)
{
    sol::function updateFunction = m_lua["Update"];
    if (updateFunction) {
        try
        {
            updateFunction(deltaTime);
        }
        catch (const std::exception&)
        {
#ifdef EDITOR
            Editor::GetInstance()->StopSimulation();
#endif
        }
    }
}

std::vector<std::string>& Script::GetExposedVariables()
{
    return m_exposedVariables;
}

std::string Script::GetFloatVariable(const std::string& variableName)
{

    for (const auto& var_name : m_exposedVariables) {
        try {
            sol::object var_value = m_lua[var_name];
            std::cout << "Added exposed variable: " << var_name << " with value: ";
            if (var_value.is<double>()) {
                std::cout << var_value.as<double>();
            }
            else if (var_value.is<int>()) {
                std::cout << var_value.as<int>();
            }
            else if (var_value.is<std::string>()) {
                std::cout << var_value.as<std::string>();
            }
            std::cout << std::endl;
        }
        catch (const sol::error& e) {
            Logger::Error("Error accessing variable '" + var_name + "': " + e.what());
        }
    }
    return " ";
    //turn m_lua[variableName];
}

void Script::DrawLine(const Math::Vector3& start, const Math::Vector3& end)
{
    return Render::DrawLine(start,end);
}

GameObject* Script::CreateGameObject(const std::string& name)
{
    return ObjectManager::GetInstance()->CreateObject(name, true);
}

GameObject* Script::GetGameObjectWithName(const std::string& name)
{
    auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();

    for (auto& gameObject : gameObjects)
    {
        if (name == gameObject->GetName())
        {
            return gameObject.get();
        }
    }

    return nullptr;
}

void Script::ReloadScript()
{
    auto filepath = IOManager::ProjectDirectory / (m_filename + std::string(".lua"));
    try {
        m_lua.script_file(filepath.string());

        std::ifstream file(filepath);
        std::string line;

        while (std::getline(file, line)) {
            if (line.find("---@Expose") != std::string::npos) {
                if (std::getline(file, line)) {
                    m_exposedVariables.push_back(line);
                }
            }
        }



    }
    catch (const sol::error& e) {
        Logger::Error("Error reloading script: " + std::string(e.what()));
    }
}

void Script::AddComponentToGameObject(GameObject* gameObject, const std::string& componentName)
{
    std::shared_ptr<Component> component = nullptr;

    if (componentName == "Mesh")
    {
        component = ComponentFactory::CreateComponent(gameObject,Component::Type::Mesh);
        std::dynamic_pointer_cast<MeshComponent>(component)->SetMesh("Default Cube");
        gameObject->AddComponent(component);
    }    
    else if (componentName == "Rigidbody")
    {
        component = ComponentFactory::CreateComponent(gameObject, Component::Type::Rigidbody);
        std::dynamic_pointer_cast<RigidbodyComponent>(component)->SetPhysicsType(PhysXManager::PhysicsType::DynamicActor);
        gameObject->AddComponent(component);
    }    
    else if (componentName == "CubeCollider")
    {
        component = ComponentFactory::CreateComponent(gameObject, Component::Type::PhysicsShape);
        gameObject->AddComponent(component);
    }
    if (component != nullptr)
    {
        component->Start();
    }
}
