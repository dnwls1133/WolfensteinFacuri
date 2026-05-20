// GraphicsHelpers.cpp : D3D12 그래픽스 헬퍼 함수 구현


#include "stdafx.h"
#include "GraphicsHelpers.h"

// ─────────────────────────────────────────────────────────
// CreateBufferResource — GPU 버퍼 생성 헬퍼 구현
//
// 반환값: 생성된 ID3D12Resource* (정점/인덱스/상수 버퍼 본체)
// ppd3dUploadBuffer: Default Heap 사용 시 임시 Upload Buffer를 외부로 반환
//                    (CommandList 실행 완료 후 Release 필요)
// ────────────────────────────────────────────────────────
ID3D12Resource* CreateBufferResource(
	ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList,
	void* pData,
	UINT nBytes,
	D3D12_HEAP_TYPE d3dHeapType,
	D3D12_RESOURCE_STATES d3dReourceStates,
	ID3D12Resource** ppd3dUploadBuffer)
{
	ID3D12Resource* pd3dBuffer = NULL;

	// 1. 버퍼 리소스 정보 구조체 작성
	D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
	::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapPropertiesDesc.Type = d3dHeapType;
	d3dHeapPropertiesDesc.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapPropertiesDesc.CreationNodeMask = 1;
	d3dHeapPropertiesDesc.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = nBytes;
	d3dResourceDesc.Height = 1;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // 업로드 힙은 CPU에서 쓰기 편한 레이아웃으로 생성
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 2. Heap Type에 따라 초기 리소스 상태 결정
	D3D12_RESOURCE_STATES d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST; // Default Heap은 복사 대상이므로 COPY_DEST로 시작
	if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD)
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_GENERIC_READ; // Upload Heap은 CPU에서 쓰기 편한 상태로 시작
	}
	else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK)
	{
		d3dResourceInitialStates = D3D12_RESOURCE_STATE_COPY_DEST; // Readback Heap은 복사 대상이므로 COPY_DEST로 시작
	}

	// 3. GPU 측 메인 버퍼 (Default or Upload Heap) 생성
	HRESULT hResult = pd3dDevice->CreateCommittedResource(
		&d3dHeapPropertiesDesc,
		D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc,
		d3dResourceInitialStates,
		NULL,
		__uuidof(ID3D12Resource),
		(void**)&pd3dBuffer);

	if (FAILED(hResult))
	{
		MessageBox(NULL, L"버퍼 리소스 생성 실패", L"오류", MB_OK);
		return NULL;
	}

	// 4. Upload Heap을 통한 데이터 복사 (Default Heap인 경우)
	if (SUCCEEDED(hResult) && pData)
	{
		switch (d3dHeapType)
		{
		case D3D12_HEAP_TYPE_DEFAULT:
		{
			// CPU에서 직접 접근 가능한 Upload Heap 별도 생성
			if (ppd3dUploadBuffer)
			{
				d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
				pd3dDevice->CreateCommittedResource(
					&d3dHeapPropertiesDesc,
					D3D12_HEAP_FLAG_NONE,
					&d3dResourceDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					NULL,
					__uuidof(ID3D12Resource),
					(void**)ppd3dUploadBuffer);

				// Upload Heap에 데이터 복사
				D3D12_RANGE d3dReadRange = { 0,0 };
				UINT8* pBufferDataBegin = NULL;
				(*ppd3dUploadBuffer)->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
				memcpy(pBufferDataBegin, pData, nBytes);
				(*ppd3dUploadBuffer)->Unmap(0, NULL);

				// CommandList로 Upload -> Default 복사 명령 기록
				pd3dCommandList->CopyResource(pd3dBuffer, *ppd3dUploadBuffer);

				// 리소스 베리어 : COPY_DEST → 원하는 초기 상태
				D3D12_RESOURCE_BARRIER d3dResourceBarrier;
				::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
				d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				d3dResourceBarrier.Transition.pResource = pd3dBuffer;
				d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				d3dResourceBarrier.Transition.StateAfter = d3dReourceStates;
				d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);
			}
			break;
		}

		case D3D12_HEAP_TYPE_UPLOAD:
		{
			// Upload Heap에 데이터 복사 (CPU에서 직접 접근 가능)
			D3D12_RANGE d3dReadRange = { 0,0 };
			UINT8* pBufferDataBegin = NULL;
			pd3dBuffer->Map(0, &d3dReadRange, (void**)&pBufferDataBegin);
			memcpy(pBufferDataBegin, pData, nBytes);
			pd3dBuffer->Unmap(0, NULL);
			break;
		}
		case D3D12_HEAP_TYPE_READBACK:
			break;
		}
	}

	return pd3dBuffer;
}