#include "pch.cpp"

#include "Editor.h"
#include "src/IMGUI/imgui.h"

#include <iostream>
#include "LevelProperties.h"
#include "src/IMGUI/imgui_internal.h"
#include "GameObject.h"
#include "Matrix.h"
#include "ObjectManager.h"
#include "Spectral.h"
#include "ModelManager.h"
#include <filesystem>
#include "TextureManager.h"
#include "Player.h"
#include "ComponentFactory.h"
#include "MaterialManager.h"
#include "iRender.h"
#include "ShadowManager.h"
#include "ProfilerManager.h"
#include "src/IMGUI/imgui_impl_dx11.h"
#include "src/IMGUI/imgui_impl_win32.h"

#include <ppltasks.h>
#include <future>
#include "IOManager.h"
#include "StringUtils.h"
#include "EditorUtils.h"
#include <DirectXCollision.h>
#include <ShlObj.h>

int Editor::ColorPickerMask = ImGuiColorEditFlags_NoAlpha |ImGuiColorEditFlags_NoPicker |ImGuiColorEditFlags_NoOptions |ImGuiColorEditFlags_NoSmallPreview |ImGuiColorEditFlags_NoTooltip 
|ImGuiColorEditFlags_NoLabel |ImGuiColorEditFlags_NoSidePreview |ImGuiColorEditFlags_NoDragDrop 
|ImGuiColorEditFlags_NoBorder |ImGuiColorEditFlags_InputRGB |ImGuiColorEditFlags_DisplayRGB |ImGuiColorEditFlags_Uint8;

Editor::Editor()
    : m_currentGizmoOperation(ImGuizmo::TRANSLATE)
    , m_currentGizmoMode(ImGuizmo::WORLD)
    , m_useSnap(false)
    , m_started(false)
    , m_windowsOpen(true)
    , m_isImporting(false)
    , m_PropertyWindowType(PropertyWindowType_Disabled)
    , m_rightMenuSizeX(400)
    , m_leftMenuSizeX(400)
    , m_defaultImageSize{200,200}
{
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGuizmo::AllowAxisFlip(false);
    //ImFont* font_title = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\trebuc.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    m_defaultWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    m_componentTypes[Component::ComponentType_MeshComponent] = "Mesh Component";
    m_componentTypes[Component::ComponentType_PhysicsComponent] = "Physics Component";
    m_componentTypes[Component::ComponentType_LightComponent] = "Light Component";
    m_componentTypes[Component::ComponentType_ParticleComponent] = "Particle Component";
    m_componentTypes[Component::ComponentType_TerrainComponent] = "Terrain Component";
    LogMessage("Welcome!");

}

Editor::~Editor()
{
}


void Editor::PreRender()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

void Editor::Render()
{
    ImGui::Render();
    {
        auto lockedContext = Render::GetContext();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

void Editor::Update()
{
    HandleRaycastSelection();
    m_mainViewport = ImGui::GetMainViewport(); 
    PreRender();
    TopMenu();
    if (!m_started)
    {
        int gridSize = 20;
        float scale = 4.0f;
        
        Math::Vector3 offset(-gridSize * 0.5f * scale, 0.0f, -gridSize * 0.5f * scale);
        
        for (int i = 0; i < gridSize + 1; i++)
        {
            Render::DrawLine(Math::Vector3(i * scale, 0, 0) + offset, Math::Vector3(i * scale, 0, gridSize * scale) + offset,Math::Vector3(1.0f,0.0f,1.0f, 0.3f));
        }
        for (int i = 0; i < gridSize + 1; i++)
        {
            Render::DrawLine(Math::Vector3(0, 0, i * scale) + offset, Math::Vector3(gridSize * scale, 0, i * scale) + offset, Math::Vector3(1.0f, 0.0f, 1.0f, 0.3f));
        }
        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_Z))
        {
            if (!m_undoStack.empty())
            {
                m_undoStack.back().Execute();
                m_undoStack.pop_back();
            }
            
        }

        PropertiesWindow();
        GameObjectsWindow();
        if (SelectedGameObject())
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Delete))
            {
                ObjectManager::GetInstance()->Destroy(SelectedGameObject());
                SetSelectedGameObject(nullptr);
                return;
            }
            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_D) && !ImGui::IsKeyDown(ImGuiKey_MouseRight))
            {
                auto duplicateGameObject = ObjectManager::GetInstance()->CreateObject(SelectedGameObject()->GetName());
                EditorUtils::DuplicateGameObject(duplicateGameObject, SelectedGameObject());
                duplicateGameObject->SetParent(SelectedGameObject()->GetParent());
                SetSelectedGameObject(duplicateGameObject);
            }



            if (EditTransform(SelectedGameObject()->GetMatrix()))
            {
                SelectedGameObject()->UpdateTransform();
            }

            static Math::Matrix beforeUsed;
            static bool wasUsing = false;
            if (ImGuizmo::IsUsingAny())
            {
                wasUsing = true;
            }
            else if (wasUsing)
            {
                wasUsing = false;

                ChangedItem item{};
                item.gameObject = SelectedGameObject();
                ChangedValue value{};
                value.matrix = beforeUsed;
                
                m_undoStack.push_back(Undo(UndoOperator::UNDO_EDIT_TRANSFORM, item, value));
            }
            else if (wasUsing == false)
            {
                beforeUsed = SelectedGameObject()->GetMatrix();
            }
        }
        PropertyWindow();
        LogWindow();
    }

    if (ShadowManager::GetInstance()->DrawedShadows)
    {
        ImGui::Begin("Shadow Depth Texture");
        ImGui::Image(ShadowManager::GetInstance()->GetShadowTexture().Get(), ImVec2(256,256));
        ImGui::End();
    }

    ImGui::Begin("Timers");
    ImGui::Separator();
    for (const auto& [timerName, time] : ProfilerManager::GetInstance()->GetTimers())
    {
        ImGui::Text(std::string("Timer: " + timerName ).c_str());
        ImGui::Text(std::to_string(time).c_str());
        ImGui::Separator();
    }
    ImGui::End();

    ImportWindow();

    HandleMeshDrawing();
}



