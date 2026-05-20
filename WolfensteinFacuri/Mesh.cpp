// [Phase 3 변환]
// - 6종 메시 모두 D3D12 정점/인덱스 버퍼로 업로드
// - 정점/인덱스 데이터는 기존 GDI 버전과 동일 (좌표·순서 보존)
//
#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsHelpers.h" //CreateBufferResource 사용




// CMesh 

CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

CMesh::~CMesh()
{
	// [D3D12 추가] GPU 리소스 해제
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();

}

void CMesh::ReleaseUploadBuffers()
{
	// CommandList 실행 완료 후 임시 업로드 버퍼 해제 (CGameFramework::BuildObjects에서 호출)
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;

	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;

}


// OBB Extents 계산 — 기존 GDI 버전 GetBoundingBoxExtents()와 동일한 알고리즘
// 정점 배열을 순회하며 (max - min) / 2 를 추출한다.
void CMesh::CalculateBoundingBoxExtents(const CDiffusedVertex* pVertices, UINT nVertices)
{
	if (!pVertices || nVertices == 0) {
		m_xmf3Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
		return;
	}

	// CDiffusedVertex의 첫 멤버가 XMFLOAT3 위치 - 동일 메모리 레이아웃 활용
	const XMFLOAT3* p0 = reinterpret_cast<const XMFLOAT3*>(&pVertices[0]);
	float maxX = p0->x; float maxY = p0->y; float maxZ = p0->z;
	float minX = p0->x; float minY = p0->y; float minZ = p0->z;

	for (UINT i = 1; i < nVertices; ++i)
	{
		const XMFLOAT3* p = reinterpret_cast<const XMFLOAT3*>(&pVertices[i]);
		if (p->x > maxX) maxX = p->x; if (p->y > maxY) maxY = p->y; if (p->z > maxZ) maxZ = p->z;
		if (p->x < minX) minX = p->x; if (p->y < minY) minY = p->y; if (p->z < minZ) minZ = p->z;
	}

	m_xmf3Extents = XMFLOAT3(
		(maxX- minX) * 0.5f,
		(maxY - minY) * 0.5f,
		(maxZ - minZ) * 0.5f
	);

}
// CMesh::Render — IA 단계 바인딩 후 인덱스 기반 드로우 콜
// (기존 GDI: 삼각형마다 Polygon() 호출 → GPU의 DrawIndexedInstanced 한 번으로 대체)

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);

	if (m_pd3dIndexBuffer) {
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, m_nStartIndex, m_nBaseVertex, 0);
	}
	else 
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}

void CMesh::RenderInstanced(ID3D12GraphicsCommandList* pd3dCommandList, UINT nInstanceCount)
{
    if (nInstanceCount == 0) return;

	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView);

    if (m_pd3dIndexBuffer) {
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
        pd3dCommandList->DrawIndexedInstanced(
			m_nIndices, nInstanceCount, m_nStartIndex, m_nBaseVertex, 0);
    }
    else
		pd3dCommandList->DrawInstanced(m_nVertices, nInstanceCount, 0, 0);
}


XMFLOAT3 CMesh::GetBoundingBoxExtents() const
{
	
	return m_xmf3Extents;
}

CCubeMesh::CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
	, float fWHD)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	float fx = fWHD * 0.5f;
	float fy = fWHD * 0.5f;
	float fz = fWHD * 0.5f;

	// 1. 점 8개 생성
	m_nVertices = 8;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	
	std::array<CDiffusedVertex, 8> pVertices;
	pVertices[0] = CDiffusedVertex(XMFLOAT3(-fx, +fy, -fz),RANDOM_COLOR); // 앞면 좌상단 (0)
	pVertices[1] = CDiffusedVertex(XMFLOAT3(+fx, +fy, -fz),RANDOM_COLOR); // 앞면 우상단 (1)
	pVertices[2] = CDiffusedVertex(XMFLOAT3(+fx, -fy, -fz),RANDOM_COLOR); // 앞면 우하단 (2)
	pVertices[3] = CDiffusedVertex(XMFLOAT3(-fx, -fy, -fz),RANDOM_COLOR); // 앞면 좌하단 (3)
	pVertices[4] = CDiffusedVertex(XMFLOAT3(-fx, +fy, +fz),RANDOM_COLOR); // 뒷면 좌상단 (4)
	pVertices[5] = CDiffusedVertex(XMFLOAT3(+fx, +fy, +fz),RANDOM_COLOR); // 뒷면 우상단 (5)
	pVertices[6] = CDiffusedVertex(XMFLOAT3(+fx, -fy, +fz),RANDOM_COLOR); // 뒷면 우하단 (6)
	pVertices[7] = CDiffusedVertex(XMFLOAT3(-fx, -fy, +fz),RANDOM_COLOR); // 뒷면 좌하단 (7)

	CalculateBoundingBoxExtents(pVertices.data(), m_nVertices);
	
	// 정점 버퍼 GPU 업로드
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
		pVertices.data(), m_nVertices * m_nStride,
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;


	// [핵심] 선(2개)이 아니라 삼각형(3개) 단위로 인덱스를 묶습니다.
	// 시계 방향(Clockwise)으로 점을 묶어야 앞면(Front Face)으로 인식됩니다!
	m_nIndices = 36;
	int indices[36] = {
		0, 1, 2,  0, 2, 3, // 앞면
		4, 6, 5,  4, 7, 6, // 뒷면
		4, 5, 1,  4, 1, 0, // 윗면
		3, 2, 6,  3, 6, 7, // 아랫면
		1, 5, 6,  1, 6, 2, // 오른쪽 면
		4, 0, 3,  4, 3, 7 // 왼쪽 면
	};

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        indices, m_nIndices * sizeof(UINT),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);

	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = m_nIndices * sizeof(UINT);
}

