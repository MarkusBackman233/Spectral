#pragma once
#ifdef EDITOR
#include "pch.h"
#include "src/IMGUI/ImGuizmo.h"
#include "Matrix.h"
#include "Undo.h"
#include "PropertyWindow.h"
#include "EditorCameraController.h"
#include "ObjectSelector.h"
#include "AssetBrowser.h"

class LevelProperties;
class GameObject;
class ObjectManager;
class Component;
class Texture;
class TerrainEditor;
class TerrainComponent;



class Editor
{
public:
	Editor();
	~Editor();
	static Editor* GetInstance() {
		static Editor instance;
		return &instance;
	}
	static int ColorPickerMask;
	 
	void PreRender(); 
	void Render();
	void Update(float deltaTime);

	void HandleDropFile(const std::filesystem::path& filename);

	void GameObjectListItem(GameObject* gameObject, const ImGuiTextFilter& filter);

	bool EditTransform(Math::Matrix& matrix);

	void SetLevelProperties(LevelProperties* levelProperties) { m_levelProperties = levelProperties; }

	bool IsStarted() const { return m_started; }

	void StopSimulation();
	 
	ImVec2 GetDefaultTextureSize() const { return m_defaultImageSize; }

	void SetPropertyWindow(std::shared_ptr<PropertyWindow> propertyWindow);

	void AddUndoAction(std::shared_ptr<Undo> undo);

	bool IsViewportHovered() const { return m_isViewportHovered; }

	ObjectSelector* GetObjectSelector() { return &m_objectSelector; }
	EditorCameraController* GetEditorCameraController() { return &m_editorCameraController; }
	AssetBrowser* GetAssetBrowser() { return &m_assetBrowser; }

	void OpenTerrainEditor(TerrainComponent* terrainComponent);

private:
	void PropertiesWindow();
	void TopMenu();
	void GameObjectsWindow();
	void GameObjectComponentWindow();
	void TransformWindow();

	void DrawObjectOutline(GameObject* gameObject, bool isChildOfSelected);
	void DrawGrid();
	void Viewport(bool enableObjectSelection = true);
	Math::Vector3 GetPositionInFontOfCamera(float distance);

	LevelProperties* m_levelProperties;
	EditorCameraController m_editorCameraController;

	ImVec2 m_defaultImageSize;

	ImGuizmo::OPERATION m_currentGizmoOperation;
	ImGuizmo::MODE m_currentGizmoMode;
	bool m_useSnap;
	bool m_started;
	bool m_windowsOpen;

	bool m_isViewportHovered;

	int m_rightMenuSizeX;
	int m_leftMenuSizeX;
	int m_bottomMenuSizeY;

	ImGuiWindowFlags m_defaultWindowFlags;
	ImGuiViewport* m_mainViewport;

	ObjectSelector m_objectSelector;
	AssetBrowser m_assetBrowser;

	std::shared_ptr<PropertyWindow> m_propertyWindow;
	std::shared_ptr<TerrainEditor> m_terrainEditor;

	std::deque<std::shared_ptr<Undo>> m_undoStack;
};

#endif