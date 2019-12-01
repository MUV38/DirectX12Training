#include "Texture/Texture.h"

Texture::Texture()
{
}

Texture::~Texture()
{
	Release();
}

void Texture::Init(ID3D12Resource* resource, DescriptorPool* descriptorPool, DescriptorHandle descriptorHandle)
{
	m_resource = resource;
	m_descriptorPool = descriptorPool;
	m_descriptorHandle = descriptorHandle;
}

void Texture::Release()
{
	m_resource = nullptr;
	if (m_descriptorPool)
	{
		m_descriptorPool->Free(m_descriptorHandle);
		m_descriptorPool = nullptr;
	}
}
