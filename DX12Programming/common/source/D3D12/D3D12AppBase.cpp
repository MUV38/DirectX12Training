#include <fstream>
#include <exception>
#include <stdexcept>
#include <experimental/filesystem>
#include "D3D12/D3D12AppBase.h"
#include "Util/D3D12Util.h"
#include "Util/Assert.h"

D3D12AppBase::D3D12AppBase()
    : m_frameIndex(0)
{
	m_backBufferRenderTargets.resize(FrameBufferCount);
    m_frameFenceValues.resize(FrameBufferCount);

    m_fenceWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

D3D12AppBase::~D3D12AppBase()
{
}

// 初期化
void D3D12AppBase::Initialize(HWND hWnd)
{
    HRESULT hr;

    // DirectXTexのための初期化
#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
    Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
    if (FAILED(initialize))
    {
        throw std::runtime_error("RoInitializeWrapper failed.");
    }
#else // (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(initialize))
    {
        throw std::runtime_error("CoInitializeEx() failed.");
    }
#endif //  (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)

    // デバッグ
    UINT dxgiFlags = 0;
#if (_DEBUG)
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();
        dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;

#if 0 // GBVを有効かする場合
        ComPtr<ID3D12Debug3> debug3;
        debug.As(&debug3);
        if (debug3)
        {
            debug3->SetEnableGPUBasedValidation(true);
        }
#endif // 0
    }
#endif // (_DEBUG)

    // DXGIFactory
    ComPtr<IDXGIFactory3> factory;
    hr = CreateDXGIFactory2(dxgiFlags, IID_PPV_ARGS(&factory));
    if (FAILED(hr))
    {
        throw std::runtime_error("CreateDXGIFactory2 failed.");
    }

    // ハードウェアアダプタの検索
    {
        UINT adapterIndex = 0;
        ComPtr<IDXGIAdapter1> adapter;
        while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
        {
            DXGI_ADAPTER_DESC1 desc1{};
            adapter->GetDesc1(&desc1);
            ++adapterIndex;

            // WARPでないかチェック
            if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            // Deviceが作成できかチェック
            hr = D3D12CreateDevice(
                adapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                __uuidof(ID3D12Device), nullptr
            );
            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        adapter.As(&m_adapter);
    }

    // Device
    hr = D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
    if (FAILED(hr))
    {
        throw std::runtime_error("D3D12CreateDevice failed.");
    }

    // CommandQueue
    D3D12_COMMAND_QUEUE_DESC queueDesc
    {
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        0,
        D3D12_COMMAND_QUEUE_FLAG_NONE,
        0
    };
    hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr))
    {
        throw std::runtime_error("CreateCommandQueue failed.");
    }

    // HWNDからサイズ取得
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // SwapChain
    {
        DXGI_SWAP_CHAIN_DESC1 scDesc{};
        scDesc.BufferCount = FrameBufferCount;
        scDesc.Width = width;
        scDesc.Height = height;
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        hr = factory->CreateSwapChainForHwnd(
            m_commandQueue.Get(),
            hWnd,
            &scDesc,
            nullptr,
            nullptr,
            &swapChain
        );
        if (FAILED(hr))
        {
            throw std::runtime_error("CreateSwapChainForHwnd failed.");
        }
        swapChain.As(&m_swapChain);
    }

    // DescripterHeap
    PrepareDescriptorHeaps();
    // RenderTarget
    PrepareRenderTargetView();
    // DepthBuffer
    CreateDepthBuffer(width, height);
    // CommandAllocator
    CreateCommandAllocators();
    // Fence
    CreateFrameFences();

    // CommandList
    hr = m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&m_commandList)
    );
    m_commandList->Close();

    m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(width), float(height));
    m_scissorRect = CD3DX12_RECT(0, 0, LONG(width), LONG(height));

    // imgui
    {
        m_imguiSrvDescriptorHandle = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::CbvSrvUav);
        m_imgui.Init(
            &hWnd,
            m_device.Get(),
            FrameBufferCount,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            m_imguiSrvDescriptorHandle.GetHeap(),
            m_imguiSrvDescriptorHandle.GetCPUHandle(),
            m_imguiSrvDescriptorHandle.GetGPUHandle()
        );
    }

    OnInitialize();
}

