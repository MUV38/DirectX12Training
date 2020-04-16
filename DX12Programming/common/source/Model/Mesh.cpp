#include "Model/Mesh.h"
#include "D3D12/d3dx12.h"
#include "D3D12/D3D12Util.h"

Mesh::Mesh(ID3D12Device* device, const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
    : m_vertices(vertices)
    , m_indices(indices)
{
    // バッファ作成
    m_vb = D3D12Util::CreateBuffer(device, sizeof(Vertex) * vertices.size(), &vertices[0]);
    m_ib = D3D12Util::CreateBuffer(device, sizeof(Index) * indices.size(), &indices[0]);
    // ビュー作成
    m_vbView.BufferLocation = m_vb->GetGPUVirtualAddress();
    m_vbView.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * vertices.size());
    m_vbView.StrideInBytes = sizeof(Vertex);
    m_ibView.BufferLocation = m_ib->GetGPUVirtualAddress();
    m_ibView.SizeInBytes = static_cast<UINT>(sizeof(Index) * indices.size());
    m_ibView.Format = DXGI_FORMAT_R32_UINT;
}

Mesh::~Mesh()
{

}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList)
{
    commandList->IASetVertexBuffers(0, 1, &m_vbView);
    commandList->IASetIndexBuffer(&m_ibView);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT numIndices = static_cast<UINT>(m_indices.size());
    commandList->DrawIndexedInstanced(numIndices, 1, 0, 0, 0);
}