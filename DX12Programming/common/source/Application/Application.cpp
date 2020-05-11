#include <fstream>
#include <exception>
#include <stdexcept>
#include <experimental/filesystem>
#include <core/assert/assert.h>
#include "Application/Application.h"
#include "D3D12/D3D12Util.h"

Application::Application()
    : m_frameIndex(0)
    , mDeltaTime(0)
    , mShowAppInfo(true)
{
	m_backBufferRenderTargets.resize(FrameBufferCount);
    m_frameFenceValues.resize(FrameBufferCount);

    m_fenceWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

Application::~Application()
{
}

// 実行
int Application::Run(HWND hWnd)
{
    // 初期化
    Initialize(hWnd);

    try
    {
        MSG msg{};
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            // フレーム開始
            begingFrame();
            // 更新
            Update(getDeltaTime());
            // 描画
            Render();
            // フレーム終了
            endFrame();
        }

        // 終了処理
        Finalize();

        return static_cast<int>(msg.wParam);
    }
    catch (std::runtime_error e)
    {
        DebugBreak();
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }

    return 0;
}

// 初期化
void Application::Initialize(HWND hWnd)
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

    // タイマー
    {
        mElapsedTimer.start();
    }

    // 初期化イベント
    OnInitialize();
}

// 終了
void Application::Finalize()
{
    // 終了イベント
    OnFinalize();

    // imgui終了
    m_imgui.Shutdown();
}

// 更新
void Application::Update(float deltaTime)
{
    // imguiのフレーム開始
    m_imgui.NewFrame();
    
    // アプリケーション情報を表示
    showAppInfo();

    // 更新イベント
    OnUpdate(deltaTime);
}

// 描画
void Application::Render()
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

    // 描画イベント
    OnRender(m_commandList);

    // imgui
    {
        m_commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
        m_imgui.Render(m_commandList.Get());
    }

    // 描画終了
    EndRender();
}

void Application::PrepareDescriptorHeaps()
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

void Application::PrepareRenderTargetView()
{
    // スワップチェインイメージへのレンダーターゲットビュー生成
    for (UINT i = 0; i < FrameBufferCount; ++i)
    {
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBufferRenderTargets[i]));
        m_device->CreateRenderTargetView(m_backBufferRenderTargets[i].Get(), nullptr, m_backBufferRtvDescriptorHandle[i].GetCPUHandle());
    }
}

void Application::CreateDepthBuffer(int width, int height)
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

void Application::CreateCommandAllocators()
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

void Application::CreateFrameFences()
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
void Application::WaitForGPU()
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
Application::ComPtr<IDXGIAdapter1>& Application::GetAdapter()
{
    return m_adapter;
}

// デバイス
Application::ComPtr<ID3D12Device>& Application::GetDevice()
{
    return m_device;
}

// コマンドキュー
Application::ComPtr<ID3D12CommandQueue>& Application::GetCommandQueue()
{
    return m_commandQueue;
}

// スワップチェイン
Application::ComPtr<IDXGISwapChain4>& Application::GetSwapChain()
{
    return m_swapChain;
}

// バックバッファのレンダーターゲット取得
Application::ComPtr<ID3D12Resource>& Application::GetBackBufferRenderTarget()
{
    return m_backBufferRenderTargets[m_frameIndex];
}
Application::ComPtr<ID3D12Resource>& Application::GetBackBufferRenderTarget(uint32_t index)
{
    ASSERT(index < m_backBufferRenderTargets.size());

    return m_backBufferRenderTargets[index];
}

// コマンドリスト
Application::ComPtr<ID3D12GraphicsCommandList>& Application::GetCommandList()
{
    return m_commandList;
}

// コマンドアロケータ
Application::ComPtr<ID3D12CommandAllocator>& Application::GetCommandAllocator()
{
    return m_commandAllocators[m_frameIndex];
}
Application::ComPtr<ID3D12CommandAllocator>& Application::GetCommandAllocator(uint32_t index)
{
    ASSERT(index < m_commandAllocators.size());

    return m_commandAllocators[index];
}

// フレームインデックス
UINT Application::GetFrameIndex() const
{
    return m_frameIndex;
}

// ビューポート
const CD3DX12_VIEWPORT& Application::GetViewport() const
{
    return m_viewport;
}

// シザー矩形
const CD3DX12_RECT& Application::GetScissorRect() const
{
    return m_scissorRect;
}

// デスクリプターマネージャー
DescriptorManager& Application::GetDescriptorManager()
{
    return m_descriptorManager;
}

// バックバッファのRTVハンドル
const DescriptorHandle& Application::GetBackBufferRtvDescriptorHandle() const
{
    return m_backBufferRtvDescriptorHandle[m_frameIndex];
}
const DescriptorHandle& Application::GetBackBufferRtvDescriptorHandle(uint32_t index) const
{
    ASSERT(index < FrameBufferCount);

    return m_backBufferRtvDescriptorHandle[index];
}

// バックバッファのDSVハンドル
const DescriptorHandle& Application::GetBackBufferDsvDescriptorHandle() const
{
    return m_backBufferDsvDescriptorHandle;
}

// 経過時間取得
float Application::getElapsedTime() const
{
    return static_cast<float>(mElapsedTimer.getTime());
}

// デルタタイム取得
float Application::getDeltaTime() const
{
    return mDeltaTime;
}

// 描画開始
void Application::BeginRender()
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
void Application::EndRender()
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

    // スワップ
    m_swapChain->Present(1, 0);

    // GPU待ち
    WaitForGPU();
}

// フレーム開始
void Application::begingFrame()
{
    // デルタ時間取得
    mDeltaTimer.stop();
    mDeltaTime = static_cast<float>(mDeltaTimer.getTime());

    // フレームのタイマー開始
    mDeltaTimer.start();

    // フレーム開始イベント
    OnBeginFrame();
}

// フレーム終了
void Application::endFrame()
{
    // フレーム終了イベント
    OnEndFrame();
}

// アプリケーション情報を表示
void Application::showAppInfo()
{
    bool* isShow = &mShowAppInfo;
    if (!(*isShow))
    {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    const float distance = 10.0f;
    const float deltaTime = mDeltaTime;
    static int corner = 3;
    if (corner != -1)
    {
        ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - distance : distance, (corner & 2) ? io.DisplaySize.y - distance : distance);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("App Info", isShow, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("%.3f ms/frame (%.1f FPS)", deltaTime, 1000.0f / deltaTime);
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1)) 
            {
                corner = -1;
            }
            if (ImGui::MenuItem("Top-left", NULL, corner == 0))
            {
                corner = 0;
            }
            if (ImGui::MenuItem("Top-right", NULL, corner == 1))
            {
                corner = 1;
            }
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2))
            {
                corner = 2;
            }
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3))
            {
                corner = 3;
            }
            if (ImGui::MenuItem("Close"))
            {
                *isShow = false;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}
