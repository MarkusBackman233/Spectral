#include "IOManager.h"
#include <iostream>
#include "Mesh.h"
#include "iRender.h"
#include "ObjectManager.h"
#include "Logger.h"
#include "Matrix.h"
#include <filesystem>
#include "ComponentFactory.h"
#include <ppltasks.h>
#include "StringUtils.h"
#include "SceneManager.h"
#include "Texture.h"
#include "DefaultMaterial.h"
#include "Editor.h"

#include "MeshComponent.h"
#include "EditorUtils.h"
#include "DefaultAssets.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp\scene.h>

#include <algorithm>
#include <execution>

#include "EditorCameraController.h"
#include "ProjectBrowserManager.h"

#include "AudioManager.h"
#include "ResourceManager.h"
#include <rapidjson/prettywriter.h>
#include "LightComponent.h"
#include "Light.h"
#include "MathFunctions.h"
#include "Model.h"
std::string IOManager::IniFailedToFindItem = "NotFound";

std::string IOManager::ProjectName = "";

std::filesystem::path IOManager::ProjectDirectory = "";
std::filesystem::path IOManager::ExecutableDirectory = "";

std::array<IOManager::IOResourceData, static_cast<uint8_t>(ResourceType::Num)> IOManager::IOResources
{
    IOManager::IOResourceData{
        .Folder = "Models",
        .SpectralExtension = {".model"},
        .SupportedExtensions = { ".fbx" ,".blend", ".obj", ".gltf", ".glb" }
    },
    IOManager::IOResourceData{
        .Folder = "Textures",
        .SpectralExtension = {},
        .SupportedExtensions = { ".dds" ,".png" ,".jpg", ".jpeg", ".tga" }
    },
    IOManager::IOResourceData{
        .Folder = "Audio",
        .SpectralExtension = ".wav",
        .SupportedExtensions = { ".wav" }
    },
    IOManager::IOResourceData{
        .Folder = "Scenes",
        .SpectralExtension = ".scene",
        .SupportedExtensions = { ".scene" }
    },
    IOManager::IOResourceData{
        .Folder = "Scripts",
        .SpectralExtension = { ".lua" },
        .SupportedExtensions = { ".lua" }
    },
    IOManager::IOResourceData{
        .Folder = "Materials",
        .SpectralExtension = ".material",
        .SupportedExtensions = { ".material" }
    },
    IOManager::IOResourceData{
        .Folder = "Prefabs",
        .SpectralExtension = ".prefab",
        .SupportedExtensions = { ".prefab" }
    },
};




void IOManager::SetExecutableDirectiory()
{
    WCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    IOManager::ExecutableDirectory = std::filesystem::path(path).parent_path();
}

bool IOManager::LoadProject(std::optional<std::string> forceProject /*= std::nullopt*/)
{
    SetExecutableDirectiory();
#ifdef EDITOR
    if (forceProject.has_value())
    {
        IOManager::ProjectDirectory = forceProject.value();
        IOManager::CollectProjectFiles();
        IOManager::LoadSpectralScene("asd.json");
    }
    else
    {
        ProjectBrowserManager projectBrowserManager;// Will wait here until a project is selected and loaded;
        if (projectBrowserManager.HasLoadedProject() == false)
            return false;
    }
#else
    IOManager::ProjectDirectory = IOManager::ExecutableDirectory;
    IOManager::CollectProjectFiles();
    IOManager::LoadSpectralScene("asd.json");
#endif

    Render::SetWindowSize(Math::Vector2i(1280, 720));
    Render::ShowWindow(true);

    return true;
}

