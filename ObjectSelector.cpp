#include "ObjectSelector.h"
#include "GameObject.h"
#include "iInput.h"
#include "Matrix.h"
#include "ObjectManager.h"
#include "EditorUtils.h"

#include "Intersection.h"
#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "LightComponent.h"
#include "Editor.h"
#include "UndoTransform.h"

using namespace Spectral;

void ObjectSelector::SetSelectedGameObject(GameObject* gameObject)
{
    m_selectedGameObjects.clear();
    if (gameObject)
    {
        m_selectedGameObjects.push_back(gameObject);
    }
}

void ObjectSelector::AddSelectedGameObject(GameObject* gameObject)
{
    if (Input::GetKeyHeld(InputId::Control) == false)
    {
        SetSelectedGameObject(gameObject);
        return;
    }
    if (IsAnyParentSelected(gameObject))
    {
        return;
    }
    if (IsGameObjectSelected(gameObject))
    {
        RemoveSelectedGameObject(gameObject);
        return;
    }
    DeselectAllChildren(gameObject);
    m_selectedGameObjects.push_back(gameObject);
}

void ObjectSelector::RemoveSelectedGameObject(GameObject* gameObject)
{
    auto it = std::find_if(m_selectedGameObjects.begin(), m_selectedGameObjects.end(),
        [&](GameObject* p) { return p->GetId() == gameObject->GetId(); });
    if (it != m_selectedGameObjects.end())
    {
        m_selectedGameObjects.erase(it);
    }
}

bool ObjectSelector::IsGameObjectSelected(GameObject* gameObject)
{
    for (auto* selectedGameObject : m_selectedGameObjects)
    {
        if (selectedGameObject == gameObject)
        {
            return true;
        }
    }
    return false;
}

bool ObjectSelector::IsAnyParentSelected(GameObject* gameObject)
{
    if (gameObject->GetParent() == nullptr)
    {
        return false;
    }

    for (auto* selectedGameObject : GetSelectedGameObjects())
    {
        if (gameObject->GetParent()->GetId() == selectedGameObject->GetId())
        {
            return true;
        }
    }
    return IsAnyParentSelected(gameObject->GetParent());
}

void ObjectSelector::DeselectAllChildren(GameObject* gameObject)
{
    for (GameObject* child : gameObject->GetChildren())
    {
        DeselectAllChildren(child);
        RemoveSelectedGameObject(child);
    }
}

void ObjectSelector::HandleSelectedGameObject(Editor* editor)
{
    if (Input::GetKeyHeld(InputId::Delete))
    {
        for (GameObject* gameObject : m_selectedGameObjects)
        {
            ObjectManager::GetInstance()->Destroy(gameObject);
        }
        m_selectedGameObjects.clear();
        return;
    }
    if (Input::GetKeyHeld(InputId::Control) && Input::GetKeyPressed(InputId::D) && !Input::GetKeyHeld(InputId::Mouse2))
    {
        auto previouslySelectedGameObjects = m_selectedGameObjects;
        m_selectedGameObjects.clear();
        for (GameObject* gameObject : previouslySelectedGameObjects)
        {
            auto duplicateGameObject = ObjectManager::GetInstance()->CreateObject(gameObject->GetName());
            EditorUtils::DuplicateGameObject(duplicateGameObject, gameObject);
            if (gameObject->GetParent())
            {
                duplicateGameObject->SetParent(gameObject->GetParent());
            }
            AddSelectedGameObject(duplicateGameObject);
        }
    }


    std::vector<std::pair<GameObject*, Math::Matrix>> tempChildObjects;

    auto selectedGameObjectInverseMatrix = SelectedGameObject()->GetWorldMatrix().GetInverse();

    for (auto* selectedGameObject : GetSelectedGameObjects())
    {
        if (selectedGameObject->GetId() != SelectedGameObject()->GetId() && IsAnyParentSelected(selectedGameObject) == false)
        {
            Math::Matrix localMatrix = selectedGameObject->GetWorldMatrix() * selectedGameObjectInverseMatrix;

            tempChildObjects.emplace_back(selectedGameObject, localMatrix);
        }
    }
    auto objectMatrix = SelectedGameObject()->GetWorldMatrix();
    if (editor->EditTransform(objectMatrix))
    {
        SelectedGameObject()->SetWorldMatrix(objectMatrix);
    }
    for (auto& [object, localMatrix] : tempChildObjects)
    {
        object->SetWorldMatrix(localMatrix * SelectedGameObject()->GetWorldMatrix());
    }

    static std::vector<std::pair<GameObject*, Math::Matrix>> beforeUsed;
    static bool wasUsing = false;
    if (ImGuizmo::IsUsingAny())
    {
        wasUsing = true;
    }
    else if (wasUsing)
    {
        wasUsing = false;
        editor->AddUndoAction(std::make_unique<UndoTransform>(beforeUsed));
    }
    else if (wasUsing == false)
    {
        beforeUsed.clear();
        for (GameObject* gameOjbect : m_selectedGameObjects)
        {
            beforeUsed.emplace_back(gameOjbect, gameOjbect->GetWorldMatrix());
        }
    }
}

void ObjectSelector::HandleRaycastSelection()
{
    if (!Input::GetKeyPressed(InputId::Mouse1) || !EditorUtils::IsCursorInViewport())
    {
        return;
    }

    Math::Vector3 rayOrigin, rayDirection;
    if (!EditorUtils::CursorToWorldDirection(rayOrigin, rayDirection))
        return;

    float minDistance = std::numeric_limits<float>::max();
    GameObject* currentClosest = nullptr;

    for (const auto& object : ObjectManager::GetInstance()->GetGameObjects())
    {
        std::shared_ptr<Mesh> mesh;
        if (auto meshComponent = object->GetComponentOfType<MeshComponent>())
        {
            mesh = meshComponent->GetMesh();
        }
        else if (auto terrainComponent = object->GetComponentOfType<TerrainComponent>())
        {
            mesh = terrainComponent->GetMesh();
        }
        else if (auto lightComponent = object->GetComponentOfType<LightComponent>())
        {
            float distance = 0.0f;
            if (Intersection::BoundingSphere(object->GetWorldMatrix(), 0.5f, rayOrigin, rayDirection, distance))
            {
                if (distance < 300.0f)
                {
                    AddSelectedGameObject(object.get());
                    return;
                }
            }
        }
        if (!mesh)
            continue;
        float distance = 0.0f;
        if (Intersection::MeshTriangles(mesh.get(), object->GetWorldMatrix(), rayOrigin, rayDirection, distance))
        {
            if (distance < minDistance)
            {
                minDistance = distance;
                currentClosest = object.get();
            }
        }
    }

    if (currentClosest == nullptr)
    {
        SetSelectedGameObject(nullptr);
        return;
    }

    auto objectToSelect = Input::GetKeyHeld(InputId::LAlt) ? currentClosest : currentClosest->GetRootGameObject();
    AddSelectedGameObject(objectToSelect);
}