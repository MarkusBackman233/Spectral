#ifdef EDITOR
#include "PropertyWindowFactory.h"
#include "MeshComponent.h"
#include "Editor.h"
#include "MeshPropertyWindow.h"
#include "TexturePropertyWindow.h"
#include "MaterialPropertyWindow.h"
#include "ComponentPropertyWindow.h"
#include "ScriptPropertyWindow.h"
#include "AudioSourcePropertyWindow.h"
#include "Texture.h"
#include "Material.h"
#include "ComponentFactory.h"
#include "GameObject.h"
#include "ScriptComponent.h"
#include "AudioSourceComponent.h"

void PropertyWindowFactory::SelectMesh(std::shared_ptr<Mesh>& selectMesh)
{
    Editor::GetInstance()->SetPropertyWindow(
        std::make_shared<MeshPropertyWindow>(
            [&selectMesh](std::shared_ptr<Mesh> mesh)
        {
            selectMesh = mesh;
        }
    ));
}

void PropertyWindowFactory::SelectTexture(std::shared_ptr<Material>& selectMaterial, int selectTextureId, const std::string& currentSelectedTextureName /* = "" */)
{
    Editor::GetInstance()->SetPropertyWindow(
        std::make_shared<TexturePropertyWindow>(
            [selectMaterial, selectTextureId](std::shared_ptr<Texture> texture)
    {
        selectMaterial->SetTexture(selectTextureId, texture);
    },
    currentSelectedTextureName
    ));
}

void PropertyWindowFactory::SelectMaterial(std::shared_ptr<Material>& material)
{
    Editor::GetInstance()->SetPropertyWindow(
        std::make_shared<MaterialPropertyWindow>(
            [&material](std::shared_ptr<Material> selectedMaterial)
    {
        material = selectedMaterial;
    }
    ));
}

void PropertyWindowFactory::SelectComponent(GameObject* gameObject)
{
    Editor::GetInstance()->SetPropertyWindow(
        std::make_shared<ComponentPropertyWindow>(
            [gameObject](Component::Type type)
    {
        gameObject->AddComponent(ComponentFactory::CreateComponent(gameObject, type));
    }
    ));
}

void PropertyWindowFactory::SelectAudioSource(AudioSourceComponent* audioSourceComponent)
{
    Editor::GetInstance()->SetPropertyWindow(
        std::make_shared<AudioSourcePropertyWindow>(
            [audioSourceComponent](std::shared_ptr<AudioSource> audioSource)
    {
        audioSourceComponent->SetAudioSource(audioSource);
    }
    ));
}

void PropertyWindowFactory::SelectScript(ScriptComponent* selectScriptComponent)
{
    Editor::GetInstance()->SetPropertyWindow(
        std::make_shared<ScriptPropertyWindow>(
            [selectScriptComponent](std::shared_ptr<Script> script)
    {
        selectScriptComponent->SetScript(script);
    }
        ));
}
#endif