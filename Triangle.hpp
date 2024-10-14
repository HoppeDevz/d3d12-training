#pragma once

#include <d3d12.h>
#include <d3dcompiler.h>

#include <DirectXMath.h>
#include <DirectXColors.h>

#include <string>
#include <sstream>

#include "Mesh.hpp"

using namespace DirectX;

struct Vertex {

	XMFLOAT3 Pos;
	XMFLOAT4 Color;

};

class Triangle {

	public:
		ID3D12RootSignature* pRootSignature = nullptr;
		ID3D12PipelineState* pPipelineState = nullptr;
		Mesh* pMesh = nullptr;

		Triangle();
		~Triangle();

		void init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);

		void buildGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList);
		void buildRootSignature(ID3D12Device* pDevice);
		void buildPipelineState(ID3D12Device* pDevice);

		void Draw(ID3D12GraphicsCommandList* pCommandList);

};