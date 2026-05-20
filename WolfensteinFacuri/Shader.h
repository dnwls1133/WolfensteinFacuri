#pragma once
// Shader.h
// [Phase 4 신규 파일]
// - 기존 CGraphicsPipeline의 역할을 D3D12 PSO 기반 Shader 클래스로 대체
// - CShader: PSO 1개를 관리하는 기반 클래스
// - CDiffusedShader: 위치+색상 정점을 처리하는 셰이더 (VSDiffused/PSDiffused 사용)
//

class CCamera;

// 게임 오브젝트의 월드 행렬을 셰이더에 넘기기 위한 상수 버퍼 구조체

struct CB_GAMEOBJECT_INFO
{
	XMFLOAT4X4 m_xmf4x4World; // 월드 행렬
	XMFLOAT4 m_xmf4Color;     // 색상 (디퓨즈 셰이더용)
};


// CShader: PSO 1개를 관리하는 기반 클래스
class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int m_nReferences = 0;

protected:
	// PSO 배열 현재는 1개
	ID3D12PipelineState** m_ppd3dPipelineStates = NULL;
	int m_nPipelineStates = 0;

public:
	void AddRef() { ++m_nReferences; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	// 파이프라인 상태 컴포넌트 생성 함수들 
	virtual D3D12_INPUT_LAYOUT_DESC		CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC		CreateRasterizerState();
	virtual D3D12_BLEND_DESC			CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC	CreateDepthStencilState();
	virtual D3D12_SHADER_BYTECODE		CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE       CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	// 셰이더 파일 컴파일 헬퍼
	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR* pszFileName,
		LPSTR pszShaderName, LPSTR pszShaderProfile, ID3DBlob** ppd3dShaderBlob);

	// PSO 생성 - Device + RootSignature  필요
	virtual void CreateShader(ID3D12Device* pd3dDevice,
		ID3D12RootSignature* pd3dRootSignature);

	// 셰이더 변수(상수) 갱신 - 매 프레임 또는 매 오브젝트마다 호출
	virtual void CretaeShaderVariables(ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// 게임 오브젝트의 월드 행렬을 b0 슬롯에 업로드
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList,
		XMFLOAT4X4* pxmf4x4World);

	// 렌더링 함수 - 매 오브젝트마다 호출
	// PSO 바인딩
	virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

};

// ─────────────────────────────────────────────────────────
// CDiffusedShader : 위치+색상 정점용 셰이더
// - InputLayout: POSITION (R32G32B32) + COLOR (R32G32B32A32)
// - VS: VSDiffused, PS: PSDiffused (Shaders.hlsl)
// ─────────────────────────────────────────────────────────


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