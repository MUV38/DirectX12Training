#include "pch.h"
#include "Util/FullScreenQuad.h"

#include <stdexcept>
#include "Util/D3D12Util.h"

//-------------------------------------------------------------------------------------------------------------------
// FullScreenQuad
//-------------------------------------------------------------------------------------------------------------------
FullScreenQuad::FullScreenQuad()
{
}

FullScreenQuad::~FullScreenQuad()
{
}

void FullScreenQuad::Setup(ID3D12Device* device, const SetupParam& param)
{
	HRESULT hr;

	// ���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@�쐬
	{
		// reference: https://rootllama.wordpress.com/2015/02/12/drawing-a-full-screen-quad/
		const Vertex vertices[] = {
			{ { -1.0f,  1.0f, 0.5f }, { 0.0f, 0.0f } },
			{ {  3.0f,  1.0f, 0.5f }, { 2.0f, 0.0f } },
			{ { -1.0f, -3.0f, 0.5f }, { 0.0f, 2.0f } },
		};
		const size_t numVertex = _countof(vertices);

		const Index indices[] = { 0, 1, 2 };
		const size_t numIndex = _countof(indices);

		// �o�b�t�@�쐬
		m_vb = D3D12Util::CreateBuffer(device, sizeof(Vertex) * numVertex, &vertices[0]);
		m_ib = D3D12Util::CreateBuffer(device, sizeof(Index) * numIndex, &indices[0]);
		// �r���[�쐬
		m_vbView.BufferLocation = m_vb->GetGPUVirtualAddress();
		m_vbView.SizeInBytes = static_cast<UINT>(sizeof(Vertex) * numVertex);
		m_vbView.StrideInBytes = sizeof(Vertex);
		m_ibView.BufferLocation = m_ib->GetGPUVirtualAddress();
		m_ibView.SizeInBytes = static_cast<UINT>(sizeof(Index) * numIndex);
		m_ibView.Format = DXGI_FORMAT_R16_UINT;
	}

	// �V�F�[�_�[
	{
		ComPtr<ID3DBlob> errBlob;
		hr = D3D12Util::CompileShaderFromFile(L"../assets/shader/FullScreenQuad_vs.hlsl", L"vs_6_0", m_vs, errBlob);
		if (FAILED(hr))
		{
			OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
		}

		m_ps = param.psBlob;
		if (!m_ps)
		{
			hr = D3D12Util::CompileShaderFromFile(L"../assets/shader/FullScreenQuad_ps.hlsl", L"ps_6_0", m_ps, errBlob);
			if (FAILED(hr))
			{
				OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
			}
		}
	}

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA },
	};

	// RootSignature
	{
		auto signature = param.rootSignatureBlob;
		hr = device->CreateRootSignature(
			0,
			signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&m_rootSignature)
		);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateRootSignature failed");
		}
	}

	// PipelineStateObject
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	{
		// Shader.
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());
		// BlendState.
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		// RasterizerState.
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		// RenderTarget.
		psoDesc.NumRenderTargets = param.numRenderTargets;
		memcpy(psoDesc.RTVFormats, param.rtvFormats, sizeof(DXGI_FORMAT) * 8);
		// DepthStencil.
		CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
		{
			depthStencilDesc.DepthEnable = false;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		}
		psoDesc.DSVFormat = param.dsvFormat;
		psoDesc.DepthStencilState = depthStencilDesc;
		// InputLayout.
		psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

		// RootSignature.
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// MultiSample.
		psoDesc.SampleDesc = { 1, 0 };
		psoDesc.SampleMask = UINT_MAX; // �����Y���ƊG���o�Ȃ�&�x�����o�Ȃ�.
	}
	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateGraphicsPipelineState failed");
	}
}

void FullScreenQuad::SetPipelineState(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetPipelineState(m_pipeline.Get());
}

void FullScreenQuad::SetGraphicsRootSignature(ID3D12GraphicsCommandList* commandList)
{
	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
}

void FullScreenQuad::Draw(ID3D12GraphicsCommandList* commandList)
{
	// ���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@
	commandList->IASetVertexBuffers(0, 1, &m_vbView);
	commandList->IASetIndexBuffer(&m_ibView);
	// �g�|���W
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// �`��
	commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------
// FullScreenQuad::SetupParam
//-------------------------------------------------------------------------------------------------------------------
void FullScreenQuad::SetupParam::SetupRootSignatureOneTexture(ID3D12Device* device)
{
	HRESULT hr;

	// DescripterRange
	CD3DX12_DESCRIPTOR_RANGE srv, sampler;
	srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

	// RootParameter
	CD3DX12_ROOT_PARAMETER rootParams[2];
	rootParams[0].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);
	rootParams[1].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);

	// RootSignature
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		_countof(rootParams), rootParams,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	ComPtr<ID3DBlob> errBlob;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &errBlob);
	if (FAILED(hr))
	{
		throw std::runtime_error("D3D12SerializeRootSignature faild.");
	}
}