CCubeMesh::~CCubeMesh()
{
}
CPlaneMesh::CPlaneMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                      float fWidth, float fDepth)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    float fx = fWidth * 0.5f;
    float fz = fDepth * 0.5f;
 
    m_nVertices = 4;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
 
    // 바닥 타일은 동일한 회갈색 — 기존 SetColor(RGB(100,100,20))에 대응
    XMFLOAT4 xmf4FloorColor(100.0f / 255.0f, 100.0f / 255.0f, 20.0f / 255.0f, 1.0f);
 
    CDiffusedVertex pVertices[4];
    pVertices[0] = CDiffusedVertex(XMFLOAT3(-fx, 0.0f, -fz), xmf4FloorColor);
    pVertices[1] = CDiffusedVertex(XMFLOAT3(+fx, 0.0f, -fz), xmf4FloorColor);
    pVertices[2] = CDiffusedVertex(XMFLOAT3(+fx, 0.0f, +fz), xmf4FloorColor);
    pVertices[3] = CDiffusedVertex(XMFLOAT3(-fx, 0.0f, +fz), xmf4FloorColor);
 
    CalculateBoundingBoxExtents(pVertices, m_nVertices);
 
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pVertices, m_nStride * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = m_nStride;
    m_d3dVertexBufferView.SizeInBytes    = m_nStride * m_nVertices;
 
    m_nIndices = 6;
    UINT pnIndices[6] = { 0, 2, 1,  0, 3, 2 }; // [기존 GDI와 동일]
 
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pnIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}
 
CPlaneMesh::~CPlaneMesh() {}
 
 
// ═════════════════════════════════════════════════════════
// CPlayerMesh — 전투기 형태 (15 정점, 28 삼각형 / 84 인덱스)
// [보존] 기존 GDI 버전의 정점·인덱스 그대로
// ═════════════════════════════════════════════════════════
CPlayerMesh::CPlayerMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                       float fScale)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    float s = fScale;
 
    m_nVertices = 15;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
 
    CDiffusedVertex pVertices[15];
 
    // --- 동체 (Fuselage) ---
    pVertices[0]  = CDiffusedVertex(XMFLOAT3(0.0f,         0.0f,        2.0f * s),  RANDOM_COLOR); // 노즈
    pVertices[1]  = CDiffusedVertex(XMFLOAT3(-0.5f * s,   -0.5f * s,    1.0f * s),  RANDOM_COLOR); // 좌하
    pVertices[2]  = CDiffusedVertex(XMFLOAT3(+0.5f * s,   -0.5f * s,    1.0f * s),  RANDOM_COLOR); // 우하
    pVertices[3]  = CDiffusedVertex(XMFLOAT3(+0.5f * s,   +0.5f * s,    1.0f * s),  RANDOM_COLOR); // 우상
    pVertices[4]  = CDiffusedVertex(XMFLOAT3(-0.5f * s,   +0.5f * s,    1.0f * s),  RANDOM_COLOR); // 좌상
    pVertices[5]  = CDiffusedVertex(XMFLOAT3(-0.25f * s,  -0.25f * s,   0.0f),       RANDOM_COLOR); // 좌하 (꼬리쪽)
    pVertices[6]  = CDiffusedVertex(XMFLOAT3(+0.25f * s,  -0.25f * s,   0.0f),       RANDOM_COLOR); // 우하
    pVertices[7]  = CDiffusedVertex(XMFLOAT3(+0.25f * s,  +0.25f * s,   0.0f),       RANDOM_COLOR); // 우상
    pVertices[8]  = CDiffusedVertex(XMFLOAT3(-0.25f * s,  +0.25f * s,   0.0f),       RANDOM_COLOR); // 좌상
    pVertices[9]  = CDiffusedVertex(XMFLOAT3(0.0f,         0.0f,       -1.0f * s),  RANDOM_COLOR); // 꼬리 끝
 
    // --- 주익 (Delta Wings) ---
    pVertices[10] = CDiffusedVertex(XMFLOAT3(-3.0f * s,   0.0f,         1.0f * s),  RANDOM_COLOR); // 좌익 끝
    pVertices[11] = CDiffusedVertex(XMFLOAT3(+3.0f * s,   0.0f,         1.0f * s),  RANDOM_COLOR); // 우익 끝
 
    // --- 수직 꼬리날개 ---
    pVertices[12] = CDiffusedVertex(XMFLOAT3(0.0f,        1.5f * s,    0.0f),       RANDOM_COLOR);
 
    // --- 수평 꼬리날개 ---
    pVertices[13] = CDiffusedVertex(XMFLOAT3(-2.0f * s,   0.0f,         0.0f),       RANDOM_COLOR);
    pVertices[14] = CDiffusedVertex(XMFLOAT3(+2.0f * s,   0.0f,         0.0f),       RANDOM_COLOR);
 
    CalculateBoundingBoxExtents(pVertices, m_nVertices);
 
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pVertices, m_nStride * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = m_nStride;
    m_d3dVertexBufferView.SizeInBytes    = m_nStride * m_nVertices;
 
    // [기존 GDI와 동일한 인덱스]
    m_nIndices = 87;
    UINT pnIndices[87] = {
        // --- 노즈 부분 (4개 삼각형) ---
        0, 3, 4,
        0, 4, 1,
        0, 2, 3,
        0, 1, 2,
        // --- 동체 중간 (직육면체) ---
        1, 4, 8, 1, 8, 5, // 좌측면
        4, 3, 7, 4, 7, 8, // 상단면
        3, 2, 6, 3, 6, 7, // 우측면
        2, 1, 5, 2, 5, 6, // 하단면
        // --- 동체 후방 (4개 삼각형) ---
        5, 8, 9,
        8, 7, 9,
        7, 6, 9,
        6, 5, 9,
        // --- 동체 단면 마감 ---
        5, 6, 7, 5, 7, 8,
        1, 2, 3, 1, 3, 4,
        // --- 주익 ---
        4, 1, 10,
        1, 4, 10,
        2, 3, 11,
        3, 2, 11,
        // --- 수직 꼬리날개 ---
        8, 12, 7,
        // --- 수평 꼬리날개 ---
        8, 13, 5,
        5, 13, 8,
        7, 14, 6,
        6, 14, 7
    };
 
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pnIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}
 
