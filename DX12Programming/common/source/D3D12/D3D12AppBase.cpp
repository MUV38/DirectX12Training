#include "pch.h"
#include <fstream>
#include <exception>
#include <stdexcept>
#include <experimental/filesystem>
#include "D3D12/D3D12AppBase.h"
#include "Util/D3D12Util.h"

D3D12AppBase::D3D12AppBase()
    : m_rtvDescriptorSize(0)
    , m_dsvDescriptorSize(0)
    , m_frameIndex(0)
{
	m_backBuffers.resize(FrameBufferCount);
    m_frameFenceValues.resize(FrameBufferCount);

    m_fenceWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

D3D12AppBase::~D3D12AppBase()
{
}

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
    ComPtr<IDXGIAdapter1> useAdapter;
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
        adapter.As(&useAdapter);
    }

    // Device
    hr = D3D12CreateDevice(useAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device));
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

    Prepare();
}

void D3D12AppBase::Terminate()
{
    Cleanup();
}

void D3D12AppBase::Render()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_commandAllocators[m_frameIndex]->Reset();
    m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

    // スワップチェイン表示可能からレンダーターゲット描画可能へ
    auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		m_backBuffers[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &barrierToRT);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        m_heapRtv->GetCPUDescriptorHandleForHeapStart(),
        m_frameIndex, m_rtvDescriptorSize
    );
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsv(
        m_heapDsv->GetCPUDescriptorHandleForHeapStart()
    );

    // レンダーターゲットクリア
    const float clearColor[] = { 0.3f, 0.3f, 0.3f, 0.0f };
    m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    // デプスバッファのクリア
    m_commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 描画対象の設定
    m_commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

    MakeCommand(m_commandList);

    // レンダーターゲット描画可能からスワップチェイン表示可能へ
    auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_backBuffers[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    m_commandList->ResourceBarrier(1, &barrierToPresent);

    m_commandList->Close();

    // コマンドリスト実行
    ID3D12CommandList* lists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(static_cast<UINT>(_countof(lists)), lists);

    m_swapChain->Present(1, 0);

    WaitPreviousFrame();
}

void D3D12AppBase::PrepareDescriptorHeaps()
{
    // RTVのディスクリプタヒープ
    HRESULT hr;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc
    {
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        FrameBufferCount,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        0
    };
    hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_heapRtv));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed CreateDescriptorHeap(RTV)");
    }
    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // DSVのディスクリプタヒープ
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc
    {
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        1,
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        0
    };
    hr = m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_heapDsv));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed CreateDescriptorHeap(DSv)");
    }
}

void D3D12AppBase::PrepareRenderTargetView()
{
    // スワップチェインイメージへのレンダーターゲットビュー生成
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_heapRtv->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < FrameBufferCount; ++i)
    {
        m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
        m_device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);

        // 参照するディスクリプタの変更
        rtvHandle.Offset(1, m_rtvDescriptorSize);
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
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_heapDsv->GetCPUDescriptorHandleForHeapStart());
    m_device->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, dsvHandle);
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

void D3D12AppBase::WaitPreviousFrame()
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

HRESULT D3D12AppBase::CompileShaderFromFile(const std::wstring& fileName, const std::wstring& profile, ComPtr<ID3DBlob>& shaderBlob, ComPtr<ID3DBlob>& errorBlob)
{
    using namespace std::experimental::filesystem;

    path filePath(fileName);
    std::ifstream infile(filePath);
    std::vector <char> srcData;
    if (!infile)
    {
        throw std::runtime_error("Shader not found.");
    }
    srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
    infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

    // DXCによるコンパイル
    ComPtr<IDxcLibrary> library;
    ComPtr<IDxcCompiler> compiler;
    ComPtr<IDxcBlobEncoding> source;
    ComPtr<IDxcOperationResult> dxcResult;

    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    library->CreateBlobWithEncodingFromPinned(srcData.data(), UINT(srcData.size()), CP_ACP, &source);
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    LPCWSTR compilerFlags[] = {
#if _DEBUG
        L"/Zi", L"/O0",
#else 
        L"/O2" // リリースでは最適化
#endif
    };
    compiler->Compile(
        source.Get(), 
        filePath.wstring().c_str(),
        L"main", 
        profile.c_str(),
        compilerFlags, 
        _countof(compilerFlags),
        nullptr, 
        0, 
        nullptr,
        &dxcResult
    );

    HRESULT hr;
    dxcResult->GetStatus(&hr);
    if (SUCCEEDED(hr))
    {
        dxcResult->GetResult(
            reinterpret_cast<IDxcBlob * *>(shaderBlob.GetAddressOf())
        );
    }
    else
    {
        dxcResult->GetErrorBuffer(
            reinterpret_cast<IDxcBlobEncoding * *>(errorBlob.GetAddressOf())
        );
    }

    return hr;
}

