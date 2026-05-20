#pragma once

// InstanceBuffer.h
// [Phase 9.1 신규 파일]
// 인스턴스 데이터를 GPU에 업로드/관리하는 헬퍼 클래스
//
// 사용 패턴:
//   1) Create(device, maxInstances) — Upload Heap 버퍼 생성 + 영구 매핑
//   2) 매 프레임 시작 시 Begin() — 카운터 리셋
//   3) AddInstance(world, color)  — 인스턴스 데이터 추가
//   4) Bind(commandList)          — 정점 버퍼 슬롯 1에 바인딩
//   5) (메시 RenderInstanced 호출) — DrawIndexedInstanced
//

// ─────────────────────────────────────────────────────────
// 인스턴스 단위 데이터 (Shaders.hlsl의 VS_INPUT_INSTANCED와 정확히 일치)
//   - World 행렬은 row_major로 저장 (CPU Transpose 없이 그대로)
//   - Color는 정점 색상과 곱해짐
// ─────────────────────────────────────────────────────────

struct InstanceData
{
	XMFLOAT4X4 world;
	XMFLOAT4  color;
};

class CInstancebuffer
{
public:
	CInstancebuffer();
	~CInstancebuffer();
	

	// 생성 
	void Create(ID3D12Device* pd3dDevice, UINT nMaxInstances);

	// 프레임 시작 시 카운터 리셋
	void Begin() { m_nCurrentCount = 0; }

	// 인스턴스 데이터 추가
	void AddInstance(const XMFLOAT4X4& world, const XMFLOAT4& color);

	// 정점 버퍼 슬롯 1에 바인딩
	void Bind(ID3D12GraphicsCommandList* pd3dCommandList, UINT nSlot = 1);

	UINT GetCount() const { return m_nCurrentCount; }

	void Release();

private:
	ID3D12Resource* m_pd3dInstanceBuffer = NULL;

	InstanceData* m_pMappedData = NULL;

	UINT m_nMaxInstances = 0;
	UINT m_nCurrentCount = 0;

	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;

};