bool IOManager::LoadFBX(const std::filesystem::path& filename)
{
    Assimp::Importer importer;
    //const aiScene* scene = importer.ReadFile(filename.string(), aiProcess_PopulateArmatureData | aiProcess_PreTransformVertices | aiProcess_FixInfacingNormals | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_GlobalScale | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
    const aiScene* scene = importer.ReadFile(filename.string(), 
        aiProcess_PopulateArmatureData | 
        aiProcess_MakeLeftHanded | 
        aiProcess_Triangulate | 
        aiProcess_GlobalScale | 
        aiProcess_GenNormals | 
        aiProcess_CalcTangentSpace | 
        aiProcess_OptimizeMeshes |
        aiProcess_ImproveCacheLocality |
        aiProcess_JoinIdenticalVertices |
        aiProcess_OptimizeGraph
    );
    
    std::string strippedFilename(filename.filename().string().c_str());
    if (scene == nullptr)
    {
        Logger::Error("Could not load model: {} {}", strippedFilename, importer.GetErrorString());
        return false;
    }

    std::shared_ptr<Model> model = std::make_shared<Model>();
    model->m_filename = filename.filename().replace_extension(IOManager::GetResourceData<ResourceType::Model>().SpectralExtension).string();
    model->SetPath(Editor::GetInstance()->GetAssetBrowser()->m_currentOpenFolder / model->m_filename);
    IOManager::ProcessNode(model.get(), std::filesystem::path(filename).parent_path(), scene->mRootNode, scene, model->m_root, Math::Matrix());
    model->CalculateBoundingBox();
    ResourceManager::GetInstance()->AddResource<Model>(model);

    auto meshObject = ObjectManager::GetInstance()->CreateObject(strippedFilename);
    auto meshComponent = ComponentFactory::CreateComponent(meshObject, Component::Type::Mesh);
    std::static_pointer_cast<MeshComponent>(meshComponent)->SetMesh(ResourceManager::GetInstance()->GetResource<Model>(model->m_filename));
    meshObject->AddComponent(meshComponent);

    model->Save();
    //for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    //    aiMesh* mesh = scene->mMeshes[i];
    //
    //    if (mesh->HasBones())
    //    {
    //        for (unsigned int j = 0; j < mesh->mNumBones; j++)
    //        {
    //            aiBone* bone = mesh->mBones[j];
    //            auto object = ObjectManager::GetInstance()->CreateObject(bone->mName.C_Str());
    //            object->SetParent(rootObject);
    //            {
    //                aiVector3D position, rotation, scale;
    //                bone->mOffsetMatrix.Decompose(scale, rotation, position);
    //
    //                Math::Vector3 newPos;
    //                newPos.y = -position.z;
    //                newPos.x = position.y;
    //                newPos.z = position.z;
    //
    //                rotation *= 57.2957795f;
    //                Math::Matrix matrix;
    //                EditorUtils::RecomposeMatrix(matrix, newPos, *static_cast<Math::Vector3*>((void*)&rotation), *static_cast<Math::Vector3*>((void*)&scale));
    //                object->SetLocalMatrix(matrix);
    //            }
    //
    //        }
    //        //aiArmature* armature = mesh->mArmature;
    //        //
    //        //std::cout << "Armature name: " << armature->mName.C_Str() << std::endl;
    //        //std::cout << "Num bones: " << armature->mNumBones << std::endl;
    //        //
    //        //for (unsigned int j = 0; j < armature->mNumBones; ++j) {
    //        //    aiBone* bone = armature->mBones[j];
    //        //    std::cout << "Bone " << j << ": " << bone->mName.C_Str() << std::endl;
    //        //}
    //    }
    //}
    return true;
}