CPlayerMesh::~CPlayerMesh() {}
 
 
// ═════════════════════════════════════════════════════════
// CEnemyMesh — 팔면체 본체 + 다리 + 안테나 (12 정점, 26 삼각형 / 78 인덱스)
// [보존] 기존 GDI 버전과 동일
// ═════════════════════════════════════════════════════════
CEnemyMesh::CEnemyMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                     float fScale)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    float s = fScale * 0.5f;
    float ls = s * 1.5f;
 
    m_nVertices = 12;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
 
    CDiffusedVertex pVertices[12];
 
    // --- 본체 (팔면체 6 정점) ---
    pVertices[0] = CDiffusedVertex(XMFLOAT3(0.0f,  s,    0.0f), RANDOM_COLOR);   // 위
    pVertices[1] = CDiffusedVertex(XMFLOAT3(0.0f, -s,    0.0f), RANDOM_COLOR);   // 아래
    pVertices[2] = CDiffusedVertex(XMFLOAT3(0.0f,  0.0f, s),    RANDOM_COLOR);   // 앞
    pVertices[3] = CDiffusedVertex(XMFLOAT3(s,    0.0f,  0.0f), RANDOM_COLOR);   // 우
    pVertices[4] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -s),    RANDOM_COLOR);   // 뒤
    pVertices[5] = CDiffusedVertex(XMFLOAT3(-s,   0.0f,  0.0f), RANDOM_COLOR);   // 좌
 
    // --- 다리 (피라미드 끝점 4개) ---
    pVertices[6] = CDiffusedVertex(XMFLOAT3( ls, -ls,  ls), RANDOM_COLOR);
    pVertices[7] = CDiffusedVertex(XMFLOAT3( ls, -ls, -ls), RANDOM_COLOR);
    pVertices[8] = CDiffusedVertex(XMFLOAT3(-ls, -ls, -ls), RANDOM_COLOR);
    pVertices[9] = CDiffusedVertex(XMFLOAT3(-ls, -ls,  ls), RANDOM_COLOR);
 
    // --- 안테나 (위쪽 피라미드 끝점 2개) ---
    pVertices[10] = CDiffusedVertex(XMFLOAT3(-s, ls, ls), RANDOM_COLOR);
    pVertices[11] = CDiffusedVertex(XMFLOAT3( s, ls, ls), RANDOM_COLOR);
 
    CalculateBoundingBoxExtents(pVertices, m_nVertices);
 
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pVertices, m_nStride * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = m_nStride;
    m_d3dVertexBufferView.SizeInBytes    = m_nStride * m_nVertices;
 
    m_nIndices = 78;
    UINT pnIndices[78] = {
        // --- 본체 (팔면체 8 삼각형) ---
        0, 2, 3,   0, 3, 4,   0, 4, 5,   0, 5, 2,   // 상단 4개
        1, 3, 2,   1, 4, 3,   1, 5, 4,   1, 2, 5,   // 하단 4개
        // --- 다리 (각 다리 3 삼각형 × 4 = 12) ---
        1, 2, 6,   2, 3, 6,   3, 1, 6,
        1, 3, 7,   3, 4, 7,   4, 1, 7,
        1, 4, 8,   4, 5, 8,   5, 1, 8,
        1, 5, 9,   5, 2, 9,   2, 1, 9,
        // --- 안테나 (각 3 삼각형 × 2 = 6) ---
        0, 5, 10,  5, 2, 10,  2, 0, 10,
        0, 2, 11,  2, 3, 11,  3, 0, 11
    };
 
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pnIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}
 
