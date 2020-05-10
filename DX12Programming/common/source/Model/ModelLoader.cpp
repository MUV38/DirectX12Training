#include "Model/ModelLoader.h"

#undef min
#undef max

#include <vector>

#include <core/windows/myWindows.h>
#include <core/string/stringUtil.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model/Polygon.h"

ModelLoader::ModelLoader()
    : m_isLoaded(false)
{
}

ModelLoader::~ModelLoader()
{
}

bool ModelLoader::Load(ID3D12Device* device, const wchar_t* filePath)
{
    Assimp::Importer importer;

    std::string filePathStr = util::ToString(filePath);

    const aiScene* scene = importer.ReadFile(
        filePathStr.c_str(),
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_ConvertToLeftHanded
    );

    if (scene)
    {
        // DoTheSceneProcessing

        // ノード
        ProcessNode(device, scene->mRootNode, scene);
        // マテリアル
        for (size_t i = 0; i < scene->mNumMaterials; ++i)
        {
            ProcessMaterial(device, scene->mMaterials[i], scene);
        }
    }
    else
    {
        OutputDebugStringA(importer.GetErrorString());
        return false;
    }

    m_isLoaded = true;

    return true;
}

bool ModelLoader::IsLoaded() const
{
    return m_isLoaded;
}

void ModelLoader::Draw(ID3D12GraphicsCommandList* commandList)
{
    if (!IsLoaded()) { return; }

    for (size_t i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i]->Draw(commandList);
    }
}

void ModelLoader::ProcessNode(ID3D12Device* device, aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        auto* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(device, mesh, scene));
    }

    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(device, node->mChildren[i], scene);
    }
}

ModelLoader::MeshPtr ModelLoader::ProcessMesh(ID3D12Device* device, aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    std::vector<Mesh::Index> indices;

    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        
        vertex.pos.x = mesh->mVertices[i].x;
        vertex.pos.y = mesh->mVertices[i].y;
        vertex.pos.z = mesh->mVertices[i].z;

        if (mesh->mTextureCoords[0])
        {
            vertex.uv.x = static_cast<float>(mesh->mTextureCoords[0][i].x);
            vertex.uv.y = static_cast<float>(mesh->mTextureCoords[0][i].y);
        }

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;

        vertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        const auto& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    MeshPtr outMesh(new Mesh(device, vertices, indices));
    return outMesh;
}

ModelLoader::MaterialPtr ModelLoader::ProcessMaterial(ID3D12Device* device, aiMaterial* material, const aiScene* scene)
{
    MaterialPtr outMaterials(new Material());
    outMaterials->setName(material->GetName().C_Str());

    uint32_t noneCount = material->GetTextureCount(aiTextureType_NONE);
    uint32_t diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
    uint32_t specularCount = material->GetTextureCount(aiTextureType_SPECULAR);
    uint32_t ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
    uint32_t emissiveCount = material->GetTextureCount(aiTextureType_EMISSIVE);
    uint32_t heightCount = material->GetTextureCount(aiTextureType_HEIGHT);
    uint32_t normalsCount = material->GetTextureCount(aiTextureType_NORMALS);
    uint32_t shininessCount = material->GetTextureCount(aiTextureType_SHININESS);
    uint32_t opacityCount = material->GetTextureCount(aiTextureType_OPACITY);
    uint32_t displacementCount = material->GetTextureCount(aiTextureType_DISPLACEMENT);
    uint32_t lightMapCount = material->GetTextureCount(aiTextureType_LIGHTMAP);
    uint32_t reflectionCount = material->GetTextureCount(aiTextureType_REFLECTION);
    uint32_t unknownCount = material->GetTextureCount(aiTextureType_UNKNOWN);

    return outMaterials;
}
