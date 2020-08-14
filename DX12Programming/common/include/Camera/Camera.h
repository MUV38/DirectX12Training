#pragma once

#include <DirectXMath.h>

//! @brief �J����
class Camera
{
public:
	Camera();
	~Camera();

	// ---View---
	//! @brief �ʒu
	DirectX::XMFLOAT3 getEye() const;
	void setEye(const DirectX::XMFLOAT3& eye);
	
	//! @brief �����_
	DirectX::XMFLOAT3 getAt() const;
	void setAt(const DirectX::XMFLOAT3& at);
	
	//! @brief �����
	DirectX::XMFLOAT3 getUp() const;
	void setUp(const DirectX::XMFLOAT3& up);

	// ---projection---
	//! @brief �A�X�y�N�g��
	float getAspectRatio() const;
	void setAspectRatio(float aspectRatio);

	//! @brief �j�A�N���b�v
	float getNear() const;
	void setNear(float Near);

	//! @brief �t�@�[�N���b�v
	float getFar() const;
	void setFar(float Far);

private:
	DirectX::XMFLOAT3 mEye; //!< �ʒu
	DirectX::XMFLOAT3 mAt;	//!< �����_
	DirectX::XMFLOAT3 mUp;	//!< �����

	float mAspectRatio;	//!< �A�X�y�N�g��
	float mNear;		//!< �j�A�N���b�v
	float mFar;			//!< �t�@�[�N���b�v
};