#pragma once

#include <DirectXMath.h>

//! @brief カメラ
class Camera
{
public:
	Camera();
	~Camera();

	// ---View---
	//! @brief 位置
	DirectX::XMFLOAT3 getEye() const;
	void setEye(const DirectX::XMFLOAT3& eye);
	
	//! @brief 注視点
	DirectX::XMFLOAT3 getAt() const;
	void setAt(const DirectX::XMFLOAT3& at);
	
	//! @brief 上方向
	DirectX::XMFLOAT3 getUp() const;
	void setUp(const DirectX::XMFLOAT3& up);

	// ---projection---
	//! @brief アスペクト比
	float getAspectRatio() const;
	void setAspectRatio(float aspectRatio);

	//! @brief ニアクリップ
	float getNear() const;
	void setNear(float Near);

	//! @brief ファークリップ
	float getFar() const;
	void setFar(float Far);

private:
	DirectX::XMFLOAT3 mEye; //!< 位置
	DirectX::XMFLOAT3 mAt;	//!< 注視点
	DirectX::XMFLOAT3 mUp;	//!< 上方向

	float mAspectRatio;	//!< アスペクト比
	float mNear;		//!< ニアクリップ
	float mFar;			//!< ファークリップ
};