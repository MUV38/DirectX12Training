#include "pch.h"
#include "Texture/TextureLoader.h"

#include <stdexcept>
#include "Texture/Texture.h"

using namespace Microsoft::WRL;

// DDS読み込み
HRESULT TextureLoader::LoadDDS(
	ID3D12Device* device, 
	const wchar_t* filePath,
	ID3D12DescriptorHeap* heap, 
	INT offsetInDescriptors, 
	UINT descriptorIncrementSize,
	ID3D12CommandAllocator* commandAlocator,
	ID3D12CommandQueue* commandQueue,
	Texture* texture
)
{
	if (!device) { return E_FAIL; }
	if (!heap) { return E_FAIL; }
	if (!texture) { return E_FAIL; }

	ComPtr<ID3D12Resource> resource = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE srv;

	DirectX::TexMetadata metadata;
	auto image = std::make_unique<DirectX::ScratchImage>();
	HRESULT hr = DirectX::LoadFromDDSFile(
		filePath,
		DirectX::DDS_FLAGS_NONE,
		&metadata, *image
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("DirectXLoadFromDDSFile faild.");
		return hr;
	}

	hr = DirectX::CreateTexture(device, metadata, &resource);
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateTexture faild.");
		return hr;
	}

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	hr = DirectX::PrepareUpload(device, image->GetImages(), image->GetImageCount(), metadata, subresources);
	if (FAILED(hr))
	{
		throw std::runtime_error("PrepareUpload faild.");
	}

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(resource.Get(), 0, static_cast<unsigned int>(subresources.size()));

	ComPtr<ID3D12Resource> textureUploadHeap;
	hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(textureUploadHeap.GetAddressOf())
	);
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateCommittedResource faild.");
	}

	// コマンド準備
	ComPtr<ID3D12GraphicsCommandList> command;
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlocator, nullptr, IID_PPV_ARGS(&command));
	ComPtr<ID3D12Fence1> fence;
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	UpdateSubresources(command.Get(), resource.Get(), textureUploadHeap.Get(), 0, 0, static_cast<unsigned int>(subresources.size()), subresources.data());

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	command->ResourceBarrier(1, &barrier);

	command->Close();

	// コマンド実行
	ID3D12CommandList* cmds[] = { command.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmds), cmds);

	// 完了したらシグナルを立てる
	const UINT64 expected = 1;
	commandQueue->Signal(fence.Get(), expected);

	// テクスチャの処理が完了するまで待つ
	while (expected != fence->GetCompletedValue())
	{
		Sleep(1);
	}

	// texture srv.
	{
		auto srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heap->GetCPUDescriptorHandleForHeapStart(), offsetInDescriptors, descriptorIncrementSize);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		{
			srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);
			srvDesc.Format = metadata.format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		}
		device->CreateShaderResourceView(resource.Get(), &srvDesc, srvHandle);
		srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(heap->GetGPUDescriptorHandleForHeapStart(), offsetInDescriptors, descriptorIncrementSize);
	}

	// 出力に格納
	{
		texture->SetResource(resource.Get());
		texture->SetShaderResourceView(srv);
	}

	return hr;
}
