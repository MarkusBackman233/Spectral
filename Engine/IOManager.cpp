#include "IOManager.h"
#include <iostream>
#include "Mesh.h"
#include "TextureManager.h"
#include "iRender.h"
#include "ObjectManager.h"
#include "Logger.h"
#include "Matrix.h"
#include <filesystem>
#include "ModelManager.h"
#include "ComponentFactory.h"
#include <ppltasks.h>
#include "MaterialManager.h"
#include "StringUtils.h"
#include "RenderManager.h"
#include "SceneManager.h"
#include "ScriptManager.h"
#include "Texture.h"
#include "Material.h"
#include "Editor.h"

#include "MeshComponent.h"
#include "EditorUtils.h"
#include "DefaultAssets.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp\scene.h>
#include "thread"

#include <algorithm>
#include <execution>

#include "EditorCameraController.h"
#include "ProjectBrowserManager.h"

#include "AudioManager.h"

std::string IOManager::IniFailedToFindItem = "NotFound";

std::string IOManager::ProjectName = "";

std::filesystem::path IOManager::ProjectDirectory = "";
std::filesystem::path IOManager::ExecutableDirectory = "";

std::string IOManager::SpectralModelExtention = ".spm";
std::string IOManager::SpectralSceneExtention = ".sps";
std::string IOManager::SpectralMaterialExtention = ".spmt";

const std::vector<std::string> IOManager::SupportedTextureFiles = { ".dds" ,".png" ,".jpg", ".jpeg", ".tga" };
const std::vector<std::string> IOManager::SupportedMeshFiles = { ".fbx" ,".blend", ".obj", ".gltf" };
const std::vector<std::string> IOManager::SupportedAudioFiles = { ".wav" };

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
    const aiScene* scene = importer.ReadFile(filename.string(), aiProcess_PreTransformVertices | aiProcess_FixInfacingNormals | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_GlobalScale | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
    auto strippedFilename = filename.filename().string();
    if (scene == nullptr)
    {
        Logger::Error("Could not load model: " + strippedFilename);
        return false;
    }
    
    auto rootObject = ObjectManager::GetInstance()->CreateObject(strippedFilename);
    aiVector3D position, rotation, scale;
    scene->mRootNode->mTransformation.Decompose(scale, rotation, position);
    rotation *= 57.2957795f;
    Math::Matrix matrix;
    EditorUtils::RecomposeMatrix(matrix, *static_cast<Math::Vector3*>((void*)&position), *static_cast<Math::Vector3*>((void*)&rotation), *static_cast<Math::Vector3*>((void*)&scale));
    rootObject->SetWorldMatrix(matrix);
    IOManager::ProcessNode(strippedFilename, std::filesystem::path(filename).parent_path(), scene->mRootNode, scene, rootObject, Math::Matrix());
    return true;
}
/*
bool IOManager::LoadTexture(const std::string& filename)
{
    namespace fs = std::filesystem;
    fs::path sourceFile = filename;
    fs::path targetParent = IOManager::ProjectDirectory;
    auto target = targetParent / sourceFile.filename();
    try
    {
        fs::create_directories(targetParent);
        fs::copy_file(sourceFile, target, fs::copy_options::overwrite_existing);
    }
    catch (std::exception& e)
    {
        Logger::Error(e.what());
    }
    
    TextureManager::GetInstance()->GetTexture(StringUtils::StripPathFromFilename(filename));
    return true;
}
*/
bool IOManager::LoadTexture(const std::filesystem::path& file)
{
    namespace fs = std::filesystem;

    fs::path targetParent = IOManager::ProjectDirectory;
    auto target = targetParent / file.filename();

    try
    {
        fs::create_directories(targetParent);
        fs::copy_file(file, target, fs::copy_options::overwrite_existing);
    }
    catch (std::exception& e)
    {
        Logger::Error(e.what());
    }

    TextureManager::GetInstance()->GetTexture(file);
    return true;
}

bool IOManager::LoadSpectralModel(const std::string& filename, std::shared_ptr<Mesh>& mesh)
{
    Logger::Info("Loading Model: " + filename);
    ReadObject readObject(GetPath(filename, SpectralModelExtention));
    if (!readObject.GetFile().is_open()) 
    {
        return false;
    }
    readObject.Read(mesh->GetName());
    readObject.Read(mesh->vertexes);
    readObject.Read(mesh->indices32);

    mesh->CalculateBoundingBox();
    mesh->CreateVertexAndIndexBuffer(Render::GetDevice());
    return true;
}

