#pragma once

#include <d3d12.h>
#include <imgui.h>

/**
 * @brief ImGui
 */
class ImGuiDx12
{
public:
	ImGuiDx12();
	~ImGuiDx12();

	/**
	 * @brief ������
	 */
	bool Init(
		HWND* hWnd,
		ID3D12Device* device, 
		int num_frames_in_flight, 
		DXGI_FORMAT rtv_format, 
		ID3D12DescriptorHeap* cbv_srv_heap,
		D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle, 
		D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle
	);

	/**
	 * @brief �V���b�g�_�E��
	 */
	void Shutdown();

	/**
	 * @brief �t���[���J�n
	 */
	void NewFrame();

	/**
	 * @brief �`��
	 */
	void Render(ID3D12GraphicsCommandList* graphics_command_list);

private:
	/// �t���[�����\�[�X
	struct FrameResources
	{
		ID3D12Resource* IndexBuffer;
		ID3D12Resource* VertexBuffer;
		int IndexBufferSize;
		int VertexBufferSize;
	};
	
	/// ���_�R���X�^���g�o�b�t�@
	struct VERTEX_CONSTANT_BUFFER
	{
		float   mvp[4][4];
	};

private:
	/// ������
	bool InitCommon();
	bool InitWin32(HWND* hWnd);
	bool InitDx12(
		ID3D12Device* device,
		int num_frames_in_flight,
		DXGI_FORMAT rtv_format,
		ID3D12DescriptorHeap* cbv_srv_heap,
		D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle,
		D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle
	);

	/// �V���b�g�_�E��
	void ShutdownCommon();
	void ShutdownWin32();
	void ShutdownDx12();

	/// �t���[���J�n
	void NewFrameCommon();
	void NewFrameWin32();
	void NewFrameDx12();

	bool CreateDeviceObjects();
	void InvalidateDeviceObjects();
	void CreateFontsTexture();
	void SetupRenderState(ImDrawData* draw_data, ID3D12GraphicsCommandList* ctx, FrameResources* fr);

	void UpdateMousePos();
	
	/// �`��f�[�^��`��
	void RenderDrawData(ImDrawData* draw_data, ID3D12GraphicsCommandList* graphics_command_list);

private:
	// win32
	HWND m_hWnd;
	INT64 m_time;
	INT64 m_ticksPerSecond;
	ImGuiMouseCursor m_lastMouseCursor;

	// dx12
	ID3D12Device* m_d3dDevice;
	ID3D10Blob* m_vertexShaderBlob;
	ID3D10Blob* m_pixelShaderBlob;
	ID3D12RootSignature* m_rootSignature;
	ID3D12PipelineState* m_pipelineState;
	DXGI_FORMAT m_rtvFormat;
	ID3D12Resource* m_fontTextureResource;
	ID3D12DescriptorHeap* m_descriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_fontSrvCpuDescHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_fontSrvGpuDescHandle;
	FrameResources* m_frameResources;
	UINT m_numFramesInFlight;
	UINT m_frameIndex;

	bool m_isInitialized;
};
