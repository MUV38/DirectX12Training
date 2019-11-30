#pragma once

#include "DescriptorPool.h"

/**
 * @brief �f�X�N���v�^�[�}�l�[�W���[
 */
class DescriptorManager
{
public:
	DescriptorManager();
	~DescriptorManager();

public:
	/**
	 * @brief �f�X�N���v�^�[�v�[���^�C�v
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
	 * @brief ������
	 */
	bool Init(ID3D12Device* device, UINT numCbvSrvUavPool, UINT numSamplerPool, UINT numRtvPool, UINT numDsvPool);

	/**
	 * @brief ���
	 */
	void Release();

	/**
	 * @brief �f�X�N���v�^�[�m��
	 */
	DescriptorHandle Alloc(DescriptorPoolType type);

	/**
	 * @brief �f�X�N���v�^�[���
	 */
	void Free(DescriptorPoolType type, const DescriptorHandle& handle);

	/**
	 * @brief �f�X�N���v�^�[�v�[���擾
	 */
	DescriptorPool* GetDescriptorPool(DescriptorPoolType type) { return &m_descriptorPool[static_cast<int>(type)]; }

private:
	DescriptorPool m_descriptorPool[NumDescriptorPoolType];
};