#pragma once

#include "Application/Application.h"

class MyApp : public Application
{
public:
	MyApp();
	virtual ~MyApp();

	virtual void OnInitialize() override;
	virtual void OnFinalize() override;
	virtual void OnUpdate(float deltaTime) override;
	virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) override;

private:

};
