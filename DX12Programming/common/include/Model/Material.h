#pragma once

//! @brief �}�e���A��
class Material
{
public:
	Material();
	~Material();

	//! @brief ���O
	const char* getName() const;
	void setName(const char* name);

private:
	// shader
	// texture
	// parameter
	// renderstate(pso)

	const char* mName;
};