bool IOManager::LoadDroppedResource(const std::filesystem::path& file)
{
    namespace fs = std::filesystem;

    const std::string extension = file.extension().string();
    std::string folder;
    ResourceType matchedType = ResourceType::Num;

    for (size_t i = 0; i < static_cast<size_t>(ResourceType::Num); ++i)
    {
        auto& data = IOResources[i];
        for (const auto& ext : data.SupportedExtensions)
        {
            if (StringUtils::StringContainsCaseInsensitive(extension, ext))
            {
                folder = data.Folder;
                matchedType = static_cast<ResourceType>(i);
                break;
            }
        }
        if (matchedType != ResourceType::Num) break;
    }

    if (matchedType == ResourceType::Num)
    {
        Logger::Error("Unsupported file extension: {}", extension);
        return false;
    }

    fs::path fileName = file.filename();
    
    if (matchedType != ResourceType::Model)
    {
        fs::path target = Editor::GetInstance()->GetAssetBrowser()->m_currentOpenFolder / fileName;

        try
        {
            fs::copy_file(file, target, fs::copy_options::overwrite_existing);
        }
        catch (const std::exception& e)
        {
            Logger::Error(e.what());
            return false;
        }
    }
    else
    {
        LoadFBX(file);
    }


    std::shared_ptr<Resource> resource = nullptr;
    switch (matchedType)
    {
    case ResourceType::Audio:
        resource = ResourceManager::GetInstance()->GetResource<AudioSource>(std::string(fileName.string().c_str()), true);
        break;
    case ResourceType::Texture:
        resource = ResourceManager::GetInstance()->GetResource<Texture>(std::string(fileName.string().c_str()), true);
        break;
    case ResourceType::Model:
        resource = ResourceManager::GetInstance()->GetResource<Model>(std::string(fileName.string().c_str()), true);
        break;
    case ResourceType::Material:
        resource = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(std::string(fileName.string().c_str()), true);
        break;
    case ResourceType::Script:
        resource = ResourceManager::GetInstance()->GetResource<Script>(std::string(fileName.string().c_str()), true);
        break;
    case ResourceType::Scene:
        // not done
        break;
    default:
        break;
    }

    return resource != nullptr || matchedType == ResourceType::Script;
}


void IOManager::SaveSpectralScene(const std::string& sceneName)
{
    Scene& scene = SceneManager::GetInstance()->GetCurrentScene();

    Json::Object obj;
    obj.emplace("SceneName", sceneName);
    const auto& lightingSettings = scene.GetLightingSettings();

    auto ambientLight = lightingSettings.AmbientLight;
    obj.emplace("AmbientLight", Json::Array{ ambientLight.x,ambientLight.y,ambientLight.z,ambientLight.w });

    auto cameraPos = Render::GetCameraPosition();
    obj.emplace("LastCameraPosition", Json::Array{ cameraPos.x,cameraPos.y,cameraPos.z });

    auto fogColor = lightingSettings.FogColor;
    obj.emplace("FogColor", Json::Array{ fogColor.x,fogColor.y,fogColor.z,fogColor.w });

    obj.emplace("CameraDistance", lightingSettings.CameraDistance);
    obj.emplace("ShadowCameraSize", lightingSettings.ShadowCameraSize);
    obj.emplace("NearDepth", lightingSettings.NearDepth);
    obj.emplace("FarDepth", lightingSettings.FarDepth);

   
    Json::Array gameObjects;
    std::vector<GameObject*> rootGameObjects;

    for (const auto& gameObject : ObjectManager::GetInstance()->GetGameObjects())
    {
        if (gameObject->GetParent() == nullptr)
        {
            rootGameObjects.push_back(gameObject.get());
        }
    }    

    for (const auto& gameObject : rootGameObjects)
    {
        gameObjects.push_back(SaveGameObject(gameObject));
    }

    obj.emplace("GameObjects", gameObjects);
    
    auto path = ProjectDirectory / GetResourceData<ResourceType::Scene>().Folder / (sceneName + ".json");

    std::filesystem::create_directories(path.parent_path());

    Json::Serialize(obj, path);



    Logger::Info("Completed Saving Scene: " + sceneName);
#ifdef EDITOR
    Json projectDescription = Json::ParseFile(ProjectDirectory / "Project_Description.json");
    projectDescription.AsObjectRef().emplace("LastScene", sceneName + ".json");
    Json::Serialize(projectDescription, ProjectDirectory / "Project_Description.json");
#endif // EDITOR
}