// 終了
void D3D12AppBase::Finalize()
{
    OnFinalize();

    m_imgui.Shutdown();
}

// 更新
void D3D12AppBase::Update()
{
    m_imgui.NewFrame();

    OnUpdate();
}

// 描画
void D3D12AppBase::Render()
{
    // 描画開始
    BeginRender();

    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_backBufferRtvDescriptorHandle[m_frameIndex].GetCPUHandle();
    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_backBufferDsvDescriptorHandle.GetCPUHandle();

    // レンダーターゲットクリア
    const float clearColor[] = { 0.3f, 0.3f, 0.3f, 0.0f };
    m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    // デプスバッファのクリア
    m_commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 描画対象の設定
    m_commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    OnRender(m_commandList);

    // imgui
    {
        m_commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
        m_imgui.Render(m_commandList.Get());
    }

    EndRender();
}

void D3D12AppBase::PrepareDescriptorHeaps()
{
	// デスクリプターマネージャ初期化
	if (!m_descriptorManager.Init(m_device.Get(), 5000, 100, 100, 100))
	{
		throw std::runtime_error("Failed DescriptorManager::Init");
	}

	for (int i=0 ; i<FrameBufferCount ; ++i)
	{
		// RTV
		m_backBufferRtvDescriptorHandle[i] = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::Rtv);
	}

	// DSV
	m_backBufferDsvDescriptorHandle = m_descriptorManager.Alloc(DescriptorManager::DescriptorPoolType::Dsv);
}

void D3D12AppBase::PrepareRenderTargetView()
{
    // スワップチェインイメージへのレンダーターゲットビュー生成
    for (UINT i = 0; i < FrameBufferCount; ++i)
    {
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferRenderTargets[i]));
        m_device->CreateRenderTargetView(m_backBufferRenderTargets[i].Get(), nullptr, m_backBufferRtvDescriptorHandle[i].GetCPUHandle());
    }
}

void D3D12AppBase::CreateDepthBuffer(int width, int height)
{
    auto depthBufferDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_D32_FLOAT,
        width,
        height,
        1, 0,
        1, 0,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
    );
    D3D12_CLEAR_VALUE depthClearValue {};
    depthClearValue.Format = depthBufferDesc.Format;
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.DepthStencil.Stencil = 0;

    HRESULT hr;
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &depthBufferDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue,
        IID_PPV_ARGS(&m_depthBuffer)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed CreateCommittedResource(DepthBuffer)");
    }

    // DSV
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc
    {
        DXGI_FORMAT_D32_FLOAT,
        D3D12_DSV_DIMENSION_TEXTURE2D,
        D3D12_DSV_FLAG_NONE,
        { // D3D12_TEX2D_DSV
            0
        }
    };
    m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, m_backBufferDsvDescriptorHandle.GetCPUHandle());
}

void D3D12AppBase::CreateCommandAllocators()
{
    HRESULT hr;
    m_commandAllocators.resize(FrameBufferCount);
    for (UINT i = 0; i < FrameBufferCount; ++i)
    {
        hr = m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocators[i])
        );
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed CreateCommandAllocator");
        }
    }
}

void D3D12AppBase::CreateFrameFences()
{
    HRESULT hr;
    m_frameFences.resize(FrameBufferCount);
    for (UINT i = 0; i < FrameBufferCount; ++i)
    {
        hr = m_device->CreateFence(
            0, // 初期値
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_frameFences[i])
        );
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed CreateFence");
        }
    }
}

// GPU待ち
void D3D12AppBase::WaitForGPU()
{
    // GPUが設定する値を設定
    auto& fence = m_frameFences[m_frameIndex];
    const auto currentValue = ++m_frameFenceValues[m_frameIndex];
    m_commandQueue->Signal(fence.Get(), currentValue);

    // GPU処理が完了しているかチェック
    auto nextIndex = (m_frameIndex + 1) % FrameBufferCount;
    const auto finishExpected = m_frameFenceValues[nextIndex];
    const auto nextFenceValue = m_frameFences[nextIndex]->GetCompletedValue();
    if (nextFenceValue < finishExpected)
    {
        // GPUの処理を待つ
        m_frameFences[nextIndex]->SetEventOnCompletion(finishExpected, m_fenceWaitEvent);
        WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
    }
}

