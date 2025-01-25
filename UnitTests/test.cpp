#include "pch.h"
#include <GameObject.h>
#include <ObjectManager.h>
#include <ComponentFactory.h>

TEST(GameObjectTests, Create)
{
	GameObject* gameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");

	const auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();

	auto it = std::find_if(gameObjects.begin(), gameObjects.end(),
		[gameObject](const std::unique_ptr<GameObject>& p) { return p->GetId() == gameObject->GetId(); });

	EXPECT_TRUE(it != gameObjects.end());
}


TEST(GameObjectTests, Destroy)
{
	GameObject* gameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");
	ObjectManager::GetInstance()->Destroy(gameObject);

	const auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();

	auto it = std::find_if(gameObjects.begin(), gameObjects.end(),
		[gameObject](const std::unique_ptr<GameObject>& p) { return p->GetId() == gameObject->GetId(); });

	EXPECT_TRUE(it == gameObjects.end());
}

TEST(GameObjectTests, SetName) 
{
	GameObject* gameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");

	std::string name = "Good Name";
	gameObject->SetName(name);

	EXPECT_EQ(name, gameObject->GetName());
	ObjectManager::GetInstance()->Destroy(gameObject);
}

TEST(GameObjectTests, SetPosition) 
{
	GameObject* gameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");
	Math::Vector3 position(1.0f, 2.0f, 3.0f);

	gameObject->SetPosition(position);

	EXPECT_EQ(gameObject->GetPosition(), position);
	ObjectManager::GetInstance()->Destroy(gameObject);
}

TEST(GameObjectTests, SetRotation)
{
	GameObject* gameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");
	Math::Vector3 rotation(1.0f, 2.0f, 3.0f);

	gameObject->SetRotation(rotation);

	EXPECT_TRUE(gameObject->GetRotation().Dot(rotation) > 0.99999f);
	ObjectManager::GetInstance()->Destroy(gameObject);
}



TEST(GameObjectTests, ChildObject)
{

	GameObject* parent = ObjectManager::GetInstance()->CreateObject("Parent");
	GameObject* child = ObjectManager::GetInstance()->CreateObject("Child");

	child->SetParent(parent);
	std::vector<GameObject*> children = parent->GetChildren();
	
	bool isChilded = std::find(children.begin(), children.end(), child) != children.end();

	EXPECT_TRUE(isChilded);
	ObjectManager::GetInstance()->Destroy(parent);
} 

TEST(GameObjectTests, UnChildObject)
{

	GameObject* parent = ObjectManager::GetInstance()->CreateObject("Parent");
	GameObject* child = ObjectManager::GetInstance()->CreateObject("Child");
	{
		child->SetParent(parent);
		child->SetParent(nullptr);
		std::vector<GameObject*> children = parent->GetChildren();
		bool isChilded = std::find(children.begin(), children.end(), child) != children.end();

		EXPECT_TRUE(isChilded == false);
	}	
	ObjectManager::GetInstance()->Destroy(parent);
}


TEST(GameObjectTests, AddAllComponents)
{

	GameObject* gameObject = ObjectManager::GetInstance()->CreateObject("Object");


	for (int i = 0; i < static_cast<int>(Component::Type::Num); i++)
	{
		Component::Type componentType = static_cast<Component::Type>(i);
		{
			auto component = ComponentFactory::CreateComponent(gameObject, componentType);
			gameObject->AddComponent(component);
		}

		const auto& components = gameObject->GetComponents();

		auto componentIterator = std::find_if(components.begin(), components.end(),
			[&](std::shared_ptr<Component> component)
		{
			return component->GetComponentType() == componentType;
		});

		EXPECT_TRUE(componentIterator != components.end());
	}



	ObjectManager::GetInstance()->Destroy(gameObject);
}