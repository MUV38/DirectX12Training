#pragma once

#include "D3D12/D3D12AppBase.h"

class App : public D3D12AppBase
{
public:
	App();
	~App();

	virtual void OnInitialize() override;
	virtual void OnFinalize() override;
	virtual void OnUpdate() override;
	virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:

};