CEnemyMesh::~CEnemyMesh() {}
 
 
// ═════════════════════════════════════════════════════════
// CWallMesh — 첨탑형 (9 정점, 14 삼각형 / 42 인덱스)
// ═════════════════════════════════════════════════════════
CWallMesh::CWallMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   float fWidth, float fHeight, float fDepth)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    float w = fWidth * 0.5f;
    float d = fDepth * 0.5f;
    float h = fHeight;
    float sh = h * 0.8f;     // 어깨 높이
    float sw = w * 0.6f;
    float sd = d * 0.6f;
 
    m_nVertices = 9;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
 
    CDiffusedVertex pVertices[9];
    // 바닥 (0~3)
    pVertices[0] = CDiffusedVertex(XMFLOAT3(-w, 0.0f, -d),    RANDOM_COLOR);
    pVertices[1] = CDiffusedVertex(XMFLOAT3( w, 0.0f, -d),    RANDOM_COLOR);
    pVertices[2] = CDiffusedVertex(XMFLOAT3( w, 0.0f,  d),    RANDOM_COLOR);
    pVertices[3] = CDiffusedVertex(XMFLOAT3(-w, 0.0f,  d),    RANDOM_COLOR);
    // 어깨 (4~7)
    pVertices[4] = CDiffusedVertex(XMFLOAT3(-sw, sh, -sd),    RANDOM_COLOR);
    pVertices[5] = CDiffusedVertex(XMFLOAT3( sw, sh, -sd),    RANDOM_COLOR);
    pVertices[6] = CDiffusedVertex(XMFLOAT3( sw, sh,  sd),    RANDOM_COLOR);
    pVertices[7] = CDiffusedVertex(XMFLOAT3(-sw, sh,  sd),    RANDOM_COLOR);
    // 꼭짓점 (8)
    pVertices[8] = CDiffusedVertex(XMFLOAT3(0.0f, h, 0.0f),   RANDOM_COLOR);
 
    CalculateBoundingBoxExtents(pVertices, m_nVertices);
 
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pVertices, m_nStride * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = m_nStride;
    m_d3dVertexBufferView.SizeInBytes    = m_nStride * m_nVertices;
 
    m_nIndices = 42;
    UINT pnIndices[42] = {
        // 측면 4개 (각 사다리꼴 = 2 삼각형)
        0, 4, 5,   0, 5, 1,
        1, 5, 6,   1, 6, 2,
        2, 6, 7,   2, 7, 3,
        3, 7, 4,   3, 4, 0,
        // 상단 피라미드 (4 삼각형)
        4, 8, 5,
        5, 8, 6,
        6, 8, 7,
        7, 8, 4,
        // 바닥 (2 삼각형)
        0, 1, 2,   0, 2, 3
    };
 
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pnIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}
 
CWallMesh::~CWallMesh() {}
 
 
// ═════════════════════════════════════════════════════════
// CMissileMesh — 사면체 (4 정점, 4 삼각형 / 12 인덱스)
// ═════════════════════════════════════════════════════════
CMissileMesh::CMissileMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                         float fScale)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    float s = fScale * 0.5f;
 
    m_nVertices = 4;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
 
    // 미사일은 노란색 고정 — 기존 SetColor(RGB(255,255,0))에 대응
    XMFLOAT4 xmf4MissileColor(1.0f, 1.0f, 0.0f, 1.0f);
 
    CDiffusedVertex pVertices[4];
    pVertices[0] = CDiffusedVertex(XMFLOAT3(0.0f,  0.0f,  s),  xmf4MissileColor); // 끝점
    pVertices[1] = CDiffusedVertex(XMFLOAT3(-s,   -s,   -s),  xmf4MissileColor);
    pVertices[2] = CDiffusedVertex(XMFLOAT3( s,   -s,   -s),  xmf4MissileColor);
    pVertices[3] = CDiffusedVertex(XMFLOAT3(0.0f,  s,   -s),  xmf4MissileColor);
 
    CalculateBoundingBoxExtents(pVertices, m_nVertices);
 
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pVertices, m_nStride * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = m_nStride;
    m_d3dVertexBufferView.SizeInBytes    = m_nStride * m_nVertices;
 
    m_nIndices = 12;
    UINT pnIndices[12] = {
        0, 3, 1,
        0, 2, 3,
        0, 1, 2,
        1, 3, 2
    };
 
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pnIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}
 
