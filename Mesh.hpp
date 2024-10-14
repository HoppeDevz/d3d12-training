#pragma once

#include <d3d12.h>
#include <string>
#include <sstream>

class Mesh {
	
	public:
		ID3DBlob* pCPUBuffer;
		ID3D12Resource* pGPUIntermediateBuffer;
		ID3D12Resource* pGPUBuffer;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

		int vertexBufferStride;
		int vertexBufferSize;

		Mesh();
		~Mesh();

		void AllocateCPUBuffer(UINT sizeInBytes);
		void AllocateGPUBuffer(ID3D12Device* pDevice, UINT sizeInBytes);
		void AllocateUploadBuffer(ID3D12Device* pDevice, UINT sizeInBytes);

		void CopyVerticesToCPUBuffer(void* vertices, UINT sizeInBytes);
		void CopyFromCPUToGPUBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);

		D3D12_VERTEX_BUFFER_VIEW* getVertexBufferView();



};
