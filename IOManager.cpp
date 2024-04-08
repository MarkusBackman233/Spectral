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

#include "EditorUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp\scene.h>

std::string IOManager::ProjectDirectory = "C:/Projects/Spectral/Exported/";
std::wstring IOManager::ProjectDirectoryWide = L"C:\\Projects\\Spectral\\Exported\\";

std::string IOManager::SpectralModelExtention = ".spm";
std::string IOManager::SpectralSceneExtention = ".sps";
std::string IOManager::SpectralMaterialExtention = ".spmt";


bool IOManager::LoadFBX(const std::string& filename)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename, aiProcess_PreTransformVertices | aiProcess_FixInfacingNormals | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_GlobalScale | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
    auto strippedFilename = StringUtils::StripPathFromFilename(filename);
    if (scene == nullptr)
    {
        Editor::GetInstance()->LogMessage("Error: Could not load file:: " + strippedFilename);
        return false;
    }
    
    auto rootObject = ObjectManager::GetInstance()->CreateObject(strippedFilename);
    aiVector3D position, rotation, scale;
    scene->mRootNode->mTransformation.Decompose(scale, rotation, position);
    rotation *= 57.2957795f;
    Math::Matrix matrix;
    EditorUtils::RecomposeMatrix(matrix, *static_cast<Math::Vector3*>((void*)&position), *static_cast<Math::Vector3*>((void*)&rotation), *static_cast<Math::Vector3*>((void*)&scale));
    rootObject->GetMatrix() = matrix;
    IOManager::ProcessNode(strippedFilename, scene->mRootNode, scene, rootObject, Math::Matrix());
    return true;
}

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
        std::cout << e.what();
    }
    
    TextureManager::GetInstance()->GetTexture(StringUtils::StripPathFromFilename(filename));
    return true;
}

bool IOManager::LoadSpectralModel(const std::string& filename, std::shared_ptr<Mesh>& mesh)
{
    LogMessage("Loading Model: " + filename);
    ReadObject readObject(ProjectDirectory+filename+SpectralModelExtention);
    if (!readObject.GetFile().is_open()) 
    {
        return false;
    }
    readObject.Read(mesh->GetName());
    readObject.Read(mesh->vertexes);
    readObject.Read(mesh->indices32);
    mesh->SetMaterial(MaterialManager::GetInstance()->GetMaterial(readObject.Read<std::string>()));

    mesh->CalculateBoundingBox();
    Render::CreateVertexAndIndexBuffer(mesh.get());
    return true;
}

void IOManager::SaveSpectralModel(std::shared_ptr<Mesh> mesh)
{
    WriteObject writeObject(ProjectDirectory + mesh->GetName() + SpectralModelExtention);
    writeObject.Write(mesh->GetName());
    writeObject.Write(mesh->vertexes);
    writeObject.Write(mesh->indices32);
    writeObject.Write(mesh->GetMaterial()->GetName());
}

void IOManager::SaveSpectralScene(const std::string& sceneName)
{
    WriteObject writeObject(ProjectDirectory + sceneName + SpectralSceneExtention);
    std::vector<GameObject*> rootGameObjects;

    for (const auto& gameObject : ObjectManager::GetInstance()->GetGameObjects())
    {
        if (gameObject->GetParent() == nullptr)
        {
            rootGameObjects.push_back(gameObject);
        }
    }


    writeObject.Write(rootGameObjects.size());
    for (const auto& gameObject : rootGameObjects)
    {
        SaveGameObject(writeObject, gameObject);
    }
    LogMessage("Completed Saving Scene: " + sceneName);
}

bool IOManager::LoadSpectralScene(const std::string& filename)
{
    ReadObject readObject(filename);
    if (!readObject.GetFile().is_open())
    {
        return false;
    }
    size_t numGameObjects = readObject.Read<size_t>();

    for (size_t i = 0; i < numGameObjects; i++)
    {
        LoadGameObject(readObject, nullptr);
    }
    return true;
}

void IOManager::SaveSpectralMaterial(std::shared_ptr<Material> material)
{
    WriteObject writeObject(ProjectDirectory + material->GetName() + SpectralMaterialExtention);
    writeObject.Write(material->GetName());
    writeObject.Write(StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(material->GetTexture(ALBEDO))));
    writeObject.Write(StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(material->GetTexture(NORMAL))));
    writeObject.Write(StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(material->GetTexture(ROUGHNESS))));
    writeObject.Write(StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(material->GetTexture(METALLIC))));
    writeObject.Write(StringUtils::StripPathFromFilename(TextureManager::GetInstance()->GetTextureName(material->GetTexture(AO))));
    writeObject.Write(material->GetMaterialSettings());
}

bool IOManager::LoadSpectralMaterial(const std::string& filename, std::shared_ptr<Material>& material)
{
    ReadObject readObject(ProjectDirectory + filename + SpectralMaterialExtention);
    if (!readObject.GetFile().is_open())
    {
        return false;
    }
    readObject.Read(material->GetName());
    material->SetTexture(ALBEDO, TextureManager::GetInstance()->GetTexture(readObject.Read<std::string>()));
    material->SetTexture(NORMAL, TextureManager::GetInstance()->GetTexture(readObject.Read<std::string>()));
    material->SetTexture(ROUGHNESS, TextureManager::GetInstance()->GetTexture(readObject.Read<std::string>()));
    material->SetTexture(METALLIC, TextureManager::GetInstance()->GetTexture(readObject.Read<std::string>()));
    material->SetTexture(AO, TextureManager::GetInstance()->GetTexture(readObject.Read<std::string>()));
    material->GetMaterialSettings() = readObject.Read<Material::MaterialSettings>();
    MaterialManager::GetInstance()->AddMaterial(material);
    return true;
}

