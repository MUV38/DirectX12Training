#pragma once

#include <D3D12/D3D12AppBase.h>
#include <Util/D3D12Util.h>

namespace GlobalRootSignatureParams {
	enum Value {
		OutputViewSlot = 0,
		AccelerationStructureSlot,
		Count
	};
}

namespace LocalRootSignatureParams {
	enum Value {
		ViewportConstantSlot = 0,
		Count
	};
}

class App : public D3D12AppBase
{
public:
	App();
	~App();

	virtual void Update() override;

	virtual void Prepare() override;
	virtual void Cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:
	struct Viewport
	{
		float left;
		float top;
		float right;
		float bottom;
	};

	struct RayGenConstantBuffer
	{
		Viewport viewport;
		Viewport stencil;
	};

private:
	/// DXR�T�|�[�g���`�F�b�N
	bool IsDirectXRaytracingSuppoted(IDXGIAdapter* adapter) const;
	/// DXR�C���^�[�t�F�[�X�쐬
	void CreateRaytracingInterfaces();
	/// RootSignature�쐬
	void CreateRootSignatures();
	/// ���C�g���[�V���OPSO�쐬
	void CreateRaytracingPipelineStateObject();
	/// �W�I���g���\�z
	void BuildGeometry();
	/// AccelerationStructure�\�z
	void BuildAccelerationStructures();
	/// �V�F�[�_�[�e�[�u���\�z
	void BuildShaderTables();
	/// ���C�g���[�V���O�o�̓��\�[�X�쐬
	void CreateRaytracingOutputResource();

	/// RootSignature�̃V���A���C�Y�ƍ쐬
	void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
	/// Local RootSignature�̃T�u�I�u�W�F�N�g�쐬�ƃV�F�[�_�[�֘A�t��
	void CreateLocalRootSignatureSubobjects(CD3DX12_STATE_OBJECT_DESC* raytracingPipeline);
private:
	ComPtr<ID3D12Device5> m_dxrDevice;
	ComPtr<ID3D12GraphicsCommandList4> m_dxrCommandList;
	ComPtr<ID3D12StateObject> m_dxrStateObject;

	ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
	ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

	RayGenConstantBuffer m_rayGenCB;

	DescriptorHandle m_uavDescriptorHandle;

	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_vertexBuffer;

	ComPtr<ID3D12Resource> m_accelerationStructure;
	ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
	ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;

	ComPtr<ID3D12Resource> m_missShaderTable;
	ComPtr<ID3D12Resource> m_hitGroupShaderTable;
	ComPtr<ID3D12Resource> m_rayGenShaderTable;

	ComPtr<ID3D12Resource> m_raytracingOutput;
};
