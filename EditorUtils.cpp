#include "EditorUtils.h"
#include "Matrix.h"
#include "GameObject.h"
#include "ComponentFactory.h"
#include "ObjectManager.h"
#include "src/IMGUI/ImGuizmo.h"
#include "iRender.h"
#include "Vector2.h"


void EditorUtils::DuplicateGameObject(GameObject* duplicate, GameObject* source)
{
    for (auto* child : source->GetChildren())
    {
        GameObject* duplicateGameObject = ObjectManager::GetInstance()->CreateObject(child->GetName());
        duplicateGameObject->SetParent(duplicate);
        DuplicateGameObject(duplicateGameObject, child);
    }
    for (const auto& component : source->GetComponents())
    {
        duplicate->AddComponent(ComponentFactory::CreateComponent(duplicate, component->GetComponentType(), component));
    }
    duplicate->SetWorldMatrixNoUpdate(source->GetWorldMatrix());
    duplicate->SetLocalMatrixNoUpdate(source->GetLocalMatrix());
}

void EditorUtils::DecomposeMatrix(const Math::Matrix& matrix, Math::Vector3& position, Math::Vector3& rotation, Math::Vector3& scale)
{
    ImGuizmo::DecomposeMatrixToComponents(matrix.Data(), position.Data(), rotation.Data(), scale.Data());
}

void EditorUtils::RecomposeMatrix(Math::Matrix& matrix, const Math::Vector3& position, const Math::Vector3& rotation, const Math::Vector3& scale)
{
    ImGuizmo::RecomposeMatrixFromComponents(position.Data(), rotation.Data(), scale.Data(), matrix.Data());
}

bool EditorUtils::CursorToWorldDirection(Math::Vector3& origin, Math::Vector3& direction)
{
    auto viewProj = Render::GetViewMatrix() * Render::GetProjectionMatrix();

    auto mousePos = ImGui::GetMousePos();
    auto windowSize = Render::GetWindowSize();

    if (mousePos.x > 0 && mousePos.x < windowSize.x && mousePos.y > 0 && mousePos.y < windowSize.y)
    {
        float ndcX = (2.0f * abs(mousePos.x)) / windowSize.x - 1.0f;
        float ndcY = 1.0f - (2.0f * abs(mousePos.y)) / windowSize.y;

        Math::Vector4 screenOrigin(ndcX, ndcY, 0.0f, 1.0);
        Math::Vector4 farPoint(ndcX, ndcY, 1.0f, 1.0);

        Math::Matrix inverseViewproj = viewProj.GetInverse();
        Math::Vector4 rayEnd = farPoint.Transform(inverseViewproj);
        auto ori = screenOrigin.Transform(inverseViewproj);

        origin.x = ori.x;
        origin.y = ori.y;
        origin.z = ori.z;



        direction = (Math::Vector3(rayEnd.x, rayEnd.y, rayEnd.z) - origin).GetNormal();
        return true;
    }
    return false;
}

bool EditorUtils::IsCursorInViewport()
{
    return !ImGuizmo::IsOver() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered();
}