void IOManager::CollectProjectFiles()
{
    std::cout << "Loading project files" << std::endl;

    std::vector<std::filesystem::directory_entry> filesToLoad;
    for (auto& file : std::filesystem::recursive_directory_iterator(ProjectDirectory))
    {
        if (file.path().extension() == SpectralModelExtention)
        {
            filesToLoad.push_back(file);
        }
    }

    int loadedFiles = 0;

    for (auto& file : filesToLoad)
    {
        auto DropfileTask = Concurrency::create_task(
            [file, &loadedFiles]()
        {
            std::string filename = StringUtils::StripPathFromFilename(file.path().string());
            size_t lastindex = filename.find_last_of(".");

            auto mesh = std::make_shared<Mesh>();
            IOManager::LoadSpectralModel(filename.substr(0, lastindex), mesh);
            ModelManager::GetInstance()->AddMesh(filename.substr(0, lastindex), mesh);

            loadedFiles++;
        }
        );
    }

    
    while (loadedFiles != filesToLoad.size())
    {
        Sleep(100);
    }


    std::cout << "Generating Mip Maps" << std::endl;

    //for (auto& [textureName, texture] : TextureManager::GetInstance()->GetCachedTextures())
    //{
    //    texture->GenerateMips();
    //}
    std::cout << "Finished project files" << std::endl;
}

void IOManager::SaveGameObject(WriteObject& writeObject, GameObject* gameObject)
{
    writeObject.Write(gameObject->GetName());
    writeObject.Write(gameObject->GetMatrix());
    writeObject.Write(gameObject->GetLocalMatrix());
    writeObject.Write(gameObject->GetComponents().size());
    for (const auto& comonent : gameObject->GetComponents())
    {
        writeObject.Write(comonent->GetComponentType());
        comonent->SaveComponent(writeObject);
    }
    writeObject.Write(gameObject->GetChildren().size());
    for (auto& child : gameObject->GetChildren())
    {
        SaveGameObject(writeObject, child);
    }
}

void IOManager::LoadGameObject(ReadObject& readObject, GameObject* parent)
{
    GameObject* gameObject = ObjectManager::GetInstance()->CreateObject(readObject.Read<std::string>());
    readObject.Read(gameObject->GetMatrix());
    readObject.Read(gameObject->GetLocalMatrix());

    if (parent != nullptr)
    {
        gameObject->SetParent(parent);
    }

    size_t numComponents = readObject.Read<size_t>();
    for (size_t i = 0; i < numComponents; i++)
    {
        auto component = ComponentFactory::CreateComponent(gameObject, readObject.Read<Component::ComponentType>());
        component->LoadComponent(readObject);
        gameObject->AddComponent(component);
    }
    size_t numChildren = readObject.Read<size_t>();
    for (size_t i = 0; i < numChildren; i++)
    {
        LoadGameObject(readObject, gameObject);
    }
}


void IOManager::ProcessMesh(const std::string& filename, aiMesh* mesh, const aiScene* scene, GameObject* gameObject)
{
    auto meshObject = ObjectManager::GetInstance()->CreateObject(mesh->mName.C_Str());
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    auto materialName = material->GetName().C_Str();
    std::string meshName = std::string(std::string(filename.c_str()) + "_" + std::string(mesh->mName.C_Str()) + "_" + std::string(materialName));
    std::replace(meshName.begin(), meshName.end(), ':', '_');
    std::replace(meshName.begin(), meshName.end(), '.', '_');

    std::shared_ptr<Mesh> newMesh = std::make_shared<Mesh>();
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
        Mesh::Vertex vertex{};

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;
        if (mesh->mTangents)
        {
            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;
        }
        else
        {
            LogMessage(meshName + " is missing tangents!");
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


    Render::CreateVertexAndIndexBuffer(newMesh.get());

    meshObject->AddComponent(std::make_shared<MeshComponent>(meshObject, newMesh));
    meshObject->GetMatrix() = gameObject->GetMatrix();
    meshObject->SetParent(gameObject);
    newMesh->CalculateBoundingBox();



    if (materialName != "")
    {
        newMesh->SetMaterial(MaterialManager::GetInstance()->GetMaterial(std::string(std::string(filename) + "_" + std::string(materialName))));
    }
    else
    {
        newMesh->SetMaterial(MaterialManager::GetInstance()->GetDefaultMaterial());
    }

    //aiString textureFilename;         IMPLEMENT THIS
    //material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilename);

    newMesh->SetName(meshName);
    ModelManager::GetInstance()->AddMesh(meshName, newMesh);

    auto DropfileTask = Concurrency::create_task(
        [newMesh]()
    {
        IOManager::SaveSpectralModel(newMesh);
    }
    );
}

void IOManager::ProcessNode(const std::string& filename, aiNode* node, const aiScene* scene, GameObject* parent, const Math::Matrix& accTransform) 
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
        newGameObject->GetMatrix() = matrix * accTransform ;
        for (UINT i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(filename, mesh, scene, newGameObject);
        }
        newGameObject->SetParent(parent);
        parent = newGameObject;
    }
    else {
        matrix = matrix * accTransform;
    }

    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(filename, node->mChildren[i], scene, parent, matrix);
    }
}