bool IOManager::LoadSpectralScene(const std::string& filename)
{

    std::string jsonData;
    try {
        std::ifstream file(ProjectDirectory / IOManager::GetResourceData<ResourceType::Scene>().Folder / filename);
        Logger::Info((ProjectDirectory / IOManager::GetResourceData<ResourceType::Scene>().Folder / filename).string());
        std::stringstream buffer;
        buffer << file.rdbuf();

        jsonData = buffer.str();

    }
    catch (const std::exception& e) {
        Logger::Error(e.what());
        return 1;
    }

    rapidjson::Document document;
    if (document.Parse(jsonData.c_str()).HasParseError()) {
        Logger::Error("Error parsing JSON");
        return 1;
    }

    Scene& scene = SceneManager::GetInstance()->GetCurrentScene();
#ifdef EDITOR
    if (document.HasMember("LastCameraPosition") && document["LastCameraPosition"].IsArray()) {
        const rapidjson::Value& cameraPositionObject = document["LastCameraPosition"];

        Math::Vector3 cameraPosition;
        cameraPosition.x = cameraPositionObject[0].GetFloat();
        cameraPosition.y = cameraPositionObject[1].GetFloat();
        cameraPosition.z = cameraPositionObject[2].GetFloat();

        Editor::GetInstance()->GetEditorCameraController()->GetWorldMatrix().SetPosition(cameraPosition);

    }
#endif // EDITOR


    if (document.HasMember("AmbientLight") && document["AmbientLight"].IsArray()) {
        const rapidjson::Value& ambientLightObject = document["AmbientLight"];
        auto& ambientLight = scene.GetLightingSettings().AmbientLight;
        ambientLight.x = ambientLightObject[0].GetFloat();
        ambientLight.y = ambientLightObject[1].GetFloat();
        ambientLight.z = ambientLightObject[2].GetFloat();
        ambientLight.w = ambientLightObject[3].GetFloat();
    }

    if (document.HasMember("FogColor") && document["FogColor"].IsArray()) {
        const rapidjson::Value& fogColorObject = document["FogColor"];
        auto& fogColor = scene.GetLightingSettings().FogColor;
        fogColor.x = fogColorObject[0].GetFloat();
        fogColor.y = fogColorObject[1].GetFloat();
        fogColor.z = fogColorObject[2].GetFloat();
        fogColor.w = fogColorObject[3].GetFloat();
    }    

    if (document.HasMember("CameraDistance") && document["CameraDistance"].IsNumber()) {
        scene.GetLightingSettings().CameraDistance = document["CameraDistance"].GetFloat();
    }    
    if (document.HasMember("ShadowCameraSize") && document["ShadowCameraSize"].IsNumber()) {
        scene.GetLightingSettings().ShadowCameraSize = document["ShadowCameraSize"].GetFloat();
    }    
    if (document.HasMember("NearDepth") && document["NearDepth"].IsNumber()) {
        scene.GetLightingSettings().NearDepth = document["NearDepth"].GetFloat();
    }    
    if (document.HasMember("FarDepth") && document["FarDepth"].IsNumber()) {
        scene.GetLightingSettings().FarDepth = document["FarDepth"].GetFloat();
    }
    scene.LoadSceneSkybox();

    if (document.HasMember("GameObjects") && document["GameObjects"].IsArray()) {
        const rapidjson::Value& gameObjects = document["GameObjects"];
        for (rapidjson::SizeType i = 0; i < gameObjects.Size(); i++) {
            const rapidjson::Value& gameObject = gameObjects[i];
            LoadGameObject(gameObject, nullptr);
        }
    }


#ifdef EDITOR
    Json projectDescription = Json::ParseFile(ProjectDirectory / "Project_Description.json");

    projectDescription.AsObjectRef().emplace("LastScene", filename);
    Json::Serialize(projectDescription, ProjectDirectory / "Project_Description.json");
#endif // EDITOR


    return true;
}

bool IOManager::CreateDefaultScene()
{
    auto sunObject = ObjectManager::GetInstance()->CreateObject("Sun");

    auto lightComponent = ComponentFactory::CreateComponent(sunObject, Component::Type::Light);
    sunObject->AddComponent(lightComponent);
    std::static_pointer_cast<LightComponent>(lightComponent)->GetLight()->Type = Light::LightType::Directional;

    Math::Matrix m = sunObject->GetWorldMatrix();
    
    m = m * Math::Matrix::MakeRotationX(Math::ConvertToRadians(70.0f));
    sunObject->SetWorldMatrix(m);
    sunObject->SetPosition(Math::Vector3(10.0f, 4.0f, 0.0f));

    Editor::GetInstance()->GetEditorCameraController()->GetWorldMatrix().SetPosition(Math::Vector3(0.0f, 2.0f, -3.0f));

    return false;
}



