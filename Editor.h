#pragma once
#include "pch.h"
#include "src/IMGUI/ImGuizmo.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <deque>
#include <functional>
#include "Vector2.h"
#include "Matrix.h"
#include "Undo.h"

class LevelProperties;
class GameObject;
class ObjectManager;
class Spectral;
class Component;
class Texture;



class Editor
{
public:
	enum PropertyWindowType
	{
		PropertyWindowType_Disabled,
		PropertyWindowType_Texture,
		PropertyWindowType_Normal,
		PropertyWindowType_Roughness,
		PropertyWindowType_Metallic,
		PropertyWindowType_Ao,
		PropertyWindowType_Component,
		PropertyWindowType_Mesh,
	};


	Editor();
	~Editor();
	static Editor* GetInstance() {
		static Editor instance;
		return &instance;
	}
	void PreRender();
	void Render();
	void Update();

	bool UpdateProjectBrowser();

	void HandleRaycastSelection();
	void HandleDropFile(std::string filename);

	void GameObjectListItem(GameObject* gameObject);

	bool EditTransform(Math::Matrix& matrix);

	void SetLevelProperties(LevelProperties* levelProperties) { m_levelProperties = levelProperties; }

	bool IsStarted() const { return m_started; }

	void LogMessage(std::string message);

	ImVec2 GetDefaultTextureSize() const { return m_defaultImageSize; }

	void OpenPropertyWindow(PropertyWindowType propertyType);

	GameObject* SelectedGameObject() { return m_selectedGameObjects.empty() ? nullptr : m_selectedGameObjects[m_selectedGameObjects.size() - 1]; }

	void SetSelectedGameObject(GameObject* gameObject);
	void AddSelectedGameObject(GameObject* gameObject);
	std::vector<GameObject*>& GetSelectedGameObjects() { return m_selectedGameObjects; }

private:

	class EditorWindow
	{
	public:
		EditorWindow(const std::string& windowName, const Math::Vector2& position, const Math::Vector2& size)
		{
			bool open = true;
			ImGui::SetNextWindowPos(ImVec2(position.x, position.y), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(size.x, size.y), ImGuiCond_Always);
			ImGui::Begin(windowName.c_str(), &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		}
		~EditorWindow() 
		{
			ImGui::End();
		}
	};




	void PropertiesWindow();
	void TopMenu();
	void GameObjectsWindow();
	void LogWindow();
	void GameObjectComponentWindow();
	void PropertyWindow();

	void ImportWindow();

	void HandleMeshDrawing();

	void TransformWindow();

	LevelProperties* m_levelProperties;

	ImVec2 m_defaultImageSize;

	ImGuizmo::OPERATION m_currentGizmoOperation;
	ImGuizmo::MODE m_currentGizmoMode;
	bool m_useSnap;
	std::vector<GameObject*> m_selectedGameObjects;

	bool m_started;
	bool m_windowsOpen;

	int m_rightMenuSizeX;
	int m_leftMenuSizeX;
	int m_bottomMenuSizeY;

	ImGuiWindowFlags m_defaultWindowFlags;

	PropertyWindowType m_PropertyWindowType;

	ImGuiViewport* m_mainViewport;

	std::unordered_map<int, std::string> m_componentTypes;

	std::deque<std::string> m_logBuffer;

	std::function<void()> m_returnPropertyFunction;
	std::string m_currentFilepath;

	static int ColorPickerMask;

	std::deque<Undo> m_undoStack;

	bool m_isImporting;

	enum ImportFiletype
	{
		ImportFiletype_Mesh,
		ImportFiletype_Texture,
	};

	std::string m_currentImportFilename;
	ImportFiletype m_currentImportFiletype;



};

