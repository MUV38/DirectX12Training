#pragma once

#include <d3d12.h>
#include <memory>
#include "Mesh.h"
#include "Material.h"

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;

/// ���f�����[�_�[
class ModelLoader
{
public:
    ModelLoader();
    ~ModelLoader();

    /// ���[�h
    bool Load(ID3D12Device* device, const wchar_t* filePath);

    /// ���[�h�ς݂�
    bool IsLoaded() const;

    /// �`��
    void Draw(ID3D12GraphicsCommandList* commandList);

private:
    using MeshPtr = std::unique_ptr<Mesh>;
    using MaterialPtr = std::unique_ptr<Material>;

private:
    void ProcessNode(ID3D12Device* device, aiNode* node, const aiScene* scene);
    MeshPtr ProcessMesh(ID3D12Device* device, aiMesh* mesh, const aiScene* scene);
    MaterialPtr ProcessMaterial(ID3D12Device* device, aiMaterial* material, const aiScene* scene);

private:
    bool m_isLoaded;
    std::vector<MeshPtr> m_meshes;
    std::vector<MaterialPtr> m_materials;
};