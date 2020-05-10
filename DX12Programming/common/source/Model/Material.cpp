#include "Model/Material.h"

Material::Material()
{
}

Material::~Material()
{
}

// –¼‘O
const char* Material::getName() const
{
	return mName;
}
void Material::setName(const char* name)
{
	mName = name;
}