// アダプター
D3D12AppBase::ComPtr<IDXGIAdapter1>& D3D12AppBase::GetAdapter()
{
    return m_adapter;
}

// デバイス
D3D12AppBase::ComPtr<ID3D12Device>& D3D12AppBase::GetDevice()
{
    return m_device;
}

// コマンドキュー
D3D12AppBase::ComPtr<ID3D12CommandQueue>& D3D12AppBase::GetCommandQueue()
{
    return m_commandQueue;
}

// スワップチェイン
D3D12AppBase::ComPtr<IDXGISwapChain4>& D3D12AppBase::GetSwapChain()
{
    return m_swapChain;
}

// バックバッファのレンダーターゲット取得
D3D12AppBase::ComPtr<ID3D12Resource>& D3D12AppBase::GetBackBufferRenderTarget()
{
    return m_backBufferRenderTargets[m_frameIndex];
}
D3D12AppBase::ComPtr<ID3D12Resource>& D3D12AppBase::GetBackBufferRenderTarget(uint32_t index)
{
    ASSERT(index < m_backBufferRenderTargets.size());

    return m_backBufferRenderTargets[index];
}

// コマンドリスト
D3D12AppBase::ComPtr<ID3D12GraphicsCommandList>& D3D12AppBase::GetCommandList()
{
    return m_commandList;
}

// コマンドアロケータ
D3D12AppBase::ComPtr<ID3D12CommandAllocator>& D3D12AppBase::GetCommandAllocator()
{
    return m_commandAllocators[m_frameIndex];
}
D3D12AppBase::ComPtr<ID3D12CommandAllocator>& D3D12AppBase::GetCommandAllocator(uint32_t index)
{
    ASSERT(index < m_commandAllocators.size());

    return m_commandAllocators[index];
}

// フレームインデックス
UINT D3D12AppBase::GetFrameIndex() const
{
    return m_frameIndex;
}

// ビューポート
const CD3DX12_VIEWPORT& D3D12AppBase::GetViewport() const
{
    return m_viewport;
}

// シザー矩形
const CD3DX12_RECT& D3D12AppBase::GetScissorRect() const
{
    return m_scissorRect;
}

// デスクリプターマネージャー
DescriptorManager& D3D12AppBase::GetDescriptorManager()
{
    return m_descriptorManager;
}

// バックバッファのRTVハンドル
const DescriptorHandle& D3D12AppBase::GetBackBufferRtvDescriptorHandle() const
{
    return m_backBufferRtvDescriptorHandle[m_frameIndex];
}
const DescriptorHandle& D3D12AppBase::GetBackBufferRtvDescriptorHandle(uint32_t index) const
{
    ASSERT(index < FrameBufferCount);

    return m_backBufferRtvDescriptorHandle[index];
}

// バックバッファのDSVハンドル
const DescriptorHandle& D3D12AppBase::GetBackBufferDsvDescriptorHandle() const
{
    return m_backBufferDsvDescriptorHandle;
}

// 描画開始
void D3D12AppBase::BeginRender()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_commandAllocators[m_frameIndex]->Reset();
    m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

    // スワップチェイン表示可能からレンダーターゲット描画可能へ
    auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
        m_backBufferRenderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &barrierToRT);
}

// 描画終了
void D3D12AppBase::EndRender()
{
    // レンダーターゲット描画可能からスワップチェイン表示可能へ
    auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        m_backBufferRenderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    m_commandList->ResourceBarrier(1, &barrierToPresent);

    m_commandList->Close();

    // コマンドリスト実行
    ID3D12CommandList* lists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(static_cast<UINT>(_countof(lists)), lists);

    m_swapChain->Present(1, 0);

    WaitForGPU();
}
