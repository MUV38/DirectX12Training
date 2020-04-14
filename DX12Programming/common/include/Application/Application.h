#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorManager.h"
#include "ImGui/ImGuiDx12.h"

class Application
{
public:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static const UINT FrameBufferCount = 2;    ///< バッファリング数
    static const UINT GpuWaitTimeout = (10 * 1000);    /// 10s

public:
    Application();
    virtual ~Application();

    //! @brief 実行
    int Run(HWND hWnd);

protected:
    /// override in subclass
    virtual void OnInitialize() {}
    virtual void OnFinalize() {}
    virtual void OnUpdate() {}
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) {}

protected:
    //! @brief 初期化
    void Initialize(HWND hWnd);
    //! @brief 終了
    void Finalize();
    //! @brief 更新
    void Update();
    //! @brief 描画
    void Render();

    //! @brief GPU待ち
    void WaitForGPU();

    //! @brief アダプター
    ComPtr<IDXGIAdapter1>& GetAdapter();
    //! @brief デバイス
    ComPtr<ID3D12Device>& GetDevice();
    //! @brief コマンドキュー
    ComPtr<ID3D12CommandQueue>& GetCommandQueue();
    //! @brief スワップチェイン
    ComPtr<IDXGISwapChain4>& GetSwapChain();
    //! @brief バックバッファのレンダーターゲット
    ComPtr<ID3D12Resource>& GetBackBufferRenderTarget();
    ComPtr<ID3D12Resource>& GetBackBufferRenderTarget(uint32_t index);
    //! @brief コマンドリスト
    ComPtr<ID3D12GraphicsCommandList>& GetCommandList();
    //! @brief コマンドアロケータ
    ComPtr<ID3D12CommandAllocator>& GetCommandAllocator();
    ComPtr<ID3D12CommandAllocator>& GetCommandAllocator(uint32_t index);
    //! @brief フレームインデックス
    UINT GetFrameIndex() const;
    //! @brief ビューポート
    const CD3DX12_VIEWPORT& GetViewport() const;
    //! @brief シザー矩形
    const CD3DX12_RECT& GetScissorRect() const;
    //! @brief デスクリプターマネージャー
    DescriptorManager& GetDescriptorManager();
    //! @brief バックバッファのRTVハンドル
    const DescriptorHandle& GetBackBufferRtvDescriptorHandle() const;
    const DescriptorHandle& GetBackBufferRtvDescriptorHandle(uint32_t index) const;
    //! @brief バックバッファのDSVハンドル
    const DescriptorHandle& GetBackBufferDsvDescriptorHandle() const;

private:
    void PrepareDescriptorHeaps();
    void PrepareRenderTargetView();
    void CreateDepthBuffer(int width, int height);
    void CreateCommandAllocators();
    void CreateFrameFences();

    //! @brief 描画開始
    void BeginRender();
    //! @brief 描画終了
    void EndRender();

private:
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