void Editor::HandleRaycastSelection()
{
    if (m_started)
    {
        return;
    }

    if (SelectedGameObject() && SelectedGameObject()->GetComponentOfType<MeshComponent>())
    {
        auto meshComponent = SelectedGameObject()->GetComponentOfType<MeshComponent>();
        if (meshComponent->GetMesh())
        {
            auto minVec = meshComponent->GetMesh()->m_minBounds;
            auto maxVec = meshComponent->GetMesh()->m_maxBounds;
    
            Math::Vector3 corners[8] = {
                Math::Vector3(minVec.x, minVec.y, minVec.z),
                Math::Vector3(minVec.x, minVec.y, maxVec.z),
                Math::Vector3(minVec.x, maxVec.y, minVec.z),
                Math::Vector3(minVec.x, maxVec.y, maxVec.z),
                Math::Vector3(maxVec.x, minVec.y, minVec.z),
                Math::Vector3(maxVec.x, minVec.y, maxVec.z),
                Math::Vector3(maxVec.x, maxVec.y, minVec.z),
                Math::Vector3(maxVec.x, maxVec.y, maxVec.z)
            };
    
            for (int i = 0; i < 8; ++i) {
                corners[i] = SelectedGameObject()->GetMatrix() * corners[i];
            }
    
            int lineIndices[12][2] = {
                {0, 1}, {1, 3}, {3, 2}, {2, 0},
                {4, 5}, {5, 7}, {7, 6}, {6, 4},
                {0, 4}, {1, 5}, {2, 6}, {3, 7}
            };
    
            for (int i = 0; i < 12; ++i) {
                int idx1 = lineIndices[i][0];
                int idx2 = lineIndices[i][1];
                Render::DrawLine(corners[idx1], corners[idx2]);
            }
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft,false) && !ImGuizmo::IsOver() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered())
    {
        auto viewProj = Render::GetViewMatrix() * Render::GetProjectionMatrix();

        auto mousePosX = ImGui::GetMousePos().x;
        auto mousePosY = ImGui::GetMousePos().y;
        auto windowSize = Render::GetWindowSize();

        if (mousePosX > 0 && mousePosX < windowSize.x && mousePosY > 0 && mousePosY < windowSize.y)
        {
            float ndcX = (2.0f * abs(mousePosX)) / windowSize.x - 1.0f;
            float ndcY = 1.0f - (2.0f * abs(mousePosY)) / windowSize.y;


            DirectX::XMMATRIX inverseviewproj = DirectX::XMMatrixInverse(nullptr, *static_cast<DirectX::XMMATRIX*>((void*)&viewProj));
            DirectX::XMVECTOR origin = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0);
            DirectX::XMVECTOR farPoint = DirectX::XMVectorSet(ndcX, ndcY, 1.0f, 1.0);
        
            DirectX::XMVECTOR rayorigin = DirectX::XMVector3TransformCoord(origin, inverseviewproj);
            DirectX::XMVECTOR rayend = DirectX::XMVector3TransformCoord(farPoint, inverseviewproj);
            DirectX::XMVECTOR raydirection = DirectX::XMVectorSubtract(rayend, rayorigin);
            raydirection = DirectX::XMVector3Normalize(raydirection);
        
            float minDistance = std::numeric_limits<float>::max();
            GameObject* currentClosest = nullptr;
            for (const auto& object : ObjectManager::GetInstance()->GetGameObjects())
            {
                if (auto meshComponent = object->GetComponentOfType<MeshComponent>())
                {
                    auto mesh = meshComponent->GetMesh();

                    if (!mesh)
                        continue;

                    DirectX::XMMATRIX inverseObjectMatrix = DirectX::XMMatrixInverse(nullptr, *static_cast<DirectX::XMMATRIX*>((void*)&object->GetMatrix()));
                    DirectX::XMVECTOR localRayOrigin = DirectX::XMVector3TransformCoord(rayorigin, inverseObjectMatrix);
                    DirectX::XMVECTOR localRayDirection = DirectX::XMVector3TransformNormal(raydirection, inverseObjectMatrix);
                    localRayDirection = DirectX::XMVector3Normalize(localRayDirection);

                    auto maxVec = meshComponent->GetMesh()->m_minBounds;
                    auto minVec = meshComponent->GetMesh()->m_maxBounds;
                    DirectX::BoundingBox boundingBox;
                    DirectX::BoundingBox::CreateFromPoints(boundingBox, DirectX::XMVectorSet(minVec.x, minVec.y, minVec.z, 1.0), DirectX::XMVectorSet(maxVec.x, maxVec.y, maxVec.z, 1.0));

                    float intersectionDistance = 0.0f;
                    if (!boundingBox.Intersects(localRayOrigin, localRayDirection, intersectionDistance))
                        continue;

                    for (unsigned int i = 0; i < mesh->indices32.size() - 3; i += 3)
                    {
                        int index1 = mesh->indices32[i];
                        int index2 = mesh->indices32[(size_t)i + 1];
                        int index3 = mesh->indices32[(size_t)i + 2];
                        DirectX::XMVECTOR p1 = DirectX::XMVectorSet(mesh->vertexes[index1].position.x, mesh->vertexes[index1].position.y, mesh->vertexes[index1].position.z, 1.0f);
                        DirectX::XMVECTOR p2 = DirectX::XMVectorSet(mesh->vertexes[index2].position.x, mesh->vertexes[index2].position.y, mesh->vertexes[index2].position.z, 1.0f);
                        DirectX::XMVECTOR p3 = DirectX::XMVectorSet(mesh->vertexes[index3].position.x, mesh->vertexes[index3].position.y, mesh->vertexes[index3].position.z, 1.0f);
                        float fakeDistance = 0.0f;
                        if (DirectX::TriangleTests::Intersects(localRayOrigin, localRayDirection, p1, p2, p3, fakeDistance))
                        {
                            auto dxHitPos = DirectX::XMVectorAdd(localRayOrigin, DirectX::XMVectorMultiply(localRayDirection, DirectX::XMVectorSet(fakeDistance, fakeDistance, fakeDistance, fakeDistance)));
                            auto hitPosition = object->GetMatrix() * *static_cast<Math::Vector3*>((void*)&dxHitPos);
                            float distance = (hitPosition - Render::GetCameraPosition()).Length();
                            if (distance < minDistance)
                            {
                                minDistance = distance;
                                currentClosest = object;
                            }
                        }
                    }
                }
            }
            if (currentClosest != nullptr)
            {
                if (ImGui::IsKeyDown(ImGuiKey_LeftAlt))
                {
                    SetSelectedGameObject(currentClosest);
                }
                else
                {
                    SetSelectedGameObject(currentClosest->GetRootGameObject());
                }
            }
        }
    }
}

