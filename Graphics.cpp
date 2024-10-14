#include <string>
#include <sstream>
#include <wrl.h>

#include "Graphics.hpp"
#include "ApplicationError.hpp"
#include "Triangle.hpp"

using namespace DirectX;

Triangle* g_triangle = new Triangle();

Graphics::Graphics(int _vWidth, int _vHeight, HWND _targetWindow) {

	this->vWidth = _vWidth;
	this->vHeight = _vHeight;
	this->targetWindow = _targetWindow;

}

void Graphics::DisplayAdaptersAndOutputs() {

	IDXGIFactory7* pFactory = nullptr;
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pFactory));

	ID3D12Debug* debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();

	IDXGIAdapter* pAdapter = nullptr;
	if (pFactory->EnumAdapters(0, &pAdapter) != D3D12_ERROR_ADAPTER_NOT_FOUND) {

		DXGI_ADAPTER_DESC adapterDesc;
		pAdapter->GetDesc(&adapterDesc);

		std::wstringstream gpuModel;
		gpuModel << L"GPU: " << adapterDesc.Description << std::endl;
		OutputDebugStringW(gpuModel.str().c_str());

		IDXGIOutput* pOutput = nullptr;
		for (int i = 0; i < 4; i++) {

			if (!FAILED(pAdapter->EnumOutputs(i, &pOutput))) {

				DXGI_OUTPUT_DESC outputDesc;
				pOutput->GetDesc(&outputDesc);

				std::wstringstream outputModel;
				outputModel << "OUTPUT_ID:" << i << " | Monitor: " << outputDesc.DeviceName << std::endl;
				OutputDebugStringW(outputModel.str().c_str());

			}
		}

	}

}

void Graphics::Init() {

	// ======================================================================================== //
	// =========== COMMANDQUEUE, COMMAND ALLOCATOR, COMMAND LIST, FENCE & SWAPCHAIN =========== // 
	// ======================================================================================== // 

	ThrowIfFailed(
		CreateDXGIFactory2(
			NULL,
			IID_PPV_ARGS(&pFactory)
		)
	);

	ThrowIfFailed(
		D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&this->pDevice)
		)
	);

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(
		pDevice->CreateCommandQueue(
			&commandQueueDesc,
			IID_PPV_ARGS(&this->pCommandQueue)
		)
	);

	ThrowIfFailed(
		pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&this->pCommandAllocator)
		)
	);

	ThrowIfFailed(
		pDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			this->pCommandAllocator,
			nullptr,
			IID_PPV_ARGS(&this->pCommandList)
		)
	);

	ThrowIfFailed(
		pDevice->CreateFence(
			this->fenceValue,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&this->pFence)
		)
	);

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = this->vWidth;
	swapChainDesc.Height = this->vHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = this->backBufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	ThrowIfFailed(
		pFactory->CreateSwapChainForHwnd(
			this->pCommandQueue,
			this->targetWindow,
			&swapChainDesc,
			nullptr,
			nullptr,
			&this->pSwapChain
		)
	);


	// ===================================================== //
	// =========== BACK BUFFERS E RENDER TARGETS =========== // 
	// ===================================================== // 

	this->pBackBuffers = new ID3D12Resource*[swapChainDesc.BufferCount];

	for (int i = 0; i < swapChainDesc.BufferCount; i++) {

		pSwapChain->GetBuffer(i, IID_PPV_ARGS(&this->pBackBuffers[i]));

	}

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;

	ThrowIfFailed(
		pDevice->CreateDescriptorHeap(
			&rtvHeapDesc,
			IID_PPV_ARGS(&this->pRtvHeap)
		)
	);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < swapChainDesc.BufferCount; i++) {
			
		pDevice->CreateRenderTargetView(this->pBackBuffers[i], nullptr, rtvHandle);
		rtvHandle.ptr += incrementSize;

	}












	// ===================================================== //
	// =================== DEPTH STENCIL =================== // 
	// ===================================================== // 

	/*D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = this->vWidth;
	depthStencilDesc.Height = this->vHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES dsHeapProperties = {};
	dsHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	dsHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	dsHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	dsHeapProperties.CreationNodeMask = 1;
	dsHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE optmizedClear = {};
	optmizedClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optmizedClear.DepthStencil.Depth = 1.0f;
	optmizedClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(
		pDevice->CreateCommittedResource(
			&dsHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optmizedClear,
			IID_PPV_ARGS(&this->pDepthStencil)
		)
	);

	D3D12_DESCRIPTOR_HEAP_DESC dpHeapDesc = {};
	dpHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dpHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dpHeapDesc.NumDescriptors = 1;

	ThrowIfFailed(
		pDevice->CreateDescriptorHeap(
			&dpHeapDesc,
			IID_PPV_ARGS(&this->pDsHeap)
		)
	);

	D3D12_CPU_DESCRIPTOR_HANDLE dpHandle = this->pDsHeap->GetCPUDescriptorHandleForHeapStart();

	pDevice->CreateDepthStencilView(
		this->pDepthStencil,
		nullptr,
		dpHandle
	);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = this->pDepthStencil;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	this->pCommandList->ResourceBarrier(1, &barrier);
	this->ExecuteCommands();*/

	g_triangle->init(this->pDevice, this->pCommandList);
	this->ExecuteCommands();
	
	ZeroMemory(&viewport, sizeof(viewport));
	ZeroMemory(&scissorRect, sizeof(scissorRect));

	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(this->vWidth);
	viewport.Height = static_cast<float>(this->vHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// ajusta o retângulo de corte
	scissorRect = { 0, 0, this->vWidth, this->vHeight };

}