void IOManager::SaveSpectralModel(std::shared_ptr<Mesh> mesh)
{
    WriteObject writeObject(GetPath(mesh->GetName(), SpectralModelExtention));
    writeObject.Write(mesh->GetName());
    writeObject.Write(mesh->vertexes);
    writeObject.Write(mesh->indices32);
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
    
    for (const auto& [materialName, material] : MaterialManager::GetInstance()->GetMaterials())
    {
        SaveSpectralMaterial(material);
    }

    for (const auto& gameObject : rootGameObjects)
    {
        gameObjects.push_back(SaveGameObject(gameObject));
    }

    obj.emplace("GameObjects", gameObjects);
    Json::Serialize(obj, GetPath(sceneName, ".json").string());

    Logger::Info("Completed Saving Scene: " + sceneName);
#ifdef EDITOR
    auto projectIniFile = ProjectDirectory / std::string("Spectral_Project.ini");
    WriteToIniFile(projectIniFile, "Project", "LastScene", sceneName + ".json");
#endif // EDITOR
}

bool IOManager::LoadSpectralScene(const std::string& filename)
{

    std::string jsonData;
    try {
        std::ifstream file(ProjectDirectory / filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

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
    auto projectIniFile = ProjectDirectory / std::string("Spectral_Project.ini");
    WriteToIniFile(projectIniFile,"Project","LastScene",filename);
#endif // EDITOR


    return true;
}

void IOManager::SaveSpectralMaterial(std::shared_ptr<Material> material)
{
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    auto SaveTextureFilename = [&material](const std::shared_ptr<Texture>& texture) -> std::string
    {
        if (texture.get())
        {
            return texture->GetFilename().c_str();
        }
        return "None";
    };


    document.AddMember("Name", rapidjson::Value(material->GetName().c_str(), allocator), allocator);
    document.AddMember("Albedo", rapidjson::Value(SaveTextureFilename(material->GetTexture(Material::BaseColor)).c_str(), allocator), allocator);
    document.AddMember("Normal", rapidjson::Value(SaveTextureFilename(material->GetTexture(Material::Normal)).c_str(), allocator), allocator);
    document.AddMember("Roughness", rapidjson::Value(SaveTextureFilename(material->GetTexture(Material::Roughness)).c_str(), allocator), allocator);
    document.AddMember("Metallic", rapidjson::Value(SaveTextureFilename(material->GetTexture(Material::Metallic)).c_str(), allocator), allocator);
    document.AddMember("AmbientOcclusion", rapidjson::Value(SaveTextureFilename(material->GetTexture(Material::AmbientOcclusion)).c_str(), allocator), allocator);
    document.AddMember("RougnessFloat", material->GetMaterialSettings().Roughness, allocator);
    document.AddMember("MetallicFloat", material->GetMaterialSettings().Metallic, allocator);
    document.AddMember("BackfaceCulling", material->GetMaterialSettings().BackfaceCulling, allocator);
    {
        auto color = material->GetMaterialSettings().Color;
        rapidjson::Value colorArray(rapidjson::kArrayType);
        colorArray.PushBack(color.x, allocator);
        colorArray.PushBack(color.y, allocator);
        colorArray.PushBack(color.z, allocator);
        colorArray.PushBack(color.w, allocator);
        document.AddMember("Color", colorArray, allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    
    std::ofstream ofs(GetPath(material->GetName(), SpectralMaterialExtention));
    if (ofs.is_open()) {
        ofs << buffer.GetString();
        ofs.close();
    }
    else {
        Logger::Error("Could not open file for writing.");
    }
}

bool IOManager::LoadSpectralMaterial(const std::string& filename, std::shared_ptr<Material>& material)
{
    std::string jsonData;
    try {
        std::ifstream file(GetPath(filename, SpectralMaterialExtention));
        if (!file.is_open()) {
            throw std::runtime_error("Could Not load Material: " + filename);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        jsonData = buffer.str();

    }
    catch (const std::exception& e) {
        Logger::Error(e.what());
        return false;
    }

    rapidjson::Document document;
    if (document.Parse(jsonData.c_str()).HasParseError()) {
        material = MaterialManager::GetInstance()->GetDefaultMaterial();

        Logger::Error("Parsing JSON for material:" + filename);
        return false;
    }


    if (document.HasMember("Name")) 
    {
        material->SetName(document["Name"].GetString());
    }

    auto LoadTexture = [&](const char* textureName, Material::TextureType textureType)
    {
        if (document.HasMember(textureName) && !std::string(document[textureName].GetString()).empty())
        {
            std::string name = document[textureName].GetString();
            if (name == std::string("none") || name == std::string("None"))
            {
                return;
            }
            material->SetTexture(textureType, TextureManager::GetInstance()->GetTexture(ProjectDirectory / name));
        }
    };

    LoadTexture("Albedo", Material::BaseColor);
    LoadTexture("Normal", Material::Normal);
    LoadTexture("Roughness", Material::Roughness);
    LoadTexture("Metallic", Material::Metallic);
    LoadTexture("AmbientOcclusion", Material::AmbientOcclusion);

    if (document.HasMember("RougnessFloat"))
    {
        material->GetMaterialSettings().Roughness = document["RougnessFloat"].GetFloat();
    }
    if (document.HasMember("MetallicFloat"))
    {
        material->GetMaterialSettings().Metallic = document["MetallicFloat"].GetFloat();
    }    
    if (document.HasMember("BackfaceCulling"))
    {
        material->GetMaterialSettings().BackfaceCulling = document["BackfaceCulling"].GetBool();
    }
    if (document.HasMember("Color") && document["Color"].IsArray()) {
        const rapidjson::Value& colorObject = document["Color"];
        auto& color = material->GetMaterialSettings().Color;
        color.x = colorObject[0].GetFloat();
        color.y = colorObject[1].GetFloat();
        color.z = colorObject[2].GetFloat();
        color.w = colorObject[3].GetFloat();
    }
    return true;
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
        if (StringUtils::StringContainsCaseInsensitive(file.path().extension().string(), ".lua"))
        {
            ScriptManager::GetInstance()->GetScript(file.path().filename().replace_extension("").string());
        }
        else if (file.path().extension() == SpectralModelExtention)
        {
            std::string filename = StringUtils::StripPathFromFilename(file.path().string());
            size_t lastindex = filename.find_last_of(".");

            auto mesh = std::make_shared<Mesh>();
            IOManager::LoadSpectralModel(filename.substr(0, lastindex), mesh);
            ModelManager::GetInstance()->AddMesh(filename.substr(0, lastindex), mesh);
        }
        else if (file.path().extension() == SupportedAudioFiles[0])
        {
            AudioManager::GetInstance()->GetAudioSource(file.path());
        }
        else
        {
            for (auto& filetype : SupportedTextureFiles)
            {
                if (StringUtils::StringContainsCaseInsensitive(file.path().extension().string(), filetype))
                {
                    TextureManager::GetInstance()->GetTexture(file.path());
                }
            }
        }

    });
    Logger::Info("Finished project files");
}

void IOManager::WriteToIniFile(const std::filesystem::path& iniPath, const std::string& attribute, const std::string& name, const std::string& value)
{
    BOOL writeResult = WritePrivateProfileStringA(attribute.c_str(), name.c_str(), value.c_str(), iniPath.string().c_str());
    if (!writeResult) {
        DWORD error = GetLastError();
        Logger::Error("Writing to INI file: " + error);
    }
}

std::string IOManager::ReadFromIniFile(const std::filesystem::path& iniPath, const std::string& attribute, const std::string& name)
{
    std::string returnValue;
    returnValue.resize(256);
    GetPrivateProfileStringA(attribute.c_str(), name.c_str(), IOManager::IniFailedToFindItem.c_str(), returnValue.data(), 256, iniPath.string().c_str());

    return returnValue;
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
    {
        Json::Array componentsArray;
        for (const auto& comonent : gameObject->GetComponents())
        {
            auto componentObject = comonent->SaveComponent();
            componentObject.emplace("Name", comonent->GetComponentName());
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
        gameObject->SetWorldMatrix(matrix);
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
        gameObject->SetLocalMatrix(matrix);
    }


    if (parent != nullptr)
    {
        gameObject->SetParent(parent);
    }

    {
        const rapidjson::Value& components = object["Components"];
        for (rapidjson::SizeType i = 0; i < components.Size(); i++) {
            const rapidjson::Value& componentObject = components[i];
            auto component = ComponentFactory::CreateComponent(gameObject, ComponentFactory::ComponentNames.find(componentObject["Name"].GetString())->second);
            component->LoadComponent(componentObject);
            gameObject->AddComponent(component);
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


void IOManager::ProcessMesh(const std::string& filename, const std::filesystem::path& path, aiMesh* mesh, const aiScene* scene, GameObject* gameObject)
{
    auto meshObject = ObjectManager::GetInstance()->CreateObject(mesh->mName.C_Str());
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::string materialName(material->GetName().data, material->GetName().length);
    
    std::string meshName = std::string(std::string(filename.c_str()) + "_" + std::string(mesh->mName.data, mesh->mName.length) + "_" + materialName);
    std::replace(meshName.begin(), meshName.end(), ':', '_');
    std::replace(meshName.begin(), meshName.end(), '.', '_');    
    std::replace(materialName.begin(), materialName.end(), ':', '_');
    std::replace(materialName.begin(), materialName.end(), '.', '_');
    std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>();
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
            Logger::Error(meshName + " is missing tangents!");
        }

        if (mesh->mTextureCoords[0]) {
            vertex.uv.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.uv.y = 1.0f - (float)mesh->mTextureCoords[0][i].y;
        }

        newMesh->vertexes.push_back(vertex);
    }

    for (UINT i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];

        for (UINT j = 0; j < mesh->mFaces[i].mNumIndices; j++)
        {
            newMesh->indices32.push_back(face.mIndices[j]);
        }
    }

    newMesh->CreateVertexAndIndexBuffer(Render::GetDevice());

    meshObject->AddComponent(std::make_shared<MeshComponent>(meshObject, newMesh));
    meshObject->SetWorldMatrix(gameObject->GetWorldMatrix());
    meshObject->SetParent(gameObject);
    newMesh->CalculateBoundingBox();

    if (materialName != "")
    {
        newMesh->SetMaterial(MaterialManager::GetInstance()->GetMaterial(std::string(std::string(filename.c_str()) + "_" + materialName)));
    }
    else
    {
        newMesh->SetMaterial(MaterialManager::GetInstance()->GetDefaultMaterial());
    }


    auto LoadAndApplyTexture = [&](Material::TextureType textureType, aiTextureType aiTextureType)
    {
        aiString textureFilename;
        material->GetTexture(aiTextureType, 0, &textureFilename);
        if (std::string(textureFilename.C_Str()) != "")
        {
            IOManager::LoadTexture(path / std::string(textureFilename.C_Str()));
            newMesh->GetMaterial()->SetTexture(textureType, TextureManager::GetInstance()->GetTexture(StringUtils::StripPathFromFilename(textureFilename.C_Str())));
        }
    };

    LoadAndApplyTexture(Material::TextureType::BaseColor, aiTextureType_DIFFUSE);
    LoadAndApplyTexture(Material::TextureType::BaseColor, aiTextureType_BASE_COLOR);
    LoadAndApplyTexture(Material::TextureType::Normal, aiTextureType_NORMALS);
    LoadAndApplyTexture(Material::TextureType::Roughness, aiTextureType_SPECULAR);
    LoadAndApplyTexture(Material::TextureType::Roughness, aiTextureType_DIFFUSE_ROUGHNESS);
    LoadAndApplyTexture(Material::TextureType::Metallic, aiTextureType_METALNESS);
    LoadAndApplyTexture(Material::TextureType::AmbientOcclusion, aiTextureType_AMBIENT_OCCLUSION);
    newMesh->SetName(meshName);
    ModelManager::GetInstance()->AddMesh(meshName, newMesh);

    auto DropfileTask = Concurrency::create_task(
        [newMesh]()
    {
        IOManager::SaveSpectralModel(newMesh);
        SaveSpectralMaterial(newMesh->GetMaterial());
    }
    );

}

void IOManager::ProcessNode(const std::string& filename, const std::filesystem::path& path, aiNode* node, const aiScene* scene, GameObject* parent, const Math::Matrix& accTransform)
{
    GameObject* newGameObject = nullptr;
    Math::Matrix matrix;
    aiVector3D position, rotation, scale;
    node->mTransformation.Decompose(scale, rotation, position);
    rotation *= 57.2957795f;
    EditorUtils::RecomposeMatrix(matrix, *static_cast<Math::Vector3*>((void*)&position), *static_cast<Math::Vector3*>((void*)&rotation), *static_cast<Math::Vector3*>((void*)&scale));
    
    if (node->mNumMeshes > 0) 
    {
        newGameObject = ObjectManager::GetInstance()->CreateObject(node->mName.C_Str());
        newGameObject->SetWorldMatrix(matrix * accTransform);
        for (UINT i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(filename, path, mesh, scene, newGameObject);
        }
        newGameObject->SetParent(parent);
        parent = newGameObject;
    }
    else {
        matrix = matrix * accTransform;
    }

    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(filename, path, node->mChildren[i], scene, parent, matrix);
    }
}

std::filesystem::path IOManager::GetPath(const std::string& filename, const std::string& extention)
{
    std::filesystem::path filePath = ProjectDirectory / (filename + extention);
    return filePath;
}
