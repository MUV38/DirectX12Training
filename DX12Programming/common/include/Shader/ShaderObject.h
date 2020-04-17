#pragma once

#include "../framework.h"

//! @brief シェーダーオブジェクト
class ShaderObject
{
public:
	ShaderObject();
	~ShaderObject();

private:
	Microsoft::WRL::ComPtr<ID3DBlob> mBlob;
};