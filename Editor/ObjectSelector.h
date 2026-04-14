#pragma once
#include <vector>

namespace Math
{
	class Matrix;
	class Vector3;
}


class GameObject;
class Editor;

struct SubMesh;

class ObjectSelector
{
public:
	GameObject* SelectedGameObject() { return m_selectedGameObjects.empty() ? nullptr : m_selectedGameObjects[m_selectedGameObjects.size() - 1]; }
	std::vector<GameObject*>& GetSelectedGameObjects() { return m_selectedGameObjects; }
	void SetSelectedGameObject(GameObject* gameObject);
	void AddSelectedGameObject(GameObject* gameObject);
	void RemoveSelectedGameObject(GameObject* gameObject);
	bool IsGameObjectSelected(GameObject* gameObject);
	bool IsAnyParentSelected(GameObject* gameObject);
	void DeselectAllChildren(GameObject* gameObject);
	void HandleSelectedGameObject(Editor* editor);
	void HandleRaycastSelection();
private:
	std::vector<GameObject*> m_selectedGameObjects;


	bool CheckModelRayOverlap(const SubMesh& subMesh, const Math::Matrix& matrix, const Math::Vector3& origin, const Math::Vector3& direction, float& distance);

};

