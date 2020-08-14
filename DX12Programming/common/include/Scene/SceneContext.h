#pragma once

#include <DirectXMath.h>

//! @brief �V�[���̃R���e�L�X�g
class SceneContext
{
public:
	SceneContext();
	~SceneContext();

	//! @brief �r���[�s��
	DirectX::XMFLOAT4X4 getViewMatrix() const;
	void setViewMatrix(const DirectX::XMFLOAT4X4& viewMtx);

	//! @brief �v���W�F�N�V�����s��
	DirectX::XMFLOAT4X4 getProjectionMatrix() const;
	void setProjectionMatrix(const DirectX::XMFLOAT4X4& projectionMtx);

private:
	DirectX::XMFLOAT4X4 mViewMtx;
	DirectX::XMFLOAT4X4 mProjectionMtx;
};