void Editor::ImportWindow()
{
    if (m_isImporting == false)
    {
        return;
    }

    ImGui::Text(std::string("Importing: " + m_currentImportFilename).c_str());

    switch (m_currentImportFiletype)
    {
    case Editor::ImportFiletype_Mesh:
    {
        bool preserveHierarchy = false;
        ImGui::Checkbox("Preserve hierarchy", &preserveHierarchy);
        break;
    }
    case Editor::ImportFiletype_Texture:
    {
        break;
    }
    }
    if (ImGui::Button("Import"))
    {
        switch (m_currentImportFiletype)
        {
        case Editor::ImportFiletype_Mesh:
        {
            auto DropfileTask = Concurrency::create_task(
                [this]() mutable
            {
                IOManager::LoadFBX(m_currentImportFilename);
                LogMessage("Completed: " + m_currentImportFilename);
            }
            );
            break;
        }
        case Editor::ImportFiletype_Texture:
        {
            IOManager::LoadTexture(m_currentImportFilename);
            LogMessage("Completed: " + m_currentImportFilename);
            break;
        }
        }
        m_isImporting = false;
    }
}

void Editor::HandleDropFile(std::string filename)
{
    static const std::vector<std::string> textureFiletypes{ ".dds" ,".png" ,".jpg", ".jpeg"};

    auto DropfileTask = Concurrency::create_task(
        [filename, this]() mutable
    {
        auto strippedFilename = StringUtils::StripPathFromFilename(filename);

        LogMessage(std::string("Loading: " + strippedFilename));

        if (StringUtils::StringContainsCaseInsensitive(filename, ".fbx"))
        {
            //m_currentImportFilename = filename;
            //m_currentImportFiletype = ImportFiletype::ImportFiletype_Mesh;
            //m_isImporting = true;
            IOManager::LoadFBX(filename);
            LogMessage("Completed: " + filename);
            return;
        }

        for (auto& filetype : textureFiletypes)
        {
            if (StringUtils::StringContainsCaseInsensitive(filename, filetype))
            {
                IOManager::LoadTexture(filename);
                LogMessage("Completed: " + filename);
                return;
            }
        }
    }
    );
}

