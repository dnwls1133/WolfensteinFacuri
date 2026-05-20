#pragma once

#include "stdafx.h"


// CreateBufferResource - GPU 버퍼 리소스를 생성하는 헬퍼 함수
// 
// Vertex Buffer, Index Buffer, CBuffer 등 GPU 리소스를 생성하는 함수
// 사용처:
//   - CMesh / CCubeMesh / CPlayerMesh 등 : 정점/인덱스 버퍼 생성
//   - CCamera                            : View/Projection 상수 버퍼 (Upload Heap)
//
// 동작:
//   - D3D12_HEAP_TYPE_DEFAULT 사용 시 Upload Heap을 통한 2단계 복사 수행
//     1) CPU → Upload Heap (CPU 접근 가능)
//     2) CommandList의 CopyResource로 Default Heap (GPU 전용)으로 복사
//   - D3D12_HEAP_TYPE_UPLOAD 사용 시 Upload Heap에 직접 생성
//     (상수 버퍼처럼 매 프레임 갱신이 필요한 리소스에 적합)
// ───────────────────────────────────────────────────────

extern ID3D12Resource* CreateBufferResource(
	ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	void* pData,
	UINT nBytes,
	D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_UPLOAD,
	D3D12_RESOURCE_STATES d3dReource = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	ID3D12Resource** ppd3dUploadBuffer = NULL
);