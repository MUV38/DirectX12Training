#include "Shader/ShaderObject.h"

#include "D3D12/D3D12Util.h"

ShaderObject::ShaderObject()
{
}

ShaderObject::~ShaderObject()
{
}

// �R���p�C��
void ShaderObject::compile(const wchar_t* filePathStr, const wchar_t* targetProfileStr)
{
	// �R���p�C��
	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> errBlob;
	hr = D3D12Util::CompileShaderFromFile(filePathStr, targetProfileStr, mBlob, errBlob);
	if (FAILED(hr))
	{
		OutputDebugStringA(static_cast<const char*>(errBlob->GetBufferPointer()));
	}

	// �o�C�g�R�[�h
	mByteCode = CD3DX12_SHADER_BYTECODE(mBlob.Get());
}

// �V�F�[�_�[�o�C�g�R�[�h
const CD3DX12_SHADER_BYTECODE& ShaderObject::getShaderByteCode() const
{
	return mByteCode;
}
