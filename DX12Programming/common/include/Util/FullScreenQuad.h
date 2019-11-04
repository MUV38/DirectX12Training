#pragma once

#include "../framework.h"

/**
 * @brief �t���X�N���[���`��
 */
class FullScreenQuad
{
public:
	FullScreenQuad();
	~FullScreenQuad();

public:
	/**
	 * @brief �Z�b�g�A�b�v�p�����[�^
	 */
	struct SetupParam
	{

	};
	/**
	 * @brief �Z�b�g�A�b�v
	 * @param [in] param �Z�b�g�A�b�v�p�����[�^
	 * 
	 * �`�悷��܂łɂP��K���Ăяo��
	 */
	void Setup(ID3D12Device* device, const SetupParam& param);

	/**
	 * @brief �`��
	 * @param [in] commandLiast �R�}���h���X�g
	 */
	void Draw(ID3D12GraphicsCommandList* commandList);

private:
	/// ���_
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};
	/// �C���f�b�N�X
	using Index = uint16_t;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vb;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ib;

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;
};