#include "Shader/ShaderObject.h"

#include "D3D12/D3D12Util.h"

ShaderObject::ShaderObject()
{
}

ShaderObject::~ShaderObject()
{
}

// コンパイル
void ShaderObject::compile(const wchar_t* filePathStr, const wchar_t* targetProfileStr)
{
	// コンパイル
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> errBlob;
	hr = D3D12Util::CompileShaderFromFile(filePathStr, targetProfileStr, mBlob, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
	}

	// バイトコード
	mByteCode = CD3DX12_SHADER_BYTECODE(mBlob.Get());
}

// シェーダーバイトコード
const CD3DX12_SHADER_BYTECODE& ShaderObject::getShaderByteCode() const
{
	return mByteCode;
}