void IOManager::CollectProjectFiles()
{
    DefaultAssets::Load();
    Logger::Info("Loading project files");
    std::filesystem::create_directories(ProjectDirectory);
    std::vector<std::filesystem::directory_entry> filesToLoad;
    for (auto& file : std::filesystem::recursive_directory_iterator(ProjectDirectory))
    {
        filesToLoad.push_back(file);
    }
    std::for_each(std::execution::par, filesToLoad.begin(), filesToLoad.end(), [](std::filesystem::directory_entry& file) 
    {

        auto extension = file.path().extension().string();
        if (!extension.empty())
        {
            auto* rm = ResourceManager::GetInstance();
            if (StringUtils::StringEqualsCaseInsensitive(extension , GetResourceData<ResourceType::Script>().SpectralExtension))
            {
                rm->GetResource<Script>(file.path());
            }
            else if (StringUtils::StringEqualsCaseInsensitive(extension , GetResourceData<ResourceType::Model>().SpectralExtension))
            {
                rm->GetResource<Model>(file.path());
            }
            else if (StringUtils::StringEqualsCaseInsensitive(extension , GetResourceData<ResourceType::Audio>().SpectralExtension))
            {
                rm->GetResource<AudioSource>(file.path());
            }
            else if (StringUtils::StringEqualsCaseInsensitive(extension , GetResourceData<ResourceType::Prefab>().SpectralExtension))
            {
                rm->GetResource<Prefab>(file.path());
            }
            else
            {

                

                for (auto& filetype : GetResourceData<ResourceType::Texture>().SupportedExtensions)
                {
                    if (StringUtils::StringEqualsCaseInsensitive(extension, filetype))
                    {
                        rm->GetResource<Texture>(file.path());
                    }
                }
            }
        }

    });
    Logger::Info("Finished project files");
}


Json::Object IOManager::SaveGameObject(GameObject* gameObject)
{
    Json::Object obj;
    obj.emplace("Name", gameObject->GetName());
    {
        Json::Array mat;
        const auto &matrix = gameObject->GetWorldMatrix();
        for (size_t r = 0; r < 4; r++)
        {
            for (size_t c = 0; c < 4; c++)
            {
                mat.push_back(matrix.data[r][c]);
            }
        }
        obj.emplace("Matrix", mat);
    }
    {
        Json::Array mat;
        const auto &matrix = gameObject->GetLocalMatrix();
        for (size_t r = 0; r < 4; r++)
        {
            for (size_t c = 0; c < 4; c++)
            {
                mat.push_back(matrix.data[r][c]);
            }
        }
        obj.emplace("LocalMatrix", mat);
    }

    if (gameObject->IsPrefab())
    {
        obj.emplace("IsPrefab", true);
    }
    {
        Json::Array componentsArray;
        for (const auto& comonent : gameObject->GetComponents())
        {
            auto componentObject = comonent->SaveComponent();
            auto it = ComponentFactory::ComponentTypes.find(comonent->GetComponentType());
            if (it != ComponentFactory::ComponentTypes.end())
            {
                componentObject.emplace("Name", it->second);
            }
            else
            {
                componentObject.emplace("Name", "null");
            }

            componentsArray.push_back(componentObject);
        }
        obj.emplace("Components", componentsArray);
    }

    {
        Json::Array childrenArray;
        for (auto& child : gameObject->GetChildren())
        {
            auto childObject = SaveGameObject(child);
            childrenArray.push_back(childObject);
        }
        obj.emplace("Children", childrenArray);
    }



    return std::move(obj);
}


