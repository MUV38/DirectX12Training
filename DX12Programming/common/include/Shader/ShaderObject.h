#pragma once

#include "../framework.h"

//! @brief シェーダーオブジェクト
class ShaderObject
{
public:
	ShaderObject();
	~ShaderObject();
	
	//! @brief コンパイル
	void compile(const wchar_t* filePathStr, const wchar_t* targetProfileStr);

	//! @brief シェーダーバイトコード
	const CD3DX12_SHADER_BYTECODE& getShaderByteCode() const;

private:
	Microsoft::WRL::ComPtr<ID3DBlob> mBlob;
	CD3DX12_SHADER_BYTECODE mByteCode;
};