void Graphics::ExecuteCommands() {

	this->pCommandList->Close();

	ID3D12CommandList* pCommandLists[] = {
		this->pCommandList
	};

	pCommandQueue->ExecuteCommandLists(
		_countof(pCommandLists), pCommandLists
	);

	this->WaitGPU();

	this->pCommandAllocator->Reset();
	this->pCommandList->Reset(this->pCommandAllocator, nullptr);

}
void Graphics::WaitGPU() {

	this->fenceValue++;

	ThrowIfFailed(
		this->pCommandQueue->Signal(this->pFence, this->fenceValue)
	);

	if (this->pFence->GetCompletedValue() < this->fenceValue) {
			
		HANDLE event = CreateEventExW(NULL, NULL, NULL, EVENT_ALL_ACCESS);

		if (!event) {
			ThrowAppError();
		}

		ThrowIfFailed(
			pFence->SetEventOnCompletion(this->fenceValue, event)
		);

		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);

	}

}



void Graphics::Clear() {

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = this->pBackBuffers[this->currentBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	this->pCommandList->ResourceBarrier(1, &barrier);

	this->pCommandList->RSSetViewports(1, &viewport);
	this->pCommandList->RSSetScissorRects(1, &scissorRect);

	D3D12_CPU_DESCRIPTOR_HANDLE cHandle = this->pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = this->pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	cHandle.ptr += incrementSize * this->currentBufferIndex;

	FLOAT color[] = { 0.9f, 0.0f, 0.5f, 1.0f };
	this->pCommandList->ClearRenderTargetView(cHandle, color, 0, nullptr);

	/*D3D12_CPU_DESCRIPTOR_HANDLE dsHandle = this->pDsHeap->GetCPUDescriptorHandleForHeapStart();
	this->pCommandList->ClearDepthStencilView(dsHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);*/

	this->pCommandList->OMSetRenderTargets(1, &cHandle, true, NULL);

	g_triangle->Draw(this->pCommandList);
	this->ExecuteCommands();

}

void Graphics::Swap() {

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = this->pBackBuffers[this->currentBufferIndex];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	this->pCommandList->ResourceBarrier(1, &barrier);
	this->ExecuteCommands();
	
	this->pSwapChain->Present(0, 0);
	currentBufferIndex = (currentBufferIndex + 1) % this->backBufferCount;

}
