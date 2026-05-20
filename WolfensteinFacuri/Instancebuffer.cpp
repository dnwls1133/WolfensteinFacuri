#include "stdafx.h"
#include "Instancebuffer.h"
#include "GraphicsHelpers.h"

CInstancebuffer::CInstancebuffer()
{
	::ZeroMemory(&m_d3dVertexBufferView, sizeof(m_d3dVertexBufferView));
}

CInstancebuffer::~CInstancebuffer()
{
	Release();
}


// Upload Heap 버퍼 생성 + 영구 매핑 
void CInstancebuffer::Create(ID3D12Device* pd3dDevice, UINT nMaxInstances)
{
	m_nMaxInstances = nMaxInstances;
	m_nCurrentCount = 0;

	UINT nBufferSize = sizeof(InstanceData) * m_nMaxInstances;

	m_pd3dInstanceBuffer = ::CreateBufferResource(pd3dDevice,
		NULL, NULL, nBufferSize,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL);

	D3D12_RANGE d3dReadRange = { 0, 0 };
	m_pd3dInstanceBuffer->Map(0, &d3dReadRange, (void**)&m_pMappedData);

	m_d3dVertexBufferView.BufferLocation = m_pd3dInstanceBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = sizeof(InstanceData);
	m_d3dVertexBufferView.SizeInBytes = nBufferSize;

}

void CInstancebuffer::AddInstance(const XMFLOAT4X4& world, const XMFLOAT4& color)
{
	if (m_nCurrentCount > m_nMaxInstances) return;
	if (!m_pMappedData) return;

	m_pMappedData[m_nCurrentCount].world = world;
	m_pMappedData[m_nCurrentCount].color = color;
	++m_nCurrentCount;
}



// 정점 버퍼 슬롯 1에 바인딩
void CInstancebuffer::Bind(ID3D12GraphicsCommandList* pd3dCommandList, UINT nSlot)
{
	if (m_nCurrentCount == 0) return;
	pd3dCommandList->IASetVertexBuffers(nSlot, 1, &m_d3dVertexBufferView);
}

void CInstancebuffer::Release()
{
	if (m_pd3dInstanceBuffer)
	{
		m_pd3dInstanceBuffer->Unmap(0, NULL);
		m_pd3dInstanceBuffer->Release();
		m_pd3dInstanceBuffer = NULL;
	}
	m_pMappedData = NULL;
	m_nCurrentCount = 0;
	m_nMaxInstances = 0;
}