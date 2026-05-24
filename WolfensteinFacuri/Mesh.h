#pragma once
// Mesh.h
// [Phase 3 변환]
// - 정점 형식: XMFLOAT3 → CDiffusedVertex(position + color)
// - 정점/인덱스: shared_ptr<배열> → ID3D12Resource* (GPU 버퍼)
// - Render 시그니처: (HDC, CCamera*) → (ID3D12GraphicsCommandList*)
// - 6종 메시(CCubeMesh, CPlaneMesh, CPlayerMesh, CEnemyMesh, CWallMesh, CMissileMesh)의
//   정점/인덱스 데이터 자체는 기존 GDI 버전과 동일하게 보존


// [D3D12 추가] 정점 클래스 계층
class CVertex
{
protected:
	XMFLOAT3 m_xmf3Position;

public:
	CVertex() : m_xmf3Position(0.0f, 0.0f, 0.0f) {}
	CVertex(XMFLOAT3 xmf3Position) : m_xmf3Position(xmf3Position) {}
	~CVertex() {};

};

class CDiffusedVertex : public CVertex
{
protected:
	XMFLOAT4 m_xmf4Diffuse; // RGBA 색상
	XMFLOAT3 m_xmf3Normal;

public:
	CDiffusedVertex()
		: m_xmf4Diffuse(0.0f, 0.0f, 0.0f, 0.0f), m_xmf3Normal(0,1,0) {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 color,
		XMFLOAT3 normal = XMFLOAT3(0, 1, 0))
		: m_xmf4Diffuse(color), m_xmf3Normal(normal) {
		m_xmf3Position = XMFLOAT3(x, y, z);
	}
	CDiffusedVertex(XMFLOAT3 pos, XMFLOAT4 color,
		XMFLOAT3 normal = XMFLOAT3(0, 1, 0))
		: m_xmf4Diffuse(color), m_xmf3Normal(normal) {
		m_xmf3Position = pos;
	}
	~CDiffusedVertex() {}
};



// CMesh : GPU 정점/ 인덱스 버퍼를 관리하는 기반 클래스

class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();


private:
	// 참조 카운트 
	int m_nReferences = 0;

protected:
	// [D3D12 추가] 정점 버퍼 (GPU 리소스 + Upload 버퍼 + 버퍼 뷰)
	ID3D12Resource* m_pd3dVertexBuffer = nullptr;
	ID3D12Resource* m_pd3dVertexUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;

	// [D3D12 추가] 인덱스 버퍼 (GPU 리소스 + Upload 버퍼 + 버퍼 뷰)
	ID3D12Resource* m_pd3dIndexBuffer = nullptr;
	ID3D12Resource* m_pd3dIndexUploadBuffer = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;

	// [D3D12 추가] 메시 정보
	UINT m_nVertices = 0;
	UINT m_nStride = 0;		// sizeof(CDiffusedVertex)
	UINT m_nOffset = 0;
	UINT m_nSlot = 0;

	UINT m_nIndices = 0;
	UINT m_nStartIndex = 0;
	int m_nBaseVertex = 0;

	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// ─────────────────────────────────────────────────────
   // [보존] OBB Extents — 기존과 동일한 GetBoundingBoxExtents() 호출 유지
   // 생성자에서 정점 데이터로부터 미리 계산하여 멤버로 저장한다.
   // ────────────────────────────────────────────────────

	XMFLOAT3 m_xmf3Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3LocalCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);

	// 정점 데이터로부터 OBB Extents 계산 (생성자에서 호출)
	void CalculateBoundingBoxExtents(const CDiffusedVertex* pVertices, UINT nVertices);

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	// [D3D12 추가] Upload Heap의 임시 버퍼를 GPU 복사 완료 후 해제
	void ReleaseUploadBuffers();

	XMFLOAT3 GetBoundingBoxExtents() const;
	XMFLOAT3 GetLocalCenter() const { return m_xmf3LocalCenter; }
	// Render 시그니처: HDC -> ID3D12GraphicsCommandList*
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);


	// 인스턴싱 렌더링
	virtual void RenderInstanced(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstanceCount);

};


class CCubeMesh : public CMesh
{
public:
	CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
		float fWHD = 2.0f);
	virtual ~CCubeMesh();
};

class CPlaneMesh : public CMesh
{
public:
	CPlaneMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
		float fWidth, float fHeight);
	virtual ~CPlaneMesh();
};

class CPlayerMesh : public CMesh
{
public:
	CPlayerMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
		, float fScale);
	virtual ~CPlayerMesh();
};

class CEnemyMesh : public CMesh
{
	public:
	CEnemyMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
		, float fScale);
	virtual ~CEnemyMesh();
};

class CWallMesh : public CMesh
{
	public:
	CWallMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
		, float fWidth, float fHeight, float fDepth);
	virtual ~CWallMesh();
};

class CMissileMesh : public CMesh
{
	public:
	CMissileMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
		, float fScale);
	virtual ~CMissileMesh();
};

class C3DTextMesh : public CMesh
{
public:
	C3DTextMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		std::string text, float fScale, XMFLOAT4 color);
	virtual ~C3DTextMesh();
};

class CStairMesh : public CMesh
{
	public:
	CStairMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float fWidth, float fHeight, float fDepth);
	virtual ~CStairMesh();
};

class CJointPartMesh : public CMesh
{
public:
	CJointPartMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float fWidth, float fHeight, float fDepth, XMFLOAT4 color);
	virtual ~CJointPartMesh();
};

class CMutantMesh : public CMesh
{
	public:
	CMutantMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float fScale);
	virtual ~CMutantMesh();
};

class CGunMesh : public CMesh
{
	public:
	CGunMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float fScale);
	virtual ~CGunMesh();
};

class CCrosshairMesh : public CMesh
{
public:
	CCrosshairMesh(ID3D12Device* , ID3D12GraphicsCommandList*,
		float fHalfLen = 0.003f,
		float fThick = 0.004f,
		XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1));
};