void IOManager::LoadGameObject(const rapidjson::Value& object, GameObject* parent)
{
    std::string name;
    if (object.HasMember("Name") && object["Name"].IsString())
    {
        name = object["Name"].GetString();
    }
    GameObject* gameObject = ObjectManager::GetInstance()->CreateObject(name);

    if (object.HasMember("IsPrefab") && object["IsPrefab"].GetBool())
    {
        auto prefabObject = ResourceManager::GetInstance()->GetResource<Prefab>(gameObject->GetName() + GetResourceData<ResourceType::Prefab>().SpectralExtension);
        if (!prefabObject)
        {
            Logger::Error("Could not load prefab for: {}", name);
            return;
        }
        EditorUtils::DuplicateGameObject(gameObject, prefabObject->GetPrefabRoot());
        gameObject->SetPrefab(prefabObject);
    }


    {
        const rapidjson::Value& jsonMatrix = object["Matrix"];
        Math::Matrix matrix;
        int r = 0;
        int c = 0;
        for (rapidjson::SizeType i = 0; i < jsonMatrix.Size(); i++)
        {
   
            matrix.data[c][r] = jsonMatrix[i].GetFloat();
            r++;
            if (r >= 4)
            {
                r = 0;
                c++;
            }
        }
        if (gameObject->IsPrefab())
        {
            gameObject->SetWorldMatrix(matrix);
        }
        else
        {
            gameObject->SetWorldMatrixNoUpdate(matrix);
        }
    }

    {
        const rapidjson::Value& jsonMatrix = object["LocalMatrix"];
        Math::Matrix matrix;
        int r = 0;
        int c = 0;
        for (rapidjson::SizeType i = 0; i < 16; i++)
        {
            matrix.data[c][r] = jsonMatrix[i].GetFloat();
            r++;
            if (r >= 4)
            {
                r = 0;
                c++;
            }
        }
        gameObject->SetLocalMatrixNoUpdate(matrix);
    }


    if (parent != nullptr)
    {
        gameObject->SetParent(parent);
    }
    if (gameObject->IsPrefab())
    {
        return;
    }

    if(object.HasMember("Components"))
    {
        const rapidjson::Value& components = object["Components"];
        for (rapidjson::SizeType i = 0; i < components.Size(); i++) {
            const rapidjson::Value& componentObject = components[i];


            if (ComponentFactory::ComponentNames.find(componentObject["Name"].GetString()) == ComponentFactory::ComponentNames.end())
            {
                continue;
            }

            auto component = ComponentFactory::CreateComponent(
                gameObject, 
                ComponentFactory::ComponentNames.find(componentObject["Name"].GetString())->second, 
                nullptr
            );
            if (component)
            {
                component->LoadComponent(componentObject);
                gameObject->AddComponent(component);
            }
        }
    }

    if (object.HasMember("Children") && object["Children"].IsArray()) {
        const rapidjson::Value& Children = object["Children"];
        for (rapidjson::SizeType i = 0; i < Children.Size(); i++) {
            const rapidjson::Value& child = Children[i];

            LoadGameObject(child, gameObject);
        }
    }
}


