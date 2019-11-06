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

		ID3D12DescriptorHeap* srvUavHeap;
		INT srvOffset;
		INT uavOffset;

		SetupParam()
			: width(1)
			, height(1)
			, format(DXGI_FORMAT_R8G8B8A8_UNORM)
			, rtvHeap(nullptr)
			, rtvOffset(-1)
			, srvUavHeap(nullptr)
			, srvOffset(-1)
			, uavOffset(-1)
		{}
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

	/**
	 * @brief ���\�[�X�擾
	 * @return ���\�[�X
	 */
	ID3D12Resource* GetResource() const { return m_resource.Get(); }

	/**
	 * @brief ShaderResourceView�擾
	 * @return ShaderResourceView
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetShaderResourceView() const { return m_srv; }

	/**
	 * @brief UnorderedAccessView�擾
	 * @return UnorderedAccessView
	 */
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetUnorderedAccessView() const { return m_uav; }

private:
	/// ���\�[�X�X�e�[�g�擾
	D3D12_RESOURCE_STATES GetResourceState(ResourceBarrierType type) const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
	D3D12_GPU_DESCRIPTOR_HANDLE m_uav;
	
	ResourceBarrierType m_resourceBarrierType;
};