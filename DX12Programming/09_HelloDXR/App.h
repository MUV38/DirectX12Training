#pragma once

#include "D3D12/D3D12AppBase.h"

class App : public D3D12AppBase
{
public:
	App();
	~App();

	virtual void Update() override;

	virtual void Prepare() override;
	virtual void Cleanup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:

};