void Editor::GameObjectListItem(GameObject* gameObject)
{
    constexpr ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

    ImGuiTreeNodeFlags node_flags = base_flags;
    for (auto* selectedGameObject : m_selectedGameObjects)
    {
        if (gameObject == selectedGameObject)
        {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }
    }

    if (gameObject->GetChildren().empty())
    {
        node_flags |= ImGuiTreeNodeFlags_Leaf;
    }

    bool node_open = ImGui::TreeNodeEx(gameObject, node_flags, gameObject->GetName().c_str());
    if (ImGui::BeginDragDropTarget() && SelectedGameObject())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_GAMEOBJECT"))
        {
            if(SelectedGameObject()->GetParent() != nullptr)
            {
                SelectedGameObject()->SetParent(nullptr);
            }
            SelectedGameObject()->SetParent(gameObject);
        }

        ImGui::EndDragDropTarget();
    }
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("_GAMEOBJECT", NULL, 0);
        ImGui::Text("Child to object");
        ImGui::EndDragDropSource();
    }

    if (ImGui::IsItemClicked())
    {
        ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ? AddSelectedGameObject(gameObject) : SetSelectedGameObject(gameObject);
    }
    if (node_open)
    {
        for (auto& childObject : gameObject->GetChildren())
        {
            GameObjectListItem(childObject);
        }
        ImGui::TreePop();
    }
}


bool Editor::EditTransform(Math::Matrix& matrix)
{
    static float snap[3] = { 1.f, 1.f, 1.f };

    if (!ImGui::IsKeyDown(ImGuiKey_MouseRight))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_W))
            m_currentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E))
            m_currentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) 
            m_currentGizmoOperation = ImGuizmo::SCALE;        
    }

    const ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    float model[16]{};
    float view[16]{};
    float projection[16]{};

    EditorUtils::MatrixToFloatMatrix(matrix, model);
    EditorUtils::MatrixToFloatMatrix(Render::GetViewMatrix(), view);
    EditorUtils::MatrixToFloatMatrix(Render::GetProjectionMatrix(), projection);
    bool isUsingGuizmo = ImGuizmo::Manipulate(view, projection, m_currentGizmoOperation, m_currentGizmoMode, model, NULL, m_useSnap ? &snap[0] : NULL);
    
    if (isUsingGuizmo)
    {
        EditorUtils::FloatMatrixToMatrix(model, matrix);
        return true;
    }

    return false;
}


void Editor::LogMessage(std::string message)
{
    static std::mutex mutex;
    mutex.lock();
    std::cout << message << "\n";
    m_logBuffer.push_back(message);
    while (m_logBuffer.size() > 250)
        m_logBuffer.pop_front();
    mutex.unlock();
}

