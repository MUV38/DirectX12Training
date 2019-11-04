#include "pch.h"
#include "Util/FullScreenQuad.h"
#include "Util/D3D12Util.h"

FullScreenQuad::FullScreenQuad()
{
}

FullScreenQuad::~FullScreenQuad()
{
}

void FullScreenQuad::Setup(ID3D12Device* device, const SetupParam& param)
{
	// 頂点バッファ、インデックスバッファ作成
	{
		const Vertex vertices[] = {
			{ { -1.0f,  1.0f, 0.5f }, { -1.0f,  1.0f } },
			{ { -1.0f, -3.0f, 0.5f }, { -1.0f, -3.0f } },
			{ {  3.0f,  1.0f, 0.5f }, { -1.0f,  3.0f } },
		};
		const size_t numVertex = _countof(vertices);

		const Index indices[] = { 0, 1, 2 };
		const size_t numIndex = _countof(indices);

		// バッファ作成
		m_vb = D3D12Util::CreateBuffer(device, sizeof(Vertex) * numVertex, &vertices[0]);
		m_ib = D3D12Util::CreateBuffer(device, sizeof(Index) * numIndex, &indices[0]);
		// ビュー作成
		m_vbView.BufferLocation = m_vb->GetGPUVirtualAddress();
		m_vbView.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * numVertex);
		m_vbView.StrideInBytes = sizeof(Vertex);
		m_ibView.BufferLocation = m_ib->GetGPUVirtualAddress();
		m_ibView.SizeInBytes = static_cast<UINT>(sizeof(Index) * numIndex);
		m_ibView.Format = DXGI_FORMAT_R32_UINT;
	}
}

void FullScreenQuad::Draw(ID3D12GraphicsCommandList* commandList)
{
}
