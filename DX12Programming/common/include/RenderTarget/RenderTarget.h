#pragma once

#include "../framework.h"

/**
 * @brief �����_�[�^�[�Q�b�g�N���X
 */
class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();

public:
	/**
	 * @brief �Z�b�g�A�b�v�p�����[�^
	 */
	struct SetupParam
	{
		UINT width, height;
		DXGI_FORMAT format;

		ID3D12DescriptorHeap* rtvHeap;
		INT rtvOffset;

		ID3D12DescriptorHeap* srvHeap;
		INT srvOffset;
	};
	/**
	 * @brief �Z�b�g�A�b�v
	 * @param [in] device �f�o�C�X
	 * @param [in] param �Z�b�g�A�b�v�p�����[�^
	 */
	void Setup(ID3D12Device* device, const SetupParam& param);

	/**
	 * @brief ���\�[�X�o���A�^�C�v
	 */
	enum class ResourceBarrierType
	{
		RenderTarget,
		NonPixelShaderResource,
		PixelShaderResource,
	};
	/**
	 * @brief ���\�[�X�o���A�ݒu
	 * @param [in] commandList �R�}���h���X�g
	 * @param [in] type ���\�[�X�o���A�^�C�v
	 */
	void SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ResourceBarrierType type);

private:
	/// ���\�[�X�X�e�[�g�擾
	D3D12_RESOURCE_STATES GetResourceState(ResourceBarrierType type) const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_view;
	
	ResourceBarrierType m_resourceBarrierType;
};