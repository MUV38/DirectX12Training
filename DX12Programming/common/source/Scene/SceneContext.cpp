#include "Scene/SceneContext.h"

SceneContext::SceneContext()
{
}

SceneContext::~SceneContext()
{
}

// ビュー行列
DirectX::XMFLOAT4X4 SceneContext::getViewMatrix() const
{
	return mViewMtx;
}
void SceneContext::setViewMatrix(const DirectX::XMFLOAT4X4& viewMtx)
{
	mViewMtx = viewMtx;
}

// プロジェクション行列
DirectX::XMFLOAT4X4 SceneContext::getProjectionMatrix() const
{
	return mProjectionMtx;
}
void SceneContext::setProjectionMatrix(const DirectX::XMFLOAT4X4& projectionMtx)
{
	mProjectionMtx = projectionMtx;
}