void Editor::PropertiesWindow()
{
    ImGui::SetNextWindowPos(ImVec2(m_mainViewport->WorkPos.x + m_mainViewport->WorkSize.x - m_rightMenuSizeX, m_mainViewport->WorkPos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)m_rightMenuSizeX, m_mainViewport->WorkPos.y + m_mainViewport->WorkSize.y), ImGuiCond_Always);
    ImGui::Begin("Properties Editor", &m_windowsOpen, m_defaultWindowFlags);

    if (ImGui::BeginTabBar("RightTabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Object Properties"))
        {
            if (SelectedGameObject() != nullptr)
            {
                auto objectText = SelectedGameObject()->GetName();
                auto cstrText = (char*)objectText.c_str();
                if (ImGui::InputText("##ObjectName", cstrText, 255))
                {
                    SelectedGameObject()->SetName(cstrText);
                }
                ImGui::Separator();
                TransformWindow();
                ImGui::Separator();

                GameObjectComponentWindow();

                if (ImGui::Button("Add Component##02", ImVec2((float)m_rightMenuSizeX, 40)))
                {
                    m_PropertyWindowType = PropertyWindowType_Component;
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene Settings"))
        {
            float* ambientLight = &Spectral::GetInstance()->GetAmbientLight().r;
            ImGui::Text("Ambient Lighting");
            ImGui::ColorPicker4("Ambient Lighting", ambientLight, ColorPickerMask);
            ImGui::DragFloat("Ambient Lighting Intensity", &Spectral::GetInstance()->GetAmbientLight().a);
            ImGui::Separator();
            float* fogColor = &Spectral::GetInstance()->GetFogColor().r;
            ImGui::Text("Fog Color");
            ImGui::ColorPicker4("Fog Color", fogColor, ColorPickerMask);
            ImGui::DragFloat("Fog Intensity", &Spectral::GetInstance()->GetFogColor().a);

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void Editor::TopMenu()
{

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {
            }            
            if (ImGui::MenuItem("Save"))
            {
                IOManager::SaveSpectralScene("asd");
            }
            if (ImGui::MenuItem("Load"))
            {
                
                IOManager::LoadSpectralScene("asd.sps");
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("New GameObject", ""))
            {
                auto newGameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");
                SetSelectedGameObject(newGameObject);
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Physics", nullptr, Spectral::GetInstance()->m_renderPhysX))
            {
                Spectral::GetInstance()->m_renderPhysX = !Spectral::GetInstance()->m_renderPhysX;
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Translate", "W", m_currentGizmoOperation == ImGuizmo::TRANSLATE))
            {
                m_currentGizmoOperation = ImGuizmo::TRANSLATE;
            }
            if (ImGui::MenuItem("Rotate", "E", m_currentGizmoOperation == ImGuizmo::ROTATE))
            {
                m_currentGizmoOperation = ImGuizmo::ROTATE;
            }
            if (ImGui::MenuItem("Scale", "R", m_currentGizmoOperation == ImGuizmo::SCALE))
            {
                m_currentGizmoOperation = ImGuizmo::SCALE;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("World", "", m_currentGizmoMode == ImGuizmo::WORLD))
            {
                m_currentGizmoMode = ImGuizmo::WORLD;
            }
            if (ImGui::MenuItem("Local", "", m_currentGizmoMode == ImGuizmo::LOCAL))
            {
                m_currentGizmoMode = ImGuizmo::LOCAL;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Snap", "", m_useSnap == true))
            {
                m_useSnap = !m_useSnap;
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        ImGui::Text(std::string(" | FPS: " + std::to_string(1.0f / Spectral::GetInstance()->GetDeltaTime())).c_str());


        ImGui::SetCursorPosX(m_mainViewport->WorkSize.x / 2);

        if (!m_started)
        {
            if (ImGui::ArrowButton("PlayId", ImGuiDir_Right))
            {
                m_started = true;
                for (auto& gameObject : ObjectManager::GetInstance()->GetGameObjects())
                {
                    for (auto& component : gameObject->GetComponents())
                    {
                        component->Start();
                    }
                }
            }
        }
        else if (ImGui::Button("X"))
        {
            m_started = false;
        }
        ImGui::EndMainMenuBar();
    }
}

void Editor::GameObjectsWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0, m_mainViewport->WorkPos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)m_leftMenuSizeX, m_mainViewport->WorkPos.y + m_mainViewport->WorkSize.y - 200), ImGuiCond_Always);
    ImGui::Begin("GameObjects", &m_windowsOpen, m_defaultWindowFlags);

    if (ImGui::BeginListBox("##GameObjectsList", ImVec2((float)m_leftMenuSizeX, m_mainViewport->WorkPos.y + m_mainViewport->WorkSize.y - 200)))
    {
        for (const auto gameObject : ObjectManager::GetInstance()->GetGameObjects())
        {
            if (gameObject->GetParent() == nullptr)
            {
                GameObjectListItem(gameObject);
            }
        }

        ImGui::EndListBox();
    }
    ImGui::End();
}

void Editor::LogWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0, m_mainViewport->WorkPos.y + m_mainViewport->WorkSize.y - 178 ), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(m_mainViewport->WorkSize.x - (float)m_rightMenuSizeX,  200), ImGuiCond_Always);
    ImGui::Begin("Debug Console", &m_windowsOpen, m_defaultWindowFlags);
    ImGui::Separator();
    if (ImGui::BeginTabBar("BottomTabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("File Explorer"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debug Console"))
        {
            auto size = ImGui::GetWindowSize();
            size.y -= 50;
            if (ImGui::BeginListBox("", size))
            {
                for (const auto& logMessage : m_logBuffer)
                {
                    ImGui::Text(logMessage.c_str());
                    ImGui::Separator();
                }
                ImGui::EndListBox();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void Editor::GameObjectComponentWindow()
{
    for (const auto& component : SelectedGameObject()->GetComponents())
    {
        if (ImGui::CollapsingHeader(component->GetComponentName().c_str()))
        {
            component->ComponentEditor();
        } 
        ImGui::Separator();
    }
}

void Editor::OpenPropertyWindow(PropertyWindowType propertyType)
{
    m_PropertyWindowType = m_PropertyWindowType == propertyType ? PropertyWindowType_Disabled : propertyType;
}

void Editor::SetSelectedGameObject(GameObject* gameObject)
{
    if (gameObject != SelectedGameObject())
    {
        ChangedItem item{};
        item.gameObject = SelectedGameObject();
        ChangedValue value{};

        m_undoStack.push_back(Undo(UndoOperator::UNDO_CHANGE_SELECTION, item, value));
    }
    m_selectedGameObjects.clear();
    if (gameObject)
    {
        m_selectedGameObjects.push_back(gameObject);
    }
}

void Editor::AddSelectedGameObject(GameObject* gameObject)
{
    for (auto* selectedGameObject : m_selectedGameObjects)
    {
        if (selectedGameObject == gameObject)
        {
            return;
        }
    }
    m_selectedGameObjects.push_back(gameObject);
}

void Editor::PropertyWindow()
{
    if (m_PropertyWindowType == PropertyWindowType_Disabled || !SelectedGameObject())
    {
        return;
    }

    bool isWindowOpen = true;
    if (!ImGui::Begin("Select", &isWindowOpen, ImGuiWindowFlags_NoCollapse))
    {
        return;
    }
    if (isWindowOpen == false)
    {
        m_PropertyWindowType = PropertyWindowType_Disabled;
    }

    const ImVec2 defaultButtonSize(ImGui::GetCurrentWindow()->Size.x, 30);

    switch (m_PropertyWindowType)
    {
        case PropertyWindowType_Texture:
        case PropertyWindowType_Normal:
        case PropertyWindowType_Roughness:
        case PropertyWindowType_Metallic:
        case PropertyWindowType_Ao:
        {
            if (!Editor::GetInstance()->SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh())
            {
                m_PropertyWindowType = PropertyWindowType_Disabled;
                return;
            }
            int columnsPerRow = std::clamp((int)std::floor(ImGui::GetCurrentWindow()->Size.x / (m_defaultImageSize.x + 15)), 1, 20);
            static char searchedString[128] = "";

            std::string selectedTextureName = "";
            if (m_PropertyWindowType == PropertyWindowType_Texture)
            {
                selectedTextureName = StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(Editor::GetInstance()->SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh()->GetMaterial()->GetTexture(ALBEDO)));
            }
            else if (m_PropertyWindowType == PropertyWindowType_Normal)
            {
                selectedTextureName = StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(Editor::GetInstance()->SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh()->GetMaterial()->GetTexture(NORMAL)));
            }            
            else if (m_PropertyWindowType == PropertyWindowType_Roughness)
            {
                selectedTextureName = StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(Editor::GetInstance()->SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh()->GetMaterial()->GetTexture(ROUGHNESS)));
            }

            ImGui::PushItemWidth(300);
            ImGui::InputText("Search", searchedString, IM_ARRAYSIZE(searchedString));
            ImGui::PopItemWidth();
            ImGui::Separator();
            if (ImGui::BeginTable("##TextureSelector", columnsPerRow, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit))
            {
                for (const auto& [textureName, texture] : TextureManager::GetInstance()->GetCachedTextures())
                {
                    if (!StringUtils::StringContainsCaseInsensitive(textureName, searchedString))
                    {
                        continue;
                    }
                    
                    auto bgColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    if (selectedTextureName == StringUtils::StripPathFromFilename(textureName))
                    {
                        bgColor = ImVec4(1.0f, 0.3f, 1.0f, 1.0f);
                    }

                    ImGui::TableNextColumn();
                    if (ImGui::ImageButton(texture->GetResourceView().Get(), m_defaultImageSize,ImVec2(0,0),ImVec2(1,1), -1, ImVec4(0, 0, 0, 0), bgColor))
                    {
                        auto material = SelectedGameObject()->GetComponentOfType<MeshComponent>()->GetMesh()->GetMaterial();


                        switch (m_PropertyWindowType)
                        {
                        case Editor::PropertyWindowType_Texture:
                            material->SetTexture(ALBEDO, texture);
                            break;
                        case Editor::PropertyWindowType_Normal:
                            material->SetTexture(NORMAL, texture);
                            break;
                        case Editor::PropertyWindowType_Roughness:
                            material->SetTexture(ROUGHNESS, texture);
                            break;
                        case Editor::PropertyWindowType_Metallic:
                            material->SetTexture(METALLIC, texture);
                            break;
                        case Editor::PropertyWindowType_Ao:
                            material->SetTexture(AO, texture);
                            break;
                        }

                        IOManager::SaveSpectralMaterial(material);
                        m_PropertyWindowType = PropertyWindowType_Disabled;
                    }

                    auto filename = StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(texture));
                    auto textWidth = ImGui::CalcTextSize(filename.c_str()).x;
                    
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (m_defaultImageSize.x + 15 - textWidth) * 0.5f);
                    ImGui::Text(filename.c_str());
                    ImGui::SetCursorPosX(0);

                }
                ImGui::EndTable();
            }
            break;
        }
        case PropertyWindowType_Mesh:
        {
            for (const auto& [meshName, mesh] : ModelManager::GetInstance()->GetCachedMeshes())
            {
                if (ImGui::Button(StringUtils::StripPathFromFilename(meshName).c_str(), defaultButtonSize))
                {
                    SelectedGameObject()->GetComponentOfType<MeshComponent>()->SetMesh(mesh);
                    m_PropertyWindowType = PropertyWindowType_Disabled;
                }
            }
            break;
        }        
        case PropertyWindowType_Component:
        {
            for (const auto& [componentId, componentName] : m_componentTypes)
            {
                if (ImGui::Button(componentName.c_str(), defaultButtonSize))
                {
                    SelectedGameObject()->AddComponent(ComponentFactory::CreateComponent(SelectedGameObject(),static_cast<Component::ComponentType>(componentId)));
                    m_PropertyWindowType = PropertyWindowType_Disabled;
                }
            }
            break;
        }        
    }
    ImGui::End();
}



