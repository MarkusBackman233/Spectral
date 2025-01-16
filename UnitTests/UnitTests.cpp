#include "pch.h"
#include "CppUnitTest.h"
#include "../Spectral/GameObject.h"
#include "../Spectral/ObjectManager.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SpectralUnitTests
{
	TEST_CLASS(GameObjectTests)
	{
	public:
		
		TEST_METHOD(TestName)
		{
			auto gameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");

			std::string name = "Good Name";
			gameObject->SetName(name);
			Assert::AreEqual(name, gameObject->GetName());

			ObjectManager::GetInstance()->Destroy(gameObject);
		}
	};
}
