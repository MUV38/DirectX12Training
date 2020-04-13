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

// ������
void D3D12AppBase::Initialize(HWND hWnd)
{
    HRESULT hr;

    // DirectXTex�̂��߂̏�����
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

    // �f�o�b�O
    UINT dxgiFlags = 0;
#if (_DEBUG)
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();
        dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;

#if 0 // GBV��L��������ꍇ
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

    // �n�[�h�E�F�A�A�_�v�^�̌���
    {
        UINT adapterIndex = 0;
        ComPtr<IDXGIAdapter1> adapter;
        while (DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &adapter))
        {
            DXGI_ADAPTER_DESC1 desc1{};
            adapter->GetDesc1(&desc1);
            ++adapterIndex;

            // WARP�łȂ����`�F�b�N
            if (desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            // Device���쐬�ł����`�F�b�N
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

    // HWND����T�C�Y�擾
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

// �I��
void D3D12AppBase::Finalize()
{
    OnFinalize();

    m_imgui.Shutdown();
}

// �X�V
void D3D12AppBase::Update()
{
    m_imgui.NewFrame();

    OnUpdate();
}

// �`��
void D3D12AppBase::Render()
{
    // �`��J�n
    BeginRender();

    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = m_backBufferRtvDescriptorHandle[m_frameIndex].GetCPUHandle();
    const D3D12_CPU_DESCRIPTOR_HANDLE dsv = m_backBufferDsvDescriptorHandle.GetCPUHandle();

    // �����_�[�^�[�Q�b�g�N���A
    const float clearColor[] = { 0.3f, 0.3f, 0.3f, 0.0f };
    m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    // �f�v�X�o�b�t�@�̃N���A
    m_commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // �`��Ώۂ̐ݒ�
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
	// �f�X�N���v�^�[�}�l�[�W��������
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
    // �X���b�v�`�F�C���C���[�W�ւ̃����_�[�^�[�Q�b�g�r���[����
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
            0, // �����l
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(&m_frameFences[i])
        );
        if (FAILED(hr))
        {
            throw std::runtime_error("Failed CreateFence");
        }
    }
}

// GPU�҂�
void D3D12AppBase::WaitForGPU()
{
    // GPU���ݒ肷��l��ݒ�
    auto& fence = m_frameFences[m_frameIndex];
    const auto currentValue = ++m_frameFenceValues[m_frameIndex];
    m_commandQueue->Signal(fence.Get(), currentValue);

    // GPU�������������Ă��邩�`�F�b�N
    auto nextIndex = (m_frameIndex + 1) % FrameBufferCount;
    const auto finishExpected = m_frameFenceValues[nextIndex];
    const auto nextFenceValue = m_frameFences[nextIndex]->GetCompletedValue();
    if (nextFenceValue < finishExpected)
    {
        // GPU�̏�����҂�
        m_frameFences[nextIndex]->SetEventOnCompletion(finishExpected, m_fenceWaitEvent);
        WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
    }
}

// �A�_�v�^�[
D3D12AppBase::ComPtr<IDXGIAdapter1>& D3D12AppBase::GetAdapter()
{
    return m_adapter;
}

// �f�o�C�X
D3D12AppBase::ComPtr<ID3D12Device>& D3D12AppBase::GetDevice()
{
    return m_device;
}

// �R�}���h�L���[
D3D12AppBase::ComPtr<ID3D12CommandQueue>& D3D12AppBase::GetCommandQueue()
{
    return m_commandQueue;
}

// �X���b�v�`�F�C��
D3D12AppBase::ComPtr<IDXGISwapChain4>& D3D12AppBase::GetSwapChain()
{
    return m_swapChain;
}

// �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g�擾
D3D12AppBase::ComPtr<ID3D12Resource>& D3D12AppBase::GetBackBufferRenderTarget()
{
    return m_backBufferRenderTargets[m_frameIndex];
}
D3D12AppBase::ComPtr<ID3D12Resource>& D3D12AppBase::GetBackBufferRenderTarget(uint32_t index)
{
    ASSERT(index < m_backBufferRenderTargets.size());

    return m_backBufferRenderTargets[index];
}

// �R�}���h���X�g
D3D12AppBase::ComPtr<ID3D12GraphicsCommandList>& D3D12AppBase::GetCommandList()
{
    return m_commandList;
}

// �R�}���h�A���P�[�^
D3D12AppBase::ComPtr<ID3D12CommandAllocator>& D3D12AppBase::GetCommandAllocator()
{
    return m_commandAllocators[m_frameIndex];
}
D3D12AppBase::ComPtr<ID3D12CommandAllocator>& D3D12AppBase::GetCommandAllocator(uint32_t index)
{
    ASSERT(index < m_commandAllocators.size());

    return m_commandAllocators[index];
}

// �t���[���C���f�b�N�X
UINT D3D12AppBase::GetFrameIndex() const
{
    return m_frameIndex;
}

// �r���[�|�[�g
const CD3DX12_VIEWPORT& D3D12AppBase::GetViewport() const
{
    return m_viewport;
}

// �V�U�[��`
const CD3DX12_RECT& D3D12AppBase::GetScissorRect() const
{
    return m_scissorRect;
}

// �f�X�N���v�^�[�}�l�[�W���[
DescriptorManager& D3D12AppBase::GetDescriptorManager()
{
    return m_descriptorManager;
}

// �o�b�N�o�b�t�@��RTV�n���h��
const DescriptorHandle& D3D12AppBase::GetBackBufferRtvDescriptorHandle() const
{
    return m_backBufferRtvDescriptorHandle[m_frameIndex];
}
const DescriptorHandle& D3D12AppBase::GetBackBufferRtvDescriptorHandle(uint32_t index) const
{
    ASSERT(index < FrameBufferCount);

    return m_backBufferRtvDescriptorHandle[index];
}

// �o�b�N�o�b�t�@��DSV�n���h��
const DescriptorHandle& D3D12AppBase::GetBackBufferDsvDescriptorHandle() const
{
    return m_backBufferDsvDescriptorHandle;
}

// �`��J�n
void D3D12AppBase::BeginRender()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_commandAllocators[m_frameIndex]->Reset();
    m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

    // �X���b�v�`�F�C���\���\���烌���_�[�^�[�Q�b�g�`��\��
    auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
        m_backBufferRenderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &barrierToRT);
}

// �`��I��
void D3D12AppBase::EndRender()
{
    // �����_�[�^�[�Q�b�g�`��\����X���b�v�`�F�C���\���\��
    auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
        m_backBufferRenderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    m_commandList->ResourceBarrier(1, &barrierToPresent);

    m_commandList->Close();

    // �R�}���h���X�g���s
    ID3D12CommandList* lists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(static_cast<UINT>(_countof(lists)), lists);

    m_swapChain->Present(1, 0);

    WaitForGPU();
}
