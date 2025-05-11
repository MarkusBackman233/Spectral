#ifdef EDITOR
#include "pch.cpp"

#include "Editor.h"
#include "src/IMGUI/imgui.h"

#include <iostream>
#include "src/IMGUI/imgui_internal.h"
#include "GameObject.h"
#include "Matrix.h"
#include "ObjectManager.h"
#include "iRender.h"
#include "ShadowManager.h"
#include "ProfilerManager.h"
#include "src/IMGUI/imgui_impl_dx11.h"
#include "src/IMGUI/imgui_impl_win32.h"
#include "RenderManager.h"
#include <ppltasks.h>
#include "IOManager.h"
#include "StringUtils.h"
#include "EditorUtils.h"
#include <ShlObj.h>
#include "MeshComponent.h"
#include "TerrainComponent.h"
#include "Logger.h"
#include "SceneManager.h"
#include "TimeManager.h"
#include "Intersection.h"
#include "DxMathUtils.h"
#include "PropertyWindowFactory.h"
#include "Texture.h"
#include "Mesh.h"
#include "ComponentFactory.h"
#include "UndoTransform.h"
#include "filesystem"
#include "iInput.h"
#include "GuiManager.h"
#include "Vector4.h"
#include "MathFunctions.h"
#include "ResourceManager.h"

using namespace Spectral;

int Editor::ColorPickerMask = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_NoTooltip 
| ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoDragDrop 
| ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_Uint8;

Editor::Editor()
    : m_currentGizmoOperation(ImGuizmo::TRANSLATE)
    , m_currentGizmoMode(ImGuizmo::WORLD)
    , m_useSnap(false)
    , m_started(false)
    , m_windowsOpen(true)
    , m_rightMenuSizeX(400)
    , m_leftMenuSizeX(200)
    , m_defaultImageSize{100,100}
    , m_propertyWindow(nullptr)
{
    ImGuizmo::AllowAxisFlip(false);
    m_defaultWindowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    Logger::Info("Welcome!");
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

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
void Editor::Update(float deltaTime)
{
    m_mainViewport = ImGui::GetMainViewport(); 
    PreRender();
    TopMenu();
    

    RenderManager::GetInstance()->GetGuiManager()->Render();
    //ImGui::SetNextWindowSize(ImVec2(500,500));
    //if (ImGui::Begin("Viewport"))
    //{
    //
    //    ID3D11ShaderResourceView* srv = RenderManager::GetInstance()->GetDeviceResources()->RenderTargetSRV();
    //    ImVec2 imageSize = ImGui::GetContentRegionAvail();
    //    ImGui::Image((void*)srv, imageSize);
    //    ImGui::End();
    //}
    //
    ImGui::SetNextWindowSize(ImVec2(350, 450), ImGuiCond_FirstUseEver);

    bool open = true;

    m_editorCameraController.Update(deltaTime);

    if (!m_started)
    {
        m_objectSelector.HandleRaycastSelection();
        ImGui::ShowStyleEditor();
        ImGui::ShowDemoWindow();
        DrawGrid();
        for (auto* selectedGameObject : m_objectSelector.GetSelectedGameObjects())
        {
            DrawObjectOutline(selectedGameObject, false);
        }

        if (Input::GetKeyHeld(InputId::Control) && Input::GetKeyPressed(InputId::Z))
        {
            if (!m_undoStack.empty())
            {
                m_undoStack.back()->Execute();
                m_undoStack.pop_back();
            }
            
        }

        PropertiesWindow();
        GameObjectsWindow();
        if (m_objectSelector.SelectedGameObject())
        {
            m_objectSelector.HandleSelectedGameObject(this);
        }
    }
    /*
    if (ImGui::Begin("Profiling"))
    {
        bool pauseCollection = ProfilerManager::GetInstance()->IsCollectionPaused();
        ImGui::Checkbox("Pause Profiling", &pauseCollection);
        ProfilerManager::GetInstance()->SetCollectionPaused(pauseCollection);
        for (const auto& [timerName, time] : ProfilerManager::GetInstance()->GetTimers())
        {
            ImGui::Text(std::string("Timer: " + timerName).c_str());
            std::vector<float> timerValues(time.size());
            for (size_t i = 0; i < time.size(); i++)
            {
                timerValues[i] = static_cast<float>(time[i]);
            }

            ImGui::PlotLines("##plot", &timerValues.front(), static_cast<int>(timerValues.size()), 0, "", 0.0f, 80.0f);
            ImGui::SameLine();

            ImGui::Text(std::to_string(time.front()).c_str());

            ImGui::Separator();
        }
        ImGui::End();
    }
    */
    if (m_propertyWindow)
    {
        m_propertyWindow->Update();
    }
}

void Editor::HandleDropFile(const std::filesystem::path& filename)
{
    Logger::Info("Loading drop file: " + filename.filename().string());
    
    for (auto& filetype : IOManager::SupportedTextureFiles)
    {
        if (StringUtils::StringContainsCaseInsensitive(filename.extension().string(), filetype))
        {
            auto DropfileTask = Concurrency::create_task(
                [filename]()
            {
                IOManager::LoadTexture(filename);
            }
            );
            return;
        }
    }    
    
    for (auto& filetype : IOManager::SupportedAudioFiles)
    {
        if (StringUtils::StringContainsCaseInsensitive(filename.extension().string(), filetype))
        {
            auto DropfileTask = Concurrency::create_task(
                [filename]()
            {

                IOManager::LoadAudioSource(filename);
            }
            );
            return;
        }
    }

    for (auto& filetype : IOManager::SupportedMeshFiles)
    {
        if (StringUtils::StringContainsCaseInsensitive(filename.extension().string(), filetype))
        {
            auto DropfileTask = Concurrency::create_task(
                [filename]()
            {
                IOManager::LoadFBX(filename);
            }
            );
            return;
        }
    }
}

void Editor::GameObjectListItem(GameObject* gameObject)
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth | // Makes the whole row clickable/highlightable
        ImGuiTreeNodeFlags_AllowItemOverlap; // Allows icons/buttons on the same line
    if(m_objectSelector.IsGameObjectSelected(gameObject))
    {
        node_flags |= ImGuiTreeNodeFlags_Selected;
    }

    const auto& children = gameObject->GetChildren();

    if (children.empty())
    {
        node_flags |= ImGuiTreeNodeFlags_Leaf;
    }
    bool node_open = ImGui::TreeNodeEx(gameObject, node_flags, gameObject->GetName().c_str());
    if (ImGui::BeginDragDropTarget() && m_objectSelector.SelectedGameObject())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_GAMEOBJECT"))
        {
            m_objectSelector.SelectedGameObject()->SetParent(gameObject);
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
        m_objectSelector.AddSelectedGameObject(gameObject);

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
            GameObjectListItem(childObject);
        }
        ImGui::TreePop();
    }
}