CMissileMesh::~CMissileMesh() {}

const char* GetCharBitmap(char c)
{
    switch (toupper(c))
    {
    case 'A': return "01110" "10001" "11111" "10001" "10001";
    case 'B': return "11110" "10001" "11110" "10001" "11110";
    case 'C': return "01111" "10000" "10000" "10000" "01111";
    case 'D': return "11110" "10001" "10001" "10001" "11110";
    case 'E': return "11111" "10000" "11110" "10000" "11111";
    case 'F': return "11111" "10000" "11110" "10000" "10000";
    case 'G': return "01111" "10000" "10011" "10001" "01111";
    case 'H': return "10001" "10001" "11111" "10001" "10001";
    case 'I': return "01110" "00100" "00100" "00100" "01110";
    case 'J': return "00011" "00001" "00001" "10001" "01110";
    case 'K': return "10001" "10010" "11100" "10010" "10001";
    case 'L': return "10000" "10000" "10000" "10000" "11111";
    case 'M': return "11011" "10101" "10001" "10001" "10001";
    case 'N': return "10001" "11001" "10101" "10011" "10001";
    case 'O': return "01110" "10001" "10001" "10001" "01110";
    case 'P': return "11110" "10001" "11110" "10000" "10000";
    case 'Q': return "01110" "10001" "10101" "10010" "01101";
    case 'R': return "11110" "10001" "11110" "10100" "10011";
    case 'S': return "01111" "10000" "01110" "00001" "11111";
    case 'T': return "11111" "00100" "00100" "00100" "00100";
    case 'U': return "10001" "10001" "10001" "10001" "01110";
    case 'V': return "10001" "10001" "10001" "01010" "00100";
    case 'W': return "10001" "10001" "10101" "11011" "10001";
    case 'X': return "10001" "01010" "00100" "01010" "10001";
    case 'Y': return "10001" "01010" "00100" "00100" "00100";
    case 'Z': return "11111" "00010" "00100" "01000" "11111";

        // === 숫자 0 ~ 9 ===
    case '0': return "01110" "10011" "10101" "11001" "01110";
    case '1': return "00100" "01100" "00100" "00100" "01110";
    case '2': return "01110" "10001" "00110" "01000" "11111";
    case '3': return "11111" "00010" "00110" "00001" "11110";
    case '4': return "00010" "00110" "01010" "11111" "00010";
    case '5': return "11111" "10000" "11110" "00001" "11110";
    case '6': return "00110" "01000" "11110" "10001" "01110";
    case '7': return "11111" "00001" "00010" "00100" "00100";
    case '8': return "01110" "10001" "01110" "10001" "01110";
    case '9': return "01110" "10001" "01111" "00010" "01100";

        // === 특수문자 및 공백 ===
    case '!': return "00100" "00100" "00100" "00000" "00100";
    case '?': return "01110" "10001" "00010" "00100" "00100";
    case '-': return "00000" "00000" "11111" "00000" "00000";
    case ' ': return "00000" "00000" "00000" "00000" "00000";

        // 매칭되지 않는 문자는 속이 빈 네모 박스로 표시
    default:  return "11111" "10001" "10101" "10001" "11111";
    }
}


