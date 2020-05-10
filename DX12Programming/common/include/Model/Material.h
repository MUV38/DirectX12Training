#pragma once

//! @brief マテリアル
class Material
{
public:
	Material();
	~Material();

	//! @brief 名前
	const char* getName() const;
	void setName(const char* name);

private:
	// shader
	// texture
	// parameter
	// renderstate(pso)

	const char* mName;
};