bool Editor::EditTransform(Math::Matrix& matrix)
{
    static float snap[3] = { 1.f, 1.f, 1.f };

    if (!Input::GetKeyHeld(InputId::Mouse2))
    {
        if (Input::GetKeyHeld(InputId::W))
            m_currentGizmoOperation = ImGuizmo::TRANSLATE;
        if (Input::GetKeyHeld(InputId::E))
            m_currentGizmoOperation = ImGuizmo::ROTATE;
        if (Input::GetKeyHeld(InputId::R))
            m_currentGizmoOperation = ImGuizmo::SCALE;        
    }

    const ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    bool isUsingGuizmo = ImGuizmo::Manipulate(
        Render::GetViewMatrix().Data(),
        Render::GetProjectionMatrix().Data(),
        m_currentGizmoOperation, 
        m_currentGizmoMode, 
        matrix.Data(),
        NULL, 
        m_useSnap ? &snap[0] : NULL
    );

    return isUsingGuizmo;
}

void Editor::PropertiesWindow()
{

    ImGui::SetNextWindowPos(ImVec2(m_mainViewport->WorkPos.x + m_mainViewport->WorkSize.x - m_rightMenuSizeX, m_mainViewport->WorkPos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)m_rightMenuSizeX , m_mainViewport->WorkPos.y + m_mainViewport->WorkSize.y ), ImGuiCond_Always);
    ImGui::Begin("Properties Editor", &m_windowsOpen, m_defaultWindowFlags);
    if (ImGui::BeginTabBar("RightTabBar", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Object Properties"))
        {
            if (m_objectSelector.SelectedGameObject() != nullptr)
            {
                std::string objectText = m_objectSelector.SelectedGameObject()->GetName();
                auto cstrText = (char*)objectText.c_str();
                if (ImGui::InputText("##ObjectName", cstrText, 255))
                {
                    m_objectSelector.SelectedGameObject()->SetName(cstrText);
                }
                ImGui::Separator();
                TransformWindow();
                ImGui::Separator();

                GameObjectComponentWindow();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scene"))
        {
            auto& scene = SceneManager::GetInstance()->GetCurrentScene();
            ImGui::Text("Ambient Lighting");
            ImGui::ColorPicker4("Ambient Lighting", scene.GetLightingSettings().AmbientLight.Data(), ColorPickerMask);
            ImGui::DragFloat("Ambient Lighting Intensity", &scene.GetLightingSettings().AmbientLight.w, 0.005f, 0.0f,1.0f);
            ImGui::Separator();
            ImGui::Text("Fog Color");
            ImGui::ColorPicker4("Fog Color", scene.GetLightingSettings().FogColor.Data(), ColorPickerMask);
            ImGui::DragFloat("Fog Intensity", &scene.GetLightingSettings().FogColor.w,0.005f,0.0f,1.0f);


            ImGui::SeparatorText("Shadow Camera");
            ImGui::SliderFloat("Size",      &scene.GetLightingSettings().ShadowCameraSize, 10.0f, 1000.0f);
            ImGui::SliderFloat("Near Depth",&scene.GetLightingSettings().NearDepth,10.0f,1000.0f);
            ImGui::SliderFloat("Far Depth", &scene.GetLightingSettings().FarDepth,10.0f,1000.0f);
            ImGui::SliderFloat("Distance",  &scene.GetLightingSettings().CameraDistance,10.0f,1000.0f);
            ImGui::SeparatorText("General");
            ImGui::SliderFloat("gamma",  &scene.GetLightingSettings().gamma,0.5f,3.0f);

            ImGui::SliderFloat("SSAOIntensity",  &scene.GetLightingSettings().SSAOIntensity,0.0f,10.0f);
            ImGui::SliderFloat("SSAORadius",  &scene.GetLightingSettings().SSAORadius,0.0f,10.0f);
            ImGui::SliderFloat("SSAOBias",  &scene.GetLightingSettings().SSAOBias,0.0f,0.1f);
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
        if (IsStarted())
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
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
                IOManager::LoadSpectralScene("asd.json");
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Create"))
        {
            auto CreateMeshObject = [&](const std::string& gameObjectName, const std::string& meshName)
            {
                auto gameObject = ObjectManager::GetInstance()->CreateObject(gameObjectName);
                gameObject->SetPosition(GetPositionInFontOfCamera(10.0f));
                m_objectSelector.SetSelectedGameObject(gameObject);
                auto meshComponent = ComponentFactory::CreateComponent(gameObject, Component::Type::Mesh);
                std::static_pointer_cast<MeshComponent>(meshComponent)->SetMesh(ResourceManager::GetInstance()->GetResource<Mesh>(meshName));

                gameObject->AddComponent(meshComponent);
            };

            if (ImGui::MenuItem("Cube", ""))
            {
                CreateMeshObject("Cube", "Default Cube");
            }
            if (ImGui::MenuItem("Plane", ""))
            {
                CreateMeshObject("Plane", "Default Plane");
            }
            if (ImGui::MenuItem("Sphere", ""))
            {
                CreateMeshObject("Sphere", "Default Sphere");
            }
            if (ImGui::MenuItem("Terrain", ""))
            {
                auto gameObject = ObjectManager::GetInstance()->CreateObject("Terrain");
                gameObject->SetPosition(GetPositionInFontOfCamera(100.0f));
                m_objectSelector.SetSelectedGameObject(gameObject);
                auto terrainComponent = ComponentFactory::CreateComponent(gameObject, Component::Type::Terrain);
                gameObject->AddComponent(terrainComponent);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Empty GameObject", ""))
            {
                auto newGameObject = ObjectManager::GetInstance()->CreateObject("new GameObject");
                newGameObject->SetPosition(GetPositionInFontOfCamera(10.0f));
                m_objectSelector.SetSelectedGameObject(newGameObject);
            }            
            if (ImGui::MenuItem("Point Light", ""))
            {
                auto newGameObject = ObjectManager::GetInstance()->CreateObject("new Point Light");
                newGameObject->SetPosition(GetPositionInFontOfCamera(10.0f));
                newGameObject->AddComponent(ComponentFactory::CreateComponent(newGameObject,Component::Type::Light));
                m_objectSelector.SetSelectedGameObject(newGameObject);
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View"))
        {
            //if (ImGui::MenuItem("Physics", nullptr, Spectral::GetInstance()->m_renderPhysX))
            //{
            //    Spectral::GetInstance()->m_renderPhysX = !Spectral::GetInstance()->m_renderPhysX;
            //}
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
        if (IsStarted())
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
        ImGui::Text(std::string(" | FPS: " + std::to_string(1.0f / TimeManager::GetDeltaTime())).c_str());


        ImGui::SetCursorPosX(m_mainViewport->WorkSize.x / 2);

        if (!m_started)
        {
            if (ImGui::ArrowButton("PlayId", ImGuiDir_Right))
            {
                m_started = true;
            }
        }
        else if (ImGui::Button("X"))
        {
            m_started = false;
            m_objectSelector.SetSelectedGameObject(nullptr);
        }
        ImGui::EndMainMenuBar();
    }
}

void Editor::GameObjectsWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0, m_mainViewport->WorkPos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)m_leftMenuSizeX, m_mainViewport->WorkPos.y + m_mainViewport->WorkSize.y ), ImGuiCond_Always);
    ImGui::Begin("GameObjects", &m_windowsOpen, m_defaultWindowFlags);
    if (ImGui::BeginTabBar("Stuff"))
    {
        if (ImGui::BeginTabItem("Game Objects"))
        {
            if (ImGui::BeginListBox("##GOList", ImGui::GetContentRegionAvail()))
            {
                auto& gameObjects = ObjectManager::GetInstance()->GetGameObjects();
                for (const auto& gameObject : gameObjects)
                {
                    if (gameObject->GetParent() == nullptr)
                    {
                        GameObjectListItem(gameObject.get());
                    }
                }
                ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, std::max(ImGui::GetContentRegionAvail().y, 100.0f)));
                if (ImGui::BeginDragDropTarget() && m_objectSelector.SelectedGameObject())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_GAMEOBJECT"))
                    {
                        m_objectSelector.SelectedGameObject()->SetParent(nullptr);
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::EndListBox();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Navmeshes"))
        {

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

}

void Editor::GameObjectComponentWindow()
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg,ImVec4(0,0,0,1));

    if (ImGui::BeginListBox("##GOCW", ImGui::GetContentRegionAvail()))
    {
        auto width = ImGui::GetWindowWidth() - 30;

        auto components = m_objectSelector.SelectedGameObject()->GetComponents();
        for (auto it = components.begin(); it != components.end();)
        {
            ImGui::PopStyleColor();
            bool close = true;
            ImGui::PushID(it->get());
            if (ImGui::CollapsingHeader(it->get()->GetComponentName().c_str(), &close, ImGuiTreeNodeFlags_None))
            {
                it->get()->ComponentEditor();
            }
            if (!close)
            {
                m_objectSelector.SelectedGameObject()->RemoveComponent(*it._Unwrapped());
            }
            else
            {
                it++;
            }
            ImGui::PopID();
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 1));
        }
        if (ImGui::Button("Add Component##02", ImVec2(ImGui::GetContentRegionAvail().x, 40)))
        {
            PropertyWindowFactory::SelectComponent(m_objectSelector.SelectedGameObject());
        }
        ImGui::EndListBox();
    }

    ImGui::PopStyleColor();
}

void Editor::StopSimulation()
{
    m_started = false;
}

void Editor::SetPropertyWindow(std::shared_ptr<PropertyWindow> propertyWindow)
{
    m_propertyWindow = propertyWindow;
}

void Editor::AddUndoAction(std::shared_ptr<Undo> undo)
{
    m_undoStack.push_back(undo);
}
void Editor::TransformWindow()
{
    auto selectedObject = m_objectSelector.SelectedGameObject();

    Math::Matrix objectMatrix = selectedObject->GetParent() ? selectedObject->GetLocalMatrix() : selectedObject->GetWorldMatrix();

    std::vector<std::pair<GameObject*, Math::Matrix>> beforeUsed;
    std::vector<std::pair<GameObject*, Math::Matrix>> tempChildObjects;

    auto selectedGameObjectInverseMatrix = selectedObject->GetWorldMatrix().GetInverse();

    for (GameObject* selectedGameObject : m_objectSelector.GetSelectedGameObjects())
    {
        beforeUsed.emplace_back(selectedGameObject, selectedGameObject->GetWorldMatrix());

        if (selectedGameObject->GetId() != selectedObject->GetId() && m_objectSelector.IsAnyParentSelected(selectedGameObject) == false)
        {
            Math::Matrix localMatrix = selectedGameObject->GetWorldMatrix() * selectedGameObjectInverseMatrix;

            tempChildObjects.emplace_back(selectedGameObject, localMatrix);
        }
    }

    Math::Vector3 matrixTranslation, matrixRotation, matrixScale;
    EditorUtils::DecomposeMatrix(objectMatrix, matrixTranslation, matrixRotation, matrixScale);
    if (ImGui::InputFloat3("Position", matrixTranslation.Data(), "%.3f", ImGuiInputTextFlags_NoUndoRedo) ||
        ImGui::InputFloat3("Rotation", matrixRotation.Data(), "%.3f", ImGuiInputTextFlags_NoUndoRedo) ||
        ImGui::InputFloat3("Scale", matrixScale.Data(), "%.3f", ImGuiInputTextFlags_NoUndoRedo))
    {
        AddUndoAction(std::make_shared<UndoTransform>(beforeUsed));

        EditorUtils::RecomposeMatrix(objectMatrix, matrixTranslation, matrixRotation, matrixScale);

        if (selectedObject->GetParent())
        {
            selectedObject->SetWorldMatrix(objectMatrix * selectedObject->GetParent()->GetWorldMatrix());
        }
        else
        {
            selectedObject->SetWorldMatrix(objectMatrix);
        }
        for (auto& [object, localMatrix] : tempChildObjects)
        {
            object->SetWorldMatrix(localMatrix * selectedObject->GetWorldMatrix());
        }
    }
}

void Editor::DrawObjectOutline(GameObject* gameObject, bool isChildOfSelected)
{
    if (gameObject->GetComponentOfType<MeshComponent>())
    {
        auto meshComponent = gameObject->GetComponentOfType<MeshComponent>();
        if (meshComponent->GetMesh())
        {
            auto minVec = meshComponent->GetMesh()->GetBoundingBoxMin();
            auto maxVec = meshComponent->GetMesh()->GetBoundingBoxMax();

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

            for (int i = 0; i < 8; ++i) 
            {
                Math::Vector4 corner4 = gameObject->GetWorldMatrix() * Math::Vector4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
                corners[i].x = corner4.x;
                corners[i].y = corner4.y;
                corners[i].z = corner4.z;
            }

            int lineIndices[12][2] = {
                {0, 1}, {1, 3}, {3, 2}, {2, 0},
                {4, 5}, {5, 7}, {7, 6}, {6, 4},
                {0, 4}, {1, 5}, {2, 6}, {3, 7}
            };

            for (int i = 0; i < 12; ++i) {
                int idx1 = lineIndices[i][0];
                int idx2 = lineIndices[i][1];


                Math::Vector4 color = isChildOfSelected ? Math::Vector4(0.9f, 0.4f, 0.9f, 1.0f) : Math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
                Render::DrawLine(corners[idx1], corners[idx2], color);
            }
        }
    }
    for (auto* child : gameObject->GetChildren())
    {
        DrawObjectOutline(child, true);
    }
}

void Editor::DrawGrid()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    Math::Vector4 color(colors[ImGuiCol_FrameBgHovered].x, colors[ImGuiCol_FrameBgHovered].y, colors[ImGuiCol_FrameBgHovered].z,0.3f);

    int gridSize = 40;
    float scale = 4.0f;

    Math::Vector3 cameraPos(Math::SnapToNearest(Render::GetCameraPosition().Swizzle("xoz"), scale));
    Math::Vector3 offset(-gridSize * 0.5f * scale, 0.0f, -gridSize * 0.5f * scale);
    offset += cameraPos;
    for (int i = 0; i < gridSize + 1; i++)
    {
        Render::DrawLine(Math::Vector3(i * scale, 0, 0) + offset, Math::Vector3(i * scale, 0, gridSize * scale) + offset, color);
    }
    for (int i = 0; i < gridSize + 1; i++)
    {
        Render::DrawLine(Math::Vector3(0, 0, i * scale) + offset, Math::Vector3(gridSize * scale, 0, i * scale) + offset, color);
    }
}

Math::Vector3 Editor::GetPositionInFontOfCamera(float distance)
{
    float minDistance = std::numeric_limits<float>::max();

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
        if (!mesh)
            continue;
        float distance;
        if (Intersection::MeshTriangles(mesh.get(), object->GetWorldMatrix(), Render::GetCameraPosition(), Render::GetCameraDirection(), distance))
        {
            if (distance < minDistance)
            {
                minDistance = distance;
            }
        }
    }

    if (minDistance > distance)
    {
        return Render::GetCameraPosition() + Render::GetCameraDirection() * distance;
    }
    return Render::GetCameraPosition() + Render::GetCameraDirection() * minDistance;
}
#endif