void Editor::HandleMeshDrawing()
{
    ImGui::Begin("Mesh Painter");
    static GameObject* selectedBrushObject;
    static GameObject* selectedCanvasObject;

    if (ImGui::Button("Set selected object as brush"))
    {
        selectedBrushObject = SelectedGameObject();
    }

    if (ImGui::Button("Set selected object as canvas"))
    {
        selectedCanvasObject = SelectedGameObject();
    }

    ImGui::Text(std::string("Selected brush: " + (selectedBrushObject ? selectedBrushObject->GetName() : "Not Selected")).c_str());
    ImGui::Text(std::string("Selected canvas: " + (selectedCanvasObject ? selectedCanvasObject->GetName() : "Not Selected")).c_str());
    static bool randomYRotation = false;
    ImGui::Checkbox("Use random Y rotation?", &randomYRotation);

    if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && ImGui::IsKeyPressed(ImGuiKey_MouseLeft, true) && selectedBrushObject && !ImGuizmo::IsOver() && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) && !ImGui::IsAnyItemHovered())
    {
        Math::Vector3 hitPosition;
        Math::Vector3 hitNormal;
        bool hitMesh = false;

        auto viewProj = Render::GetViewMatrix() * Render::GetProjectionMatrix();

        auto mousePosX = ImGui::GetMousePos().x;
        auto mousePosY = ImGui::GetMousePos().y;
        auto windowSize = Render::GetWindowSize();

        if (selectedCanvasObject && mousePosX > 0 && mousePosX < windowSize.x && mousePosY > 0 && mousePosY < windowSize.y)
        {
            float ndcX = (2.0f * abs(mousePosX)) / windowSize.x - 1.0f;
            float ndcY = 1.0f - (2.0f * abs(mousePosY)) / windowSize.y;

            DirectX::XMMATRIX inverseviewproj = DirectX::XMMatrixInverse(nullptr, *static_cast<DirectX::XMMATRIX*>((void*)&viewProj));
            DirectX::XMVECTOR origin = DirectX::XMVectorSet(ndcX, ndcY, 0.0f, 1.0);
            DirectX::XMVECTOR farPoint = DirectX::XMVectorSet(ndcX, ndcY, 1.0f, 1.0);

            DirectX::XMVECTOR rayorigin = DirectX::XMVector3TransformCoord(origin, inverseviewproj);
            DirectX::XMVECTOR rayend = DirectX::XMVector3TransformCoord(farPoint, inverseviewproj);
            DirectX::XMVECTOR raydirection = DirectX::XMVectorSubtract(rayend, rayorigin);
            raydirection = DirectX::XMVector3Normalize(raydirection);

            float minDistance = std::numeric_limits<float>::max();

            if (auto meshComponent = selectedCanvasObject->GetComponentOfType<MeshComponent>())
            {
                auto mesh = meshComponent->GetMesh();
                DirectX::XMMATRIX inverseObjectMatrix = DirectX::XMMatrixInverse(nullptr, *static_cast<DirectX::XMMATRIX*>((void*)&selectedCanvasObject->GetMatrix()));
                DirectX::XMVECTOR localRayOrigin = DirectX::XMVector3TransformCoord(rayorigin, inverseObjectMatrix);
                DirectX::XMVECTOR localRayDirection = DirectX::XMVector3TransformNormal(raydirection, inverseObjectMatrix);
                localRayDirection = DirectX::XMVector3Normalize(localRayDirection);
                for (unsigned int i = 0; i < mesh->indices32.size() - 3; i += 3)
                {
                    int index1 = mesh->indices32[i];
                    int index2 = mesh->indices32[(size_t)i + 1];
                    int index3 = mesh->indices32[(size_t)i + 2];
                    DirectX::XMVECTOR p1 = DirectX::XMVectorSet(mesh->vertexes[index1].position.x, mesh->vertexes[index1].position.y, mesh->vertexes[index1].position.z, 1.0f);
                    DirectX::XMVECTOR p2 = DirectX::XMVectorSet(mesh->vertexes[index2].position.x, mesh->vertexes[index2].position.y, mesh->vertexes[index2].position.z, 1.0f);
                    DirectX::XMVECTOR p3 = DirectX::XMVectorSet(mesh->vertexes[index3].position.x, mesh->vertexes[index3].position.y, mesh->vertexes[index3].position.z, 1.0f);

                    float fakeDistance = 0.0f;
                    if (DirectX::TriangleTests::Intersects(localRayOrigin, localRayDirection, p1, p2, p3, fakeDistance))
                    { // i need to make this much better
                        auto dxVec = DirectX::XMVectorAdd(localRayOrigin, DirectX::XMVectorMultiply(localRayDirection, DirectX::XMVectorSet(fakeDistance, fakeDistance, fakeDistance, fakeDistance)));
                        hitPosition = selectedCanvasObject->GetMatrix() *  *static_cast<Math::Vector3*>((void*)&dxVec);
                        
                        Triangle triangle(Math::Vector3(p3.m128_f32[0], p3.m128_f32[1], p3.m128_f32[2],1.0f),
                            Math::Vector3(p2.m128_f32[0], p2.m128_f32[1], p2.m128_f32[2], 1.0f),
                            Math::Vector3(p3.m128_f32[0], p3.m128_f32[1], p3.m128_f32[2], 1.0f));
                        hitNormal = triangle.CalculateNormal();
                        hitMesh = true;
                    }
                }
            }
        }
        if (hitMesh)
        {
            auto duplicateGameObject = ObjectManager::GetInstance()->CreateObject(selectedBrushObject->GetName());
            EditorUtils::DuplicateGameObject(duplicateGameObject, selectedBrushObject);
            duplicateGameObject->SetParent(nullptr);
            //auto scale = duplicateGameObject->GetMatrix().GetScale();
            //
            //Math::Matrix translationMatrix;
            //translationMatrix.SetPosition(hitPosition);
            //Math::Matrix rotationMatrix;
            //Math::Vector3 up    = hitNormal * scale.y;
            //Math::Vector3 right = up.Cross(Math::Vector3(0.0f, 1.0f, 0.0f, 0.0)).GetNormal() * scale.x;
            //Math::Vector3 front = right.Cross(up).GetNormal() * scale.z;
            //
            //
            //rotationMatrix.SetUp(up);
            //rotationMatrix.SetRight(right);
            //rotationMatrix.SetFront(front);
            //
            //duplicateGameObject->GetMatrix() = rotationMatrix * translationMatrix;
            //duplicateGameObject->UpdateTransform();
            duplicateGameObject->GetMatrix().SetPosition(hitPosition);
            if (randomYRotation)
            {
                float circleInRad = 6.2831853f;
                duplicateGameObject->GetMatrix() = Math::Matrix::MakeRotationZ(static_cast <float> (rand()) / static_cast <float> (RAND_MAX) * circleInRad) * duplicateGameObject->GetMatrix();
            }
            duplicateGameObject->UpdateTransform();
            SetSelectedGameObject(duplicateGameObject);
        }

    }

    ImGui::End();
}

