#include "pch.h"
#include "Util/D3D12Util.h"

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
		memcpy(mapped, initialData, bufferSize);
		buffer->Unmap(0, nullptr);
	}

	return buffer;
}

} // namespace D3D12Util