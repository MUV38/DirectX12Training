#pragma once

#include "DescriptorPool.h"

/**
 * @brief デスクリプターマネージャー
 */
class DescriptorManager
{
public:
	DescriptorManager();
	~DescriptorManager();

public:
	/**
	 * @brief デスクリプタープールタイプ
	 */
	enum class DescriptorPoolType
	{
		CbvSrvUav,
		Sampler,
		Rtv,
		Dsv,

		Num
	};
	const static int NumDescriptorPoolType = static_cast<int>(DescriptorPoolType::Num);

	/**
	 * @brief 初期化
	 */
	bool Init(ID3D12Device* device, UINT numCbvSrvUavPool, UINT numSamplerPool, UINT numRtvPool, UINT numDsvPool);

	/**
	 * @brief 解放
	 */
	void Release();

	/**
	 * @brief デスクリプター確保
	 */
	DescriptorHandle Alloc(DescriptorPoolType type);

	/**
	 * @brief デスクリプター解放
	 */
	void Free(DescriptorPoolType type, const DescriptorHandle& handle);

	/**
	 * @brief デスクリプタープール取得
	 */
	DescriptorPool* GetDescriptorPool(DescriptorPoolType type) { return &m_descriptorPool[static_cast<int>(type)]; }

private:
	DescriptorPool m_descriptorPool[NumDescriptorPoolType];
};