C3DTextMesh::C3DTextMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, std::string text, float fScale, XMFLOAT4 color)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    int totalCubes = 0;
    for (char c : text) {
        const char* bitmap = GetCharBitmap(c);
        for (int i = 0; i < 25; i++) if (bitmap[i] == '1') totalCubes++;
    }

    if (totalCubes == 0) return;

    m_nVertices = totalCubes * 8;
    m_nIndices = totalCubes * 36;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CDiffusedVertex* pVertices = new CDiffusedVertex[m_nVertices];
    UINT* pnIndices = new UINT[m_nIndices];

    float h = fScale * 0.5f;
    float charSpacing = fScale * 6.0f;
    float totalWidth = text.length() * charSpacing - fScale;
    float startX = -totalWidth * 0.5f;
    int cubeIdx = 0;
    float cursorX = startX;

    for (char c : text)
    {
        const char* bitmap = GetCharBitmap(c);
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                if (bitmap[row * 5 + col] == '1') {
                    float cx = cursorX + (col * fScale);
                    float cy = (2 - row) * fScale;
                    float cz = 0.0f;
                    int v = cubeIdx * 8;

                    pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(cx - h, cy + h, cz - h), color);
                    pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(cx + h, cy + h, cz - h), color);
                    pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(cx + h, cy - h, cz - h), color);
                    pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(cx - h, cy - h, cz - h), color);
                    pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(cx - h, cy + h, cz + h), color);
                    pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(cx + h, cy + h, cz + h), color);
                    pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(cx + h, cy - h, cz + h), color);
                    pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(cx - h, cy - h, cz + h), color);

                    int i = cubeIdx * 36;
                    UINT indices[36] = {
                        (UINT)v + 0, (UINT)v + 1, (UINT)v + 2,  (UINT)v + 0, (UINT)v + 2, (UINT)v + 3,
                        (UINT)v + 4, (UINT)v + 6, (UINT)v + 5,  (UINT)v + 4, (UINT)v + 7, (UINT)v + 6,
                        (UINT)v + 4, (UINT)v + 5, (UINT)v + 1,  (UINT)v + 4, (UINT)v + 1, (UINT)v + 0,
                        (UINT)v + 3, (UINT)v + 2, (UINT)v + 6,  (UINT)v + 3, (UINT)v + 6, (UINT)v + 7,
                        (UINT)v + 1, (UINT)v + 5, (UINT)v + 6,  (UINT)v + 1, (UINT)v + 6, (UINT)v + 2,
                        (UINT)v + 4, (UINT)v + 0, (UINT)v + 3,  (UINT)v + 4, (UINT)v + 3, (UINT)v + 7
                    };
                    memcpy(&pnIndices[i], indices, sizeof(UINT) * 36);
                    cubeIdx++;
                }
            }
        }
        cursorX += charSpacing;
    }

    CalculateBoundingBoxExtents(pVertices, m_nVertices);

    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList
        , pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT
        , D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = m_nStride;
    m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList
        , pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT
        , D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

    delete[] pVertices;
    delete[] pnIndices;
}
C3DTextMesh::~C3DTextMesh() {}

// ═════════════════════════════════════════════════════════
// 2. CStairMesh — 단차 맵 구현용 3단 계단 메쉬
// ═════════════════════════════════════════════════════════
CStairMesh::CStairMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    int nSteps = 3;
    m_nVertices = nSteps * 8;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CDiffusedVertex* pVertices = new CDiffusedVertex[m_nVertices];

    float stepW = fWidth * 0.5f;
    float stepH = fHeight / nSteps;
    float stepD = fDepth / nSteps;
    float startZ = -fDepth * 0.5f;
    XMFLOAT4 stairColor(0.6f, 0.6f, 0.6f, 1.0f);

    for (int i = 0; i < nSteps; i++) {
        float currentY = stepH * (i + 1);
        float zFront = startZ + (stepD * i);
        float zBack = startZ + (stepD * (i + 1));
        int vIdx = i * 8;

        pVertices[vIdx + 0] = CDiffusedVertex(XMFLOAT3(-stepW, currentY, zFront), stairColor);
        pVertices[vIdx + 1] = CDiffusedVertex(XMFLOAT3(stepW, currentY, zFront), stairColor);
        pVertices[vIdx + 2] = CDiffusedVertex(XMFLOAT3(stepW, 0.0f, zFront), stairColor);
        pVertices[vIdx + 3] = CDiffusedVertex(XMFLOAT3(-stepW, 0.0f, zFront), stairColor);
        pVertices[vIdx + 4] = CDiffusedVertex(XMFLOAT3(-stepW, currentY, zBack), stairColor);
        pVertices[vIdx + 5] = CDiffusedVertex(XMFLOAT3(stepW, currentY, zBack), stairColor);
        pVertices[vIdx + 6] = CDiffusedVertex(XMFLOAT3(stepW, 0.0f, zBack), stairColor);
        pVertices[vIdx + 7] = CDiffusedVertex(XMFLOAT3(-stepW, 0.0f, zBack), stairColor);
    }

    CalculateBoundingBoxExtents(pVertices, m_nVertices);

    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = m_nStride;
    m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

    m_nIndices = nSteps * 36;
    UINT* pnIndices = new UINT[m_nIndices];

    for (int i = 0; i < nSteps; i++) {
        int iIdx = i * 36;
        UINT vIdx = i * 8;
        UINT stepIndices[36] = {
            vIdx + 0, vIdx + 1, vIdx + 2,  vIdx + 0, vIdx + 2, vIdx + 3,
            vIdx + 4, vIdx + 6, vIdx + 5,  vIdx + 4, vIdx + 7, vIdx + 6,
            vIdx + 4, vIdx + 5, vIdx + 1,  vIdx + 4, vIdx + 1, vIdx + 0,
            vIdx + 3, vIdx + 2, vIdx + 6,  vIdx + 3, vIdx + 6, vIdx + 7,
            vIdx + 1, vIdx + 5, vIdx + 6,  vIdx + 1, vIdx + 6, vIdx + 2,
            vIdx + 4, vIdx + 0, vIdx + 3,  vIdx + 4, vIdx + 3, vIdx + 7
        };
        memcpy(&pnIndices[iIdx], stepIndices, sizeof(UINT) * 36);
    }

    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

    delete[] pVertices;
    delete[] pnIndices;
}
CStairMesh::~CStairMesh() {}