void IOManager::ProcessMesh(Model* model,
    const std::filesystem::path& path, 
    aiMesh* mesh, const aiScene* scene, 
    SubMesh& subMesh
)
{
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::string materialName = std::string(material->GetName().C_Str());
    materialName.append(GetResourceData<ResourceType::Material>().SpectralExtension);




    std::string meshName = std::string(mesh->mName.C_Str());

    subMesh.m_mesh = std::make_shared<Mesh>(meshName);
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
        Mesh::Vertex vertex{};
        memcpy(&vertex.position, &mesh->mVertices[i], sizeof(float) * 3);
        memcpy(&vertex.normal, &mesh->mNormals[i], sizeof(float) * 3);

        if (mesh->mTangents)
        {
            memcpy(&vertex.tangent, &mesh->mTangents[i], sizeof(float) * 3);
        }
        else
        {
            Logger::Error("{} is missing tangents!", meshName);
        }

        if (mesh->mTextureCoords[0]) {
            vertex.uv.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.uv.y = 1.0f - (float)mesh->mTextureCoords[0][i].y;
        }

        subMesh.m_mesh->vertexes.push_back(vertex);
    }

    for (UINT i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (UINT j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            subMesh.m_mesh->indices32.push_back(face.mIndices[j]);
        }
    }
    subMesh.m_mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
    subMesh.m_mesh->CalculateBoundingBox();




    int materialIndex = -1;

    auto& materials = model->GetMaterials();
    for (int i = 0; i < static_cast<int>(materials.size()); i++)
    {
        if (materials[i]->GetFilename() == materialName)
        {
            materialIndex = i;
        }
    }

    if (materialIndex != -1)
    {
        return;
    }

    materialIndex = static_cast<int>(materials.size());


    std::shared_ptr<DefaultMaterial> m = ResourceManager::GetInstance()->GetResource<DefaultMaterial>(materialName, true);
    if (!m && materialName != "")
    {
    
        std::shared_ptr<DefaultMaterial> createdMaterial = std::make_shared<DefaultMaterial>();
    
        auto LoadAndApplyTexture = [&](DefaultMaterial::TextureType textureType, aiTextureType aiTextureType)
        {
            aiString textureFilename;
            material->GetTexture(aiTextureType, 0, &textureFilename);
            if (std::string(textureFilename.C_Str()) != "")
            {
                auto texturePath = std::filesystem::path(textureFilename.C_Str()).filename();
    
                if (IOManager::LoadDroppedResource(path / texturePath))
                {
                    createdMaterial->SetTexture(textureType, ResourceManager::GetInstance()->GetResource<Texture>(texturePath.string()));
                    return;
                }
            }
            switch (textureType)
            {
            case DefaultMaterial::BaseColor:
                createdMaterial->SetTexture(0, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_albedo.bmp"));
                break;
            case DefaultMaterial::Normal:
                createdMaterial->SetTexture(1, ResourceManager::GetInstance()->GetResource<Texture>("TemplateGrid_normal.bmp"));
                break;
            }
        };
    
    
    
        LoadAndApplyTexture(DefaultMaterial::TextureType::BaseColor, aiTextureType_DIFFUSE);
        LoadAndApplyTexture(DefaultMaterial::TextureType::BaseColor, aiTextureType_BASE_COLOR);
        LoadAndApplyTexture(DefaultMaterial::TextureType::Normal, aiTextureType_NORMALS);
        LoadAndApplyTexture(DefaultMaterial::TextureType::Roughness, aiTextureType_SPECULAR);
        LoadAndApplyTexture(DefaultMaterial::TextureType::Roughness, aiTextureType_DIFFUSE_ROUGHNESS);
        LoadAndApplyTexture(DefaultMaterial::TextureType::Metallic, aiTextureType_METALNESS);
        LoadAndApplyTexture(DefaultMaterial::TextureType::AmbientOcclusion, aiTextureType_AMBIENT_OCCLUSION);
        createdMaterial->m_filename = materialName;
        createdMaterial->SetPath(path / materialName);
        ResourceManager::GetInstance()->AddResource<DefaultMaterial>(createdMaterial);
        createdMaterial->Save();
        materials.push_back(createdMaterial);
    }
    else
    {
        if (m)
        {
            materials.push_back(m);
        }
        else
        {
            materials.push_back(ResourceManager::GetInstance()->GetResource<DefaultMaterial>("Default.material"));
        }
    }
}

void IOManager::ProcessNode(Model* model, const std::filesystem::path& path, aiNode* node, const aiScene* scene, SubMesh& subMesh, const Math::Matrix& accTransform)
{
    Math::Matrix matrix = *static_cast<Math::Matrix*>((void*)&node->mTransformation);
    matrix.Transpose();
    matrix = matrix * accTransform;

    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        SubMesh& child = subMesh.m_submeshes.emplace_back();
        child.m_localMatrix = accTransform;

        UINT subMeshId = node->mMeshes[i];
        ProcessMesh(model, path, scene->mMeshes[subMeshId], scene, child);
    }
    
    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        //SubMesh& child = subMesh.m_submeshes.emplace_back();
        ProcessNode(model, path, node->mChildren[i], scene, subMesh, matrix);
    }
}
