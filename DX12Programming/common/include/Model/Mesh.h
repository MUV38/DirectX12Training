#pragma once

#include <wrl.h>
#include <vector>
#include <d3d12.h>
#include "Polygon.h"

/// メッシュ
class Mesh
{
public:
    /// インデックス
    using Index = uint32_t;

public:
    Mesh(ID3D12Device* device, const std::vector<Vertex>& vertices, const std::vector<Index>& indices);
    ~Mesh();

    /// 描画
    void Draw(ID3D12GraphicsCommandList* commandList);

private:
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
    std::vector<Vertex> m_vertices;
    std::vector<Index> m_indices;

    ComPtr<ID3D12Resource> m_vb;
    ComPtr<ID3D12Resource> m_ib;

    D3D12_VERTEX_BUFFER_VIEW m_vbView;
    D3D12_INDEX_BUFFER_VIEW m_ibView;
};