void Editor::TransformWindow()
{
    Math::Matrix* objectMatrix = SelectedGameObject()->GetParent() ? &SelectedGameObject()->GetLocalMatrix() : &SelectedGameObject()->GetMatrix();

    Math::Vector3 matrixTranslation, matrixRotation, matrixScale;
    EditorUtils::DecomposeMatrix(*objectMatrix, matrixTranslation, matrixRotation, matrixScale);
    if (ImGui::InputFloat3("Position", (float*)&matrixTranslation) ||
        ImGui::InputFloat3("Rotation", (float*)&matrixRotation) ||
        ImGui::InputFloat3("Scale", (float*)&matrixScale))
    {
        { // This will not work good
            ChangedItem item{};
            item.gameObject = SelectedGameObject();
            ChangedValue value{};
            value.matrix = *objectMatrix;
        
            m_undoStack.push_back(Undo(UndoOperator::UNDO_EDIT_TRANSFORM, item, value));
        }

        EditorUtils::RecomposeMatrix(*objectMatrix, matrixTranslation, matrixRotation, matrixScale);

        if (SelectedGameObject()->GetParent())
        {
            SelectedGameObject()->GetMatrix() = SelectedGameObject()->GetLocalMatrix() * SelectedGameObject()->GetParent()->GetMatrix();
        }
        SelectedGameObject()->UpdateTransform();
    }
}
