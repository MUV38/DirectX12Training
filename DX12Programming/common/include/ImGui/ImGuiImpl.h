#pragma once

#include <d3d12.h>
#include <imgui.h>

/**
 * @brief ImGui
 */
class ImGuiImpl
{
public:
	ImGuiImpl();
	~ImGuiImpl();

	/**
	 * @brief 初期化
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
	 * @brief シャットダウン
	 */
	void Shutdown();

	/**
	 * @brief フレーム開始
	 */
	void NewFrame();

	/**
	 * @brief 描画
	 */
	void RenderDrawData(ImDrawData* draw_data, ID3D12GraphicsCommandList* graphics_command_list);

private:
	/// フレームリソース
	struct FrameResources
	{
		ID3D12Resource* IndexBuffer;
		ID3D12Resource* VertexBuffer;
		int IndexBufferSize;
		int VertexBufferSize;
	};
	
	/// 頂点コンスタントバッファ
	struct VERTEX_CONSTANT_BUFFER
	{
		float   mvp[4][4];
	};

private:
	/// 初期化
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

	/// シャットダウン
	void ShutdownCommon();
	void ShutdownWin32();
	void ShutdownDx12();

	/// フレーム開始
	void NewFrameCommon();
	void NewFrameWin32();
	void NewFrameDx12();

	bool CreateDeviceObjects();
	void InvalidateDeviceObjects();
	void CreateFontsTexture();
	void SetupRenderState(ImDrawData* draw_data, ID3D12GraphicsCommandList* ctx, FrameResources* fr);

	bool UpdateMouseCursor();
	void UpdateMousePos();
	void UpdateGamepads();

private:
	// win32
	HWND m_hWnd;
	INT64 m_time;
	INT64 m_ticksPerSecond;
	ImGuiMouseCursor m_lastMouseCursor;
	bool m_hasGamepad;
	bool m_wantUpdateHasGamepad;

	// dx12
	ID3D12Device* m_d3dDevice;
	ID3D10Blob* m_vertexShaderBlob;
	ID3D10Blob* m_pixelShaderBlob;
	ID3D12RootSignature* m_rootSignature;
	ID3D12PipelineState* m_pipelineState;
	DXGI_FORMAT m_rtvFormat;
	ID3D12Resource* m_fontTextureResource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_fontSrvCpuDescHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_fontSrvGpuDescHandle;
	FrameResources* m_frameResources;
	UINT m_numFramesInFlight;
	UINT m_frameIndex;
};
