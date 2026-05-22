#pragma once
// Shader.h
// [Phase 4 �ű� ����]
// - ���� CGraphicsPipeline�� ������ D3D12 PSO ��� Shader Ŭ������ ��ü
// - CShader: PSO 1���� �����ϴ� ��� Ŭ����
// - CDiffusedShader: ��ġ+���� ������ ó���ϴ� ���̴� (VSDiffused/PSDiffused ���)
//

class CCamera;

// ���� ������Ʈ�� ���� ����� ���̴��� �ѱ�� ���� ��� ���� ����ü

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4 m_xmf4x4World; // ���� ���
	XMFLOAT4 m_xmf4Color;     // ���� (��ǻ�� ���̴���)
};


// CShader: PSO 1���� �����ϴ� ��� Ŭ����
class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int m_nReferences = 0;

protected:
	// PSO �迭 ����� 1��
	ID3D12PipelineState** m_ppd3dPipelineStates = NULL;
	int m_nPipelineStates = 0;

public:
	void AddRef() { ++m_nReferences; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	// ���������� ���� ������Ʈ ���� �Լ��� 
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC		CreateRasterizerState();
	virtual D3D12_BLEND_DESC			CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE		CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE       CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	// ���̴� ���� ������ ����
	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName,
		LPSTR pszShaderName, LPSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);

	// PSO ���� - Device + RootSignature  �ʿ�
	virtual void CreateShader(ID3D12Device* pd3dDevice,
		ID3D12RootSignature* pd3dRootSignature);

	// ���̴� ����(���) ���� - �� ������ �Ǵ� �� ������Ʈ���� ȣ��
	virtual void CretaeShaderVariables(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// ���� ������Ʈ�� ���� ����� b0 ���Կ� ���ε�
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList,
		XMFLOAT4X4* pxmf4x4World);

	// ������ �Լ� - �� ������Ʈ���� ȣ��
	// PSO ���ε�
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

};

// ������������������������������������������������������������������������������������������������������������������
// CDiffusedShader : ��ġ+���� ������ ���̴�
// - InputLayout: POSITION (R32G32B32) + COLOR (R32G32B32A32)
// - VS: VSDiffused, PS: PSDiffused (Shaders.hlsl)
// ������������������������������������������������������������������������������������������������������������������


class CDiffusedShader : public CShader
{
public:
	CDiffusedShader();
	virtual ~CDiffusedShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob) override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob) override;
	virtual void CreateShader(ID3D12Device* pd3dDevice,
		ID3D12RootSignature* pd3dRootSignature) override;



};

// ──────────────────────────────────────────────────────────
// CWireframeShader : 디버그용 OOBB 와이어프레임 PSO
// - CDiffusedShader 의 InputLayout/VS/PS 그대로 상속
// - RasterizerState: FillMode = WIREFRAME, CullMode = NONE
// - DepthStencilState: DepthWriteMask = ZERO (테스트만, 쓰지 않음)
// ──────────────────────────────────────────────────────────
class CWireframeShader : public CDiffusedShader
{
public:
	CWireframeShader();
	virtual ~CWireframeShader();

	virtual D3D12_RASTERIZER_DESC   CreateRasterizerState() override;
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
};

class CInstancedShader : public CShader
{
public:
	CInstancedShader();
	virtual ~CInstancedShader();
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_SHADER_BYTECODE   CreateVertexShader(ID3DBlob** ppd3dShaderBlob) override;
	virtual D3D12_SHADER_BYTECODE   CreatePixelShader(ID3DBlob** ppd3dShaderBlob) override;
	virtual void CreateShader(ID3D12Device* pd3dDevice,
		ID3D12RootSignature* pd3dGraphicsRootSignature) override;
};