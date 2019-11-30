#include "pch.h"
#include "Util/D3D12Util.h"

#include <fstream>
#include <exception>
#include <stdexcept>
#include <experimental/filesystem>

namespace D3D12Util {

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

/// バッファ作成
ComPtr<ID3D12Resource> CreateBuffer(ID3D12Device* device, size_t bufferSize, const void* initialData)
{
	HRESULT hr;

	ComPtr<ID3D12Resource> buffer;
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&buffer)
	);

	// 初期データがあればコピー
	if (SUCCEEDED(hr) && initialData)
	{
		void* mapped;
		CD3DX12_RANGE range(0, 0);
		hr = buffer->Map(0, &range, &mapped);
		if (SUCCEEDED(hr))
		{
			memcpy(mapped, initialData, bufferSize);
			buffer->Unmap(0, nullptr);
		}
	}

	return buffer;
}

/// シェーダーコンパイル
HRESULT CompileShaderFromFile(const std::wstring& filePath, const std::wstring& profile, ComPtr<ID3DBlob>& shaderBlob, ComPtr<ID3DBlob>& errorBlob)
{
	using namespace std::experimental::filesystem;

	path path(filePath);
	std::ifstream infile(path);
	std::vector <char> srcData;
	if (!infile)
	{
		throw std::runtime_error("Shader not found.");
	}
	srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
	infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

	// DXCによるコンパイル
	ComPtr<IDxcLibrary> library;
	ComPtr<IDxcCompiler> compiler;
	ComPtr<IDxcBlobEncoding> source;
	ComPtr<IDxcOperationResult> dxcResult;

	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	library->CreateBlobWithEncodingFromPinned(srcData.data(), UINT(srcData.size()), CP_ACP, &source);
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	LPCWSTR compilerFlags[] = {
#if _DEBUG
		L"/Zi", L"/O0",
#else 
		L"/O2" // リリースでは最適化
#endif
	};
	compiler->Compile(
		source.Get(),
		path.wstring().c_str(),
		L"main",
		profile.c_str(),
		compilerFlags,
		_countof(compilerFlags),
		nullptr,
		0,
		nullptr,
		&dxcResult
	);

	HRESULT hr;
	dxcResult->GetStatus(&hr);
	if (SUCCEEDED(hr))
	{
		dxcResult->GetResult(
			reinterpret_cast<IDxcBlob * *>(shaderBlob.GetAddressOf())
		);
	}
	else
	{
		dxcResult->GetErrorBuffer(
			reinterpret_cast<IDxcBlobEncoding * *>(errorBlob.GetAddressOf())
		);
	}

	return hr;
}

} // namespace D3D12Util