#pragma once

class GameObject;
struct ImGuiTextFilter;

class EditorGUI
{
public:
	EditorGUI();
	void Update();


private:

	void GameObjectListItem(GameObject* gameObject, const ImGuiTextFilter& filter);

};

