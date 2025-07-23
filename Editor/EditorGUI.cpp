#include "EditorGUI.h"
#include <src/IMGUI/imgui.h>
#include <GameObject.h>
#include <ObjectManager.h>

EditorGUI::EditorGUI()
{

}



void EditorGUI::Update()
{
	ImGui::ShowDemoWindow();

	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
	if (ImGui::Begin("Object Browser"))
	{
		ImGui::Text("Hello");
		ImGui::End();
	}


    if (ImGui::Begin("GameObjects"))
    {
        static ImGuiTextFilter filter;
        filter.Draw();



        if (ImGui::BeginListBox("##GOList", ImGui::GetContentRegionAvail()))
        {
            auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();
            for (const auto& gameObject : gameObjects)
            {
                if (gameObject->GetParent() == nullptr)
                    GameObjectListItem(gameObject.get(), filter);
            }
            ImGui::EndListBox();
        }
        ImGui::End();
    }
}


void EditorGUI::GameObjectListItem(GameObject* gameObject, const ImGuiTextFilter& filter)
{
    static bool unchildHovered = false;
    if (!filter.PassFilter(gameObject->GetName().c_str()))
    {
        return;
    }
    if (gameObject->GetParent() == nullptr)
    {
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImVec2 regionSize = ImVec2(ImGui::GetContentRegionAvail().x, 4.0f); // thin line
        ImGui::InvisibleButton(("##dropzone_before_" + std::to_string(gameObject->GetId())).c_str(), regionSize);
        if (ImGui::BeginDragDropTarget())
        {

            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_GAMEOBJECT"))
            {
                GameObject* dragSource = *static_cast<GameObject* const*>(payload->Data);;
                dragSource->SetParent(nullptr);
            }
            ImGui::EndDragDropTarget();
        }

        // Optional visual for drop zone
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            ImU32 color = IM_COL32(255, 255, 0, 128);
            ImGui::GetWindowDrawList()->AddRectFilled(cursorPos, ImVec2(cursorPos.x + regionSize.x, cursorPos.y + regionSize.y), color);
            unchildHovered = true;
        }
    }

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |ImGuiTreeNodeFlags_OpenOnDoubleClick |ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap;


    const auto& children = gameObject->GetChildren();
    if (children.empty())
    {
        node_flags |= ImGuiTreeNodeFlags_Leaf;
    }

   // ImVec2 min = ImGui::GetCursorScreenPos();
   // ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + ImGui::GetTextLineHeightWithSpacing()-4);
   //
   // // Draw colored background before the TreeNode
   // ImU32 bgColor = IM_COL32(255, 30, 30, 60);  // RGBA: light blue, semi-transparent
   // ImGui::GetWindowDrawList()->AddRectFilled(min, max, bgColor);
    bool node_open = ImGui::TreeNodeEx(gameObject, node_flags, gameObject->GetName().c_str());
    if (ImGui::BeginDragDropTarget())
    {

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_GAMEOBJECT"))
        {
            GameObject* dragSource = *static_cast<GameObject* const*>(payload->Data);;
            GameObject* dragDestination = gameObject;

            if (!dragDestination->HasGameObjectInParentHierarchy(dragSource))
            {
                //AddUndoAction(std::make_shared<UndoSetParent>(m_objectSelector.SelectedGameObject(), m_objectSelector.SelectedGameObject()->GetParent()));
                dragDestination->SetParent(dragSource);
            }
        }
        ImGui::EndDragDropTarget();
    }
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_GAMEOBJECT", &gameObject, sizeof(GameObject*));
        if (unchildHovered)
        {
            ImGui::Text("Unchild object");
        }
        else
        {
            ImGui::Text("Child to object");
        }
        ImGui::EndDragDropSource();
    }

    if (ImGui::IsItemClicked())
    {
        //m_objectSelector.AddSelectedGameObject(gameObject);
    }

    const auto& components = gameObject->GetComponents();

    for (auto& component : components)
    {
        component->DisplayComponentIcon();
    }

    if (node_open)
    {
        for (auto& childObject : children)
        {
            GameObjectListItem(childObject, filter);
        }
        ImGui::TreePop();
    }
}
