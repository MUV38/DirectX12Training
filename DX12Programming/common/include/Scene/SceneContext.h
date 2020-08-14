#pragma once

#include <DirectXMath.h>

//! @brief シーンのコンテキスト
class SceneContext
{
public:
	SceneContext();
	~SceneContext();

	//! @brief ビュー行列
	DirectX::XMFLOAT4X4 getViewMatrix() const;
	void setViewMatrix(const DirectX::XMFLOAT4X4& viewMtx);

	//! @brief プロジェクション行列
	DirectX::XMFLOAT4X4 getProjectionMatrix() const;
	void setProjectionMatrix(const DirectX::XMFLOAT4X4& projectionMtx);

private:
	DirectX::XMFLOAT4X4 mViewMtx;
	DirectX::XMFLOAT4X4 mProjectionMtx;
};