// ═════════════════════════════════════════════════════════
// 3. CJointPartMesh — 관절(Pivot) 중심의 계층구조 바디 파트
// ═════════════════════════════════════════════════════════
CJointPartMesh::CJointPartMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, XMFLOAT4 color)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    m_nVertices = 8;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    float w = fWidth * 0.5f;
    float d = fDepth * 0.5f;
    float h = fHeight;

    CDiffusedVertex pVertices[8];
    pVertices[0] = CDiffusedVertex(XMFLOAT3(-w, 0.0f, -d), color);
    pVertices[1] = CDiffusedVertex(XMFLOAT3(w, 0.0f, -d), color);
    pVertices[2] = CDiffusedVertex(XMFLOAT3(w, 0.0f, d), color);
    pVertices[3] = CDiffusedVertex(XMFLOAT3(-w, 0.0f, d), color);
    pVertices[4] = CDiffusedVertex(XMFLOAT3(-w, -h, -d), color);
    pVertices[5] = CDiffusedVertex(XMFLOAT3(w, -h, -d), color);
    pVertices[6] = CDiffusedVertex(XMFLOAT3(w, -h, d), color);
    pVertices[7] = CDiffusedVertex(XMFLOAT3(-w, -h, d), color);

    CalculateBoundingBoxExtents(pVertices, m_nVertices);

    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = m_nStride;
    m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

    m_nIndices = 36;
    UINT pnIndices[36] = {
        0, 1, 5,  0, 5, 4,
        3, 2, 6,  3, 6, 7,
        0, 3, 2,  0, 2, 1,
        4, 5, 6,  4, 6, 7,
        1, 2, 6,  1, 6, 5,
        0, 4, 7,  0, 7, 3
    };

    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}
CJointPartMesh::~CJointPartMesh() {}

// ═════════════════════════════════════════════════════════
// 4. CMutantMesh — 뿔이 달린 돌격형 적 캐릭터
// ═════════════════════════════════════════════════════════
CMutantMesh::CMutantMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fScale)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    float s = fScale;
    m_nVertices = 14;
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    XMFLOAT4 skin(0.3f, 0.5f, 0.1f, 1.0f);
    XMFLOAT4 horn(0.8f, 0.1f, 0.1f, 1.0f);

    CDiffusedVertex pVertices[14];
    pVertices[0] = CDiffusedVertex(XMFLOAT3(-s, s, -s), skin);
    pVertices[1] = CDiffusedVertex(XMFLOAT3(s, s, -s), skin);
    pVertices[2] = CDiffusedVertex(XMFLOAT3(s, 0.0f, -s), skin);
    pVertices[3] = CDiffusedVertex(XMFLOAT3(-s, 0.0f, -s), skin);
    pVertices[4] = CDiffusedVertex(XMFLOAT3(-s, s, s), skin);
    pVertices[5] = CDiffusedVertex(XMFLOAT3(s, s, s), skin);
    pVertices[6] = CDiffusedVertex(XMFLOAT3(s, 0.0f, s), skin);
    pVertices[7] = CDiffusedVertex(XMFLOAT3(-s, 0.0f, s), skin);
    pVertices[8] = CDiffusedVertex(XMFLOAT3(-s * 2.0f, s * 1.5f, 0.0f), horn);
    pVertices[9] = CDiffusedVertex(XMFLOAT3(s * 2.0f, s * 1.5f, 0.0f), horn);
    pVertices[10] = CDiffusedVertex(XMFLOAT3(-s * 0.5f, -s, -s * 0.5f), skin);
    pVertices[11] = CDiffusedVertex(XMFLOAT3(s * 0.5f, -s, -s * 0.5f), skin);
    pVertices[12] = CDiffusedVertex(XMFLOAT3(s * 0.5f, -s, s * 0.5f), skin);
    pVertices[13] = CDiffusedVertex(XMFLOAT3(-s * 0.5f, -s, s * 0.5f), skin);

    CalculateBoundingBoxExtents(pVertices, m_nVertices);

    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = m_nStride;
    m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

    m_nIndices = 72;
    UINT pnIndices[72] = {
        0, 1, 2,  0, 2, 3,
        5, 4, 7,  5, 7, 6,
        4, 5, 1,  4, 1, 0,
        3, 2, 6,  3, 6, 7,
        0, 4, 8,  4, 7, 8,  7, 3, 8,  3, 0, 8,
        1, 5, 9,  5, 6, 9,  6, 2, 9,  2, 1, 9,
        3, 2, 11,  3, 11, 10,
        2, 6, 12,  2, 12, 11,
        6, 7, 13,  6, 13, 12,
        7, 3, 10,  7, 10, 13
    };

    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}
