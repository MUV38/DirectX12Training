#include "pch.h"
#include "Texture.h"

Texture::Texture(ID3D12Resource* resource, D3D12_GPU_DESCRIPTOR_HANDLE srv)
	: m_resource(resource)
	, m_srv(srv)
{
}

Texture::~Texture()
{
}