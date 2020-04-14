#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorHandle.h"
#include "Descriptor/DescriptorManager.h"

//! @brief コンスタントバッファ
class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	//! @brief 作成
	void Create(ID3D12Device* device, DescriptorManager* descriptorManager, size_t bufferSize);
	
	//! @brief リソース
	Microsoft::WRL::ComPtr<ID3D12Resource>& getResource();

	//! @brief デスクリプターハンドル
	const DescriptorHandle& getDescriptorHandle() const;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;	//!< リソース
	DescriptorHandle m_descriptorHandle;				//!< デスクリプターハンドル
};