#include "Mesh.hpp"
#include "ApplicationError.hpp"

#include <d3dcompiler.h>

Mesh::Mesh() {

}

Mesh::~Mesh() {

	if (this->pCPUBuffer)
		this->pCPUBuffer->Release();

	if (this->pGPUIntermediateBuffer)
		this->pGPUIntermediateBuffer->Release();

	if (this->pGPUBuffer)
		this->pGPUBuffer->Release();

}

void Mesh::AllocateCPUBuffer(UINT sizeInBytes) {
	D3DCreateBlob(sizeInBytes, &this->pCPUBuffer);
}

void Mesh::AllocateGPUBuffer(ID3D12Device* pDevice, UINT sizeInBytes) {

	// propriedades da heap do buffer
	D3D12_HEAP_PROPERTIES bufferProp = {};
	bufferProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	bufferProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	bufferProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	bufferProp.CreationNodeMask = 1;
	bufferProp.VisibleNodeMask = 1;

	// descrição do buffer 
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = sizeInBytes;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;

	ThrowIfFailed(
		pDevice->CreateCommittedResource(
			&bufferProp,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			initState,
			nullptr,
			IID_PPV_ARGS(&this->pGPUBuffer)
		)
	);

}

void Mesh::AllocateUploadBuffer(ID3D12Device* pDevice, UINT sizeInBytes) {

	// propriedades da heap do buffer
	D3D12_HEAP_PROPERTIES bufferProp = {};
	bufferProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	bufferProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	bufferProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	bufferProp.CreationNodeMask = 1;
	bufferProp.VisibleNodeMask = 1;

	// descrição do buffer 
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 0;
	bufferDesc.Width = sizeInBytes;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_GENERIC_READ;

	ThrowIfFailed(
		pDevice->CreateCommittedResource(
			&bufferProp,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			initState,
			nullptr,
			IID_PPV_ARGS(&this->pGPUIntermediateBuffer)
		)
	);

}

void Mesh::CopyVerticesToCPUBuffer(void* vertices, UINT sizeInBytes) {

	memcpy(
		this->pCPUBuffer->GetBufferPointer(),
		vertices,
		sizeInBytes
	);

}

void Mesh::CopyFromCPUToGPUBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {
		
	D3D12_SUBRESOURCE_DATA uploadData;
	uploadData.pData = (void*)this->pCPUBuffer->GetBufferPointer();
	uploadData.RowPitch = this->vertexBufferSize;
	uploadData.SlicePitch = this->vertexBufferSize;

	std::wstringstream err;
	err << "RowPitch: " << uploadData.RowPitch << std::endl;
	err << "SlicePitch: " << uploadData.SlicePitch << std::endl;
	OutputDebugStringW(err.str().c_str());

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	UINT nrows;
	UINT64 rowSizeInBytes;
	UINT64 totalBytes;

	D3D12_RESOURCE_DESC bufferDesc = this->pGPUIntermediateBuffer->GetDesc();
	pDevice->GetCopyableFootprints(
		&bufferDesc,
		0, 1, 0, 
		&footprint, &nrows, &rowSizeInBytes, &totalBytes
	);

	std::wstringstream x;
	x << "footprint.Offset: " << footprint.Offset << std::endl;
	OutputDebugStringW(x.str().c_str());

	BYTE* pData;
	this->pGPUIntermediateBuffer->Map(0, nullptr, (void**)&pData);

	memcpy(
		pData + footprint.Offset, 
		uploadData.pData, 
		uploadData.SlicePitch
	);

	this->pGPUIntermediateBuffer->Unmap(0, nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = this->pGPUBuffer;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &barrier);

	pCommandList->CopyBufferRegion(
		this->pGPUBuffer,
		0,
		this->pGPUIntermediateBuffer,
		footprint.Offset,
		footprint.Footprint.Width
	);

	D3D12_RESOURCE_BARRIER rBarrier = {};
	rBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rBarrier.Transition.pResource = this->pGPUBuffer;
	rBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	rBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	rBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	pCommandList->ResourceBarrier(1, &rBarrier);

}

D3D12_VERTEX_BUFFER_VIEW* Mesh::getVertexBufferView() {

	this->vertexBufferView.BufferLocation = this->pGPUBuffer->GetGPUVirtualAddress();
	this->vertexBufferView.StrideInBytes = this->vertexBufferStride;
	this->vertexBufferView.SizeInBytes = this->vertexBufferSize;

	return &this->vertexBufferView;

}
