#pragma once

#include "../framework.h"
#include "Descriptor/DescriptorManager.h"
#include "ImGui/ImGuiDx12.h"

class Application
{
public:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static const UINT FrameBufferCount = 2;    ///< �o�b�t�@�����O��
    static const UINT GpuWaitTimeout = (10 * 1000);    /// 10s

public:
    Application();
    virtual ~Application();

    //! @brief ���s
    int Run(HWND hWnd);

protected:
    /// override in subclass
    virtual void OnInitialize() {}
    virtual void OnFinalize() {}
    virtual void OnUpdate() {}
    virtual void OnRender(ComPtr<ID3D12GraphicsCommandList>& command) {}

protected:
    //! @brief ������
    void Initialize(HWND hWnd);
    //! @brief �I��
    void Finalize();
    //! @brief �X�V
    void Update();
    //! @brief �`��
    void Render();

    //! @brief GPU�҂�
    void WaitForGPU();

    //! @brief �A�_�v�^�[
    ComPtr<IDXGIAdapter1>& GetAdapter();
    //! @brief �f�o�C�X
    ComPtr<ID3D12Device>& GetDevice();
    //! @brief �R�}���h�L���[
    ComPtr<ID3D12CommandQueue>& GetCommandQueue();
    //! @brief �X���b�v�`�F�C��
    ComPtr<IDXGISwapChain4>& GetSwapChain();
    //! @brief �o�b�N�o�b�t�@�̃����_�[�^�[�Q�b�g
    ComPtr<ID3D12Resource>& GetBackBufferRenderTarget();
    ComPtr<ID3D12Resource>& GetBackBufferRenderTarget(uint32_t index);
    //! @brief �R�}���h���X�g
    ComPtr<ID3D12GraphicsCommandList>& GetCommandList();
    //! @brief �R�}���h�A���P�[�^
    ComPtr<ID3D12CommandAllocator>& GetCommandAllocator();
    ComPtr<ID3D12CommandAllocator>& GetCommandAllocator(uint32_t index);
    //! @brief �t���[���C���f�b�N�X
    UINT GetFrameIndex() const;
    //! @brief �r���[�|�[�g
    const CD3DX12_VIEWPORT& GetViewport() const;
    //! @brief �V�U�[��`
    const CD3DX12_RECT& GetScissorRect() const;
    //! @brief �f�X�N���v�^�[�}�l�[�W���[
    DescriptorManager& GetDescriptorManager();
    //! @brief �o�b�N�o�b�t�@��RTV�n���h��
    const DescriptorHandle& GetBackBufferRtvDescriptorHandle() const;
    const DescriptorHandle& GetBackBufferRtvDescriptorHandle(uint32_t index) const;
    //! @brief �o�b�N�o�b�t�@��DSV�n���h��
    const DescriptorHandle& GetBackBufferDsvDescriptorHandle() const;

private:
    void PrepareDescriptorHeaps();
    void PrepareRenderTargetView();
    void CreateDepthBuffer(int width, int height);
    void CreateCommandAllocators();
    void CreateFrameFences();

    //! @brief �`��J�n
    void BeginRender();
    //! @brief �`��I��
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