#pragma once
#ifdef EDITOR
#include "pch.h"
class Mesh;
class MeshComponent;
class Texture;
class DefaultMaterial;
class GameObject;
class Script;
class ScriptComponent;
class AudioSourceComponent;


class PropertyWindowFactory
{
public:
	static void SelectMesh(std::shared_ptr<Mesh>& selectMesh);
	static void SelectTexture(std::shared_ptr<DefaultMaterial>& selectMaterial, int selectTextureId, const std::string& currentSelectedTextureName = "");
	static void SelectMaterial(std::shared_ptr<DefaultMaterial>& material);
	static void SelectComponent(GameObject* gameObject);
	static void SelectScript(ScriptComponent* selectScriptComponent);
	static void SelectAudioSource(AudioSourceComponent* audioSourceComponent);
};

#endif