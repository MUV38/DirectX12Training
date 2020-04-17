#pragma once

#include <d3d12.h>
#include <memory>
#include "Mesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;

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

private:
    void ProcessNode(ID3D12Device* device, aiNode* node, const aiScene* scene);
    MeshPtr ProcessMesh(ID3D12Device* device, aiMesh* mesh, const aiScene* scene);

private:
    bool m_isLoaded;
    std::vector<MeshPtr> m_meshes;
};