CMutantMesh::~CMutantMesh() {}

CGunMesh::CGunMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fScale)
    : CMesh(pd3dDevice, pd3dCommandList)
{
    m_nVertices = 32; // 4개 파츠(총몸, 총열, 탄창, 손잡이) * 각 8정점
    m_nStride = sizeof(CDiffusedVertex);
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    CDiffusedVertex* pVertices = new CDiffusedVertex[m_nVertices];

    XMFLOAT4 colorBody(0.25f, 0.25f, 0.25f, 1.0f);   // 짙은 회색 (총몸)
    XMFLOAT4 colorBarrel(0.45f, 0.45f, 0.45f, 1.0f); // 은회색 (총열)
    XMFLOAT4 colorMag(0.15f, 0.15f, 0.15f, 1.0f);    // 검정색 (탄창)
    XMFLOAT4 colorGrip(0.4f, 0.2f, 0.1f, 1.0f);      // 갈색 나무결 (손잡이)

    // 각 파츠의 [minX, maxX, minY, maxY, minZ, maxZ, 색상] 구조체 정의
    // 주의: +Z 방향이 총구가 바라보는 앞쪽(Forward)입니다.
    struct BoxPart { float minX, maxX, minY, maxY, minZ, maxZ; XMFLOAT4 color; };
    BoxPart parts[4] = {
        // 1. 총몸 (Receiver) - 기준이 되는 중심부 블록
        { -0.1f, 0.1f,  0.0f, 0.25f, -0.4f, 0.7f, colorBody },
        // 2. 총열 (Barrel) - 총몸 앞(+Z)으로 얇고 길게 뻗음
        { -0.04f, 0.04f, 0.1f, 0.18f,  0.7f, 1.4f, colorBarrel },
        // 3. 탄창 (Magazine) - 총몸 앞쪽 아래(-Y)로 돌출
        { -0.06f, 0.06f, -0.4f, 0.0f,  0.3f, 0.55f, colorMag },
        // 4. 손잡이 (Grip) - 방아쇠 부분 아래(-Y)로 돌출. (이 파츠의 최상단이 0,0,0 Pivot 입니다!)
        { -0.08f, 0.08f, -0.35f, 0.0f, -0.3f, -0.1f, colorGrip }
    };

    for (int i = 0; i < 4; i++)
    {
        float minX = parts[i].minX * fScale; float maxX = parts[i].maxX * fScale;
        float minY = parts[i].minY * fScale; float maxY = parts[i].maxY * fScale;
        float minZ = parts[i].minZ * fScale; float maxZ = parts[i].maxZ * fScale;
        int v = i * 8;

        // 뒷면 (-Z 방향) 정점
        pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(minX, maxY, minZ), parts[i].color); // 좌상단
        pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(maxX, maxY, minZ), parts[i].color); // 우상단
        pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(maxX, minY, minZ), parts[i].color); // 우하단
        pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(minX, minY, minZ), parts[i].color); // 좌하단
        // 앞면 (+Z 방향) 정점
        pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(minX, maxY, maxZ), parts[i].color); // 좌상단
        pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(maxX, maxY, maxZ), parts[i].color); // 우상단
        pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(maxX, minY, maxZ), parts[i].color); // 우하단
        pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(minX, minY, maxZ), parts[i].color); // 좌하단
    }

    CalculateBoundingBoxExtents(pVertices, m_nVertices);

    // [정점 버퍼 업로드]
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = m_nStride;
    m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

    // 인덱스 버퍼: 4개의 파츠(육면체) * 36개 인덱스
    m_nIndices = 4 * 36;
    UINT* pnIndices = new UINT[m_nIndices];

    for (int i = 0; i < 4; i++)
    {
        int idx = i * 36;
        UINT v = i * 8; // 각 파츠별 시작 정점 번호
        UINT indices[36] = {
            v + 0, v + 1, v + 2,  v + 0, v + 2, v + 3, // 뒷면
            v + 4, v + 6, v + 5,  v + 4, v + 7, v + 6, // 앞면
            v + 4, v + 5, v + 1,  v + 4, v + 1, v + 0, // 윗면
            v + 3, v + 2, v + 6,  v + 3, v + 6, v + 7, // 아랫면
            v + 1, v + 5, v + 6,  v + 1, v + 6, v + 2, // 우측면
            v + 4, v + 0, v + 3,  v + 4, v + 3, v + 7  // 좌측면
        };
        memcpy(&pnIndices[idx], indices, sizeof(UINT) * 36);
    }

    // [인덱스 버퍼 업로드]
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

    delete[] pVertices;
    delete[] pnIndices;
}
CGunMesh::~CGunMesh() {}