#pragma once

#include "../framework.h"

class Texture
{
public:
	Texture(ID3D12Resource* resource, D3D12_GPU_DESCRIPTOR_HANDLE srv);
	~Texture();

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_srv;
};