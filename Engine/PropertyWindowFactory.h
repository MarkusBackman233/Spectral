#pragma once
#ifdef EDITOR
#include "pch.h"
class Mesh;
class Texture;
class Material;
class GameObject;
class Script;
class ScriptComponent;

class PropertyWindowFactory
{
public:
	static void SelectMesh(std::shared_ptr<Mesh>& selectMesh);
	static void SelectTexture(std::shared_ptr<Material>& selectMaterial, int selectTextureId, const std::string& currentSelectedTextureName = "");
	static void SelectMaterial(std::shared_ptr<Mesh>& selectMesh);
	static void SelectComponent(GameObject* gameObject);
	static void SelectScript(ScriptComponent* selectScriptComponent);
};

#endif