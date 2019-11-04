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
		/// RenderTargetView
		UINT numRenderTargets;						///< �����_�[�^�[�Q�b�g��
		DXGI_FORMAT rtvFormats[8];					///< RTV�t�H�[�}�b�g
		/// DepthStencilView
		DXGI_FORMAT dsvFormat;						///< DSV�t�H�[�}�b�g
		/// �V�F�[�_�[
		Microsoft::WRL::ComPtr<ID3DBlob> ps;		///< �s�N�Z���V�F�[�_�[
		Microsoft::WRL::ComPtr<ID3DBlob> signature;	///< �V�O�l�`��

		SetupParam()
			: numRenderTargets(1)
			, dsvFormat(DXGI_FORMAT_D32_FLOAT)
			, ps(nullptr)
			, signature(nullptr)
		{
			for (int i = 0; i < 8; ++i)
			{
				rtvFormats[i] = DXGI_FORMAT_UNKNOWN;
			}
			rtvFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
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
	/// ComPtr
	template <typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	/// ���_
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 uv;
	};

	/// �C���f�b�N�X
	using Index = uint16_t;

private:
	ComPtr<ID3D12Resource> m_vb;
	ComPtr<ID3D12Resource> m_ib;

	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW m_ibView;

	ComPtr<ID3DBlob> m_vs, m_ps;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipeline;
};