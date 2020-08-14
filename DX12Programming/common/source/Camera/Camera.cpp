#include "Camera/Camera.h"

Camera::Camera()
	: mEye(0, 0, 10)
	, mAt(0, 0, 0)
	, mUp(0, 1, 0)
	, mAspectRatio(1920.0f / 1080.0f)
	, mNear(0.1f)
	, mFar(10000.0f)
{
}

Camera::~Camera()
{
}

// �ʒu
DirectX::XMFLOAT3 Camera::getEye() const
{
	return mEye;
}
void Camera::setEye(const DirectX::XMFLOAT3& eye)
{
	mEye = eye;
}

// �����_
DirectX::XMFLOAT3 Camera::getAt() const
{
	return mAt;
}
void Camera::setAt(const DirectX::XMFLOAT3& at)
{
	mAt = at;
}

// �����
DirectX::XMFLOAT3 Camera::getUp() const
{
	return mUp;
}
void Camera::setUp(const DirectX::XMFLOAT3& up)
{
	mUp = up;
}

// �A�X�y�N�g��
float Camera::getAspectRatio() const
{
	return mAspectRatio;
}
void Camera::setAspectRatio(float aspectRatio)
{
	mAspectRatio = aspectRatio;
}

// �j�A�N���b�v
float Camera::getNear() const
{
	return mNear;
}
void Camera::setNear(float Near)
{
	mNear = Near;
}

// �t�@�[�N���b�v
float Camera::getFar() const
{
	return mFar;
}
void Camera::setFar(float Far)
{
	mFar = Far;
}
