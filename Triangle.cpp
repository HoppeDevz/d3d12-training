#include "Triangle.hpp"
#include "ApplicationError.hpp"

using namespace DirectX;

Triangle::Triangle() {

    this->pMesh = new Mesh();
    this->pPipelineState = nullptr;
    this->pRootSignature = nullptr;

}

Triangle::~Triangle() {

    if (this->pMesh)
        delete this->pMesh;

    if (this->pPipelineState)
        this->pPipelineState->Release();

    if (this->pRootSignature)
        this->pRootSignature->Release();
}

void Triangle::init(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {

    this->buildGeometry(pDevice, pCommandList);
    this->buildRootSignature(pDevice);
    this->buildPipelineState(pDevice);

}

void Triangle::buildGeometry(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList) {
	
	Vertex triangleVertices[3] = {
        { XMFLOAT3(0.0f, 0.25f, 0.0f), XMFLOAT4(Colors::Green) },
        { XMFLOAT3(0.25f, -0.25f, 0.0f), XMFLOAT4(Colors::Blue) },
        { XMFLOAT3(-0.25f, -0.25f, 0.0f), XMFLOAT4(Colors::Red) }
	};

	int vertexBufferStride = sizeof(Vertex);
	int vertexBufferSize = sizeof(Vertex) * _countof(triangleVertices);

    this->pMesh->vertexBufferStride = vertexBufferStride;
    this->pMesh->vertexBufferSize = vertexBufferSize;

    this->pMesh->AllocateCPUBuffer(vertexBufferSize);
    this->pMesh->AllocateUploadBuffer(pDevice, vertexBufferSize);
    this->pMesh->AllocateGPUBuffer(pDevice, vertexBufferSize);

    this->pMesh->CopyVerticesToCPUBuffer(triangleVertices, vertexBufferSize);
    this->pMesh->CopyFromCPUToGPUBuffer(pDevice, pCommandList);

}

void Triangle::buildRootSignature(ID3D12Device* pDevice) {
	
    // descrição para uma assinatura vazia
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // serializa assinatura raiz
    ID3DBlob* serializedRootSig = nullptr;
    ID3DBlob* error = nullptr;

    ThrowIfFailed(D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &error));

    // cria uma assinatura raiz vazia
    ThrowIfFailed(
        pDevice->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&this->pRootSignature)
        )
    );

}

void Triangle::buildPipelineState(ID3D12Device* pDevice) {

    // --------------------
    // --- Input Layout ---
    // --------------------

    D3D12_INPUT_ELEMENT_DESC inputLayout[2] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // --------------------
    // ----- Shaders ------
    // --------------------

    ID3DBlob* vertexShader;
    ID3DBlob* pixelShader;

    D3DReadFileToBlob(L"../x64/Debug/vertex-shader.cso", &vertexShader);
    D3DReadFileToBlob(L"../x64/Debug/pixel-shader.cso", &pixelShader);

    // --------------------
    // ---- Rasterizer ----
    // --------------------

    D3D12_RASTERIZER_DESC rasterizer = {};
    rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
    //rasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
    rasterizer.CullMode = D3D12_CULL_MODE_BACK;
    rasterizer.FrontCounterClockwise = FALSE;
    rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizer.DepthClipEnable = TRUE;
    rasterizer.MultisampleEnable = FALSE;
    rasterizer.AntialiasedLineEnable = FALSE;
    rasterizer.ForcedSampleCount = 0;
    rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // ---------------------
    // --- Color Blender ---
    // ---------------------

    D3D12_BLEND_DESC blender = {};
    blender.AlphaToCoverageEnable = FALSE;
    blender.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
    {
        FALSE,FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };
    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blender.RenderTarget[i] = defaultRenderTargetBlendDesc;

    // ---------------------
    // --- Depth Stencil ---
    // ---------------------

    D3D12_DEPTH_STENCIL_DESC depthStencil = {};
    depthStencil.DepthEnable = FALSE;
    depthStencil.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencil.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencil.StencilEnable = FALSE;
    depthStencil.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencil.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
    { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
    depthStencil.FrontFace = defaultStencilOp;
    depthStencil.BackFace = defaultStencilOp;

    // -----------------------------------
    // --- Pipeline State Object (PSO) ---
    // -----------------------------------

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = this->pRootSignature;
    pso.VS = { reinterpret_cast<BYTE*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
    pso.PS = { reinterpret_cast<BYTE*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    pso.BlendState = blender;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState = rasterizer;
    pso.DepthStencilState = depthStencil;
    pso.InputLayout = { inputLayout, 2 };
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_UNKNOWN;
    pso.SampleDesc.Count = 1;
    pso.SampleDesc.Quality = 0;

    ThrowIfFailed(
        pDevice->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&this->pPipelineState))
    );
    
    vertexShader->Release();
    pixelShader->Release();


}

void Triangle::Draw(ID3D12GraphicsCommandList* pCommandList) {

    pCommandList->SetPipelineState(this->pPipelineState);

    pCommandList->SetGraphicsRootSignature(this->pRootSignature);
    pCommandList->IASetVertexBuffers(0, 1, this->pMesh->getVertexBufferView());
    pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    pCommandList->DrawInstanced(3, 1, 0, 0);

}
