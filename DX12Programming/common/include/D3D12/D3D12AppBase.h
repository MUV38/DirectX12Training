#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorManager.h"
#include "ImGui/ImGuiDx12.h"

class D3D12AppBase
{
public:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static const UINT FrameBufferCount = 2;    ///< バッファリング数
    static const UINT GpuWaitTimeout = (10 * 1000);    /// 10s

public:
    D3D12AppBase();
    virtual ~D3D12AppBase();

    void Initialize(HWND hWnd);
    void Terminate();

	virtual void Update();
    virtual void Render();

    /// override in subclass
    virtual void Prepare() {}
    virtual void Cleanup() {}
    virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) {}

protected:
    virtual void PrepareDescriptorHeaps();
    void PrepareRenderTargetView();
    void CreateDepthBuffer(int width, int height);
    void CreateCommandAllocators();
    void CreateFrameFences();
    void WaitPreviousFrame();

protected:
	ComPtr<IDXGIAdapter1> m_adapter;
    ComPtr<ID3D12Device> m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain4> m_swapChain;

    std::vector<ComPtr<ID3D12Resource>> m_backBufferRenderTargets;
	ComPtr<ID3D12Resource> m_depthBuffer;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    std::vector<ComPtr<ID3D12CommandAllocator>> m_commandAllocators;

    std::vector<ComPtr<ID3D12Fence1>> m_frameFences;
    std::vector<UINT64> m_frameFenceValues;
    HANDLE m_fenceWaitEvent;

    UINT m_frameIndex;

    CD3DX12_VIEWPORT  m_viewport;
    CD3DX12_RECT m_scissorRect;

	DescriptorManager m_descriptorManager;
	DescriptorHandle m_backBufferRtvDescriptorHandle[FrameBufferCount];
	DescriptorHandle m_backBufferDsvDescriptorHandle;

	ImGuiDx12 m_imgui;
	DescriptorHandle m_imguiSrvDescriptorHandle;
};