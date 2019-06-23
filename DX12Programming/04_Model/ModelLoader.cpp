#include "ModelLoader.h"

#undef min
#undef max

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include "Polygon.h"

ModelLoader::ModelLoader()
    : m_isLoaded(false)
{
}

ModelLoader::~ModelLoader()
{
}

bool ModelLoader::Load(ID3D12Device* device, const char* filePath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        filePath,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_ConvertToLeftHanded
    );

    if (scene)
    {
        // DoTheSceneProcessing

        // ƒm[ƒh‚ðˆ—
        ProcessNode(device, scene->mRootNode, scene);
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
