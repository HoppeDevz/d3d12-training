#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include "Triangle.hpp"

class Graphics {
	
public:
		IDXGIFactory7* pFactory = nullptr;
		IDXGISwapChain1* pSwapChain = nullptr;
		ID3D12Device10* pDevice = nullptr;

		ID3D12Resource** pBackBuffers = nullptr;
		ID3D12Resource* pDepthStencil = nullptr;
		ID3D12DescriptorHeap* pRtvHeap = nullptr;
		ID3D12DescriptorHeap* pDsHeap = nullptr;
		ID3D12DescriptorHeap* pSrvHeap = nullptr;

		ID3D12Fence1* pFence = nullptr;
		int fenceValue = 0;
		int currentBufferIndex = 0;

		ID3D12CommandQueue* pCommandQueue = nullptr;
		ID3D12CommandAllocator* pCommandAllocator = nullptr;
		ID3D12GraphicsCommandList* pCommandList = nullptr;

		HWND targetWindow;

		int vWidth = 0;
		int vHeight = 0;

		D3D12_VIEWPORT               viewport;
		D3D12_RECT                   scissorRect;

		int backBufferCount = 2;

	
		Graphics(int _vWidth, int _vHeight, HWND _targetWindow);

		void DisplayAdaptersAndOutputs();

		void Init();

		void ExecuteCommands();
		void WaitGPU();

		void Clear();
		void Swap();

};
