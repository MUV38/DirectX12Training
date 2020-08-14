#include "Scene/SceneContext.h"

SceneContext::SceneContext()
{
}

SceneContext::~SceneContext()
{
}

// �r���[�s��
DirectX::XMFLOAT4X4 SceneContext::getViewMatrix() const
{
	return mViewMtx;
}
void SceneContext::setViewMatrix(const DirectX::XMFLOAT4X4& viewMtx)
{
	mViewMtx = viewMtx;
}

// �v���W�F�N�V�����s��
DirectX::XMFLOAT4X4 SceneContext::getProjectionMatrix() const
{
	return mProjectionMtx;
}
void SceneContext::setProjectionMatrix(const DirectX::XMFLOAT4X4& projectionMtx)
{
	mProjectionMtx = projectionMtx;
}
