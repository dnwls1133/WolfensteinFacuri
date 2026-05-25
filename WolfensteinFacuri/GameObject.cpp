// GameObject.cpp
// [Phase 5 변환]
// - Render: GDI Polygon 루프 + 조명 계산 (~80줄) → D3D12 명령 기록 (3줄)
// - Update, Move, Rotate, GenerateBoundingBox는 100% 보존
//

#include "stdafx.h"
#include "GameObject.h"
#include "GraphicsHelpers.h"


UINT CGameObject::g_NextObjectID = 0;

CGameObject::CGameObject()
{
	m_nObjectID = g_NextObjectID++;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf4Rotation = Vector4::QuaternionIdentity();
	m_xmf4x4World = Matrix4x4::Identity();
	m_dwColor = RGB(255, 255, 255);
}

CGameObject::~CGameObject()
{
	ReleaseShaderVariables();
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader) m_pShader->Release();
}

void CGameObject::GenerateBoundingBox()
{
	XMFLOAT3 localCenter = m_pMesh->GetLocalCenter();
	XMVECTOR vOffset = XMLoadFloat3(&localCenter);
	XMVECTOR vRotated = XMVector3Rotate(vOffset, XMLoadFloat4(&m_xmf4Rotation));
	XMFLOAT3 worldCenter;
	XMStoreFloat3(&worldCenter, XMLoadFloat3(&m_xmf3Position) + vRotated);

	m_xmOOBB.Center = worldCenter;
	m_xmOOBB.Extents = m_pMesh->GetBoundingBoxExtents();
	m_xmOOBB.Orientation = m_xmf4Rotation;
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
	
}

void CGameObject::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf3Position = XMFLOAT3(x, y, z);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	// 1. 현재 나의 회전 쿼터니언을 가져옵니다.
	XMVECTOR qCurrent = XMLoadFloat4(&m_xmf4Rotation);
	// 2. 파라미터로 받은 '축'과 '각도'를 바탕으로 새로운 회전 쿼터니언을 만듭니다.
	XMVECTOR qRotate = XMQuaternionRotationAxis(XMLoadFloat3(pxmf3Axis), fAngle);
	// 3. 쿼터니언끼리 곱셈을 하여 나의 최종 회전값을 갱신합니다.
	XMVECTOR qNew = XMQuaternionMultiply(qRotate, qCurrent);
	qNew = XMQuaternionNormalize(qNew);

	
	XMStoreFloat4(&m_xmf4Rotation, qNew);
}
void CGameObject::Move(XMFLOAT3& dir, float distance)
{
	float fTimeElapsed = TIMER->GetTimeElapsed();
	m_xmf3Position = Vector3::Add(m_xmf3Position, dir, distance * fTimeElapsed);
}

void CGameObject::Update()
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(&m_xmf4Rotation));
	XMMATRIX mtxTranslate = XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z);
	// salce * rotation * translation
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, mtxTranslate);

	XMFLOAT3 localCenter = m_pMesh->GetLocalCenter();
	XMVECTOR vOffset = XMLoadFloat3(&localCenter);
	XMVECTOR vRotated = XMVector3Rotate(vOffset, XMLoadFloat4(&m_xmf4Rotation));
	XMFLOAT3 worldCenter;
	XMStoreFloat3(&worldCenter, XMLoadFloat3(&m_xmf3Position) + vRotated);


	m_xmOOBB.Center = worldCenter;
	m_xmOOBB.Orientation = m_xmf4Rotation;

}

// [D3D12 추가] CBV 버퍼 생성 + 영구 매핑

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementSize = (sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255; // 256바이트 정렬

	m_pd3dcbGameObject = ::CreateBufferResource(
		pd3dDevice, pd3dCommandList, NULL, ncbElementSize
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	// 매핑
	m_pd3dcbGameObject->Map(0, NULL, (void**)&m_pcbMappedGameObject);

	// [추가] 디버그 와이어프레임 전용 CB (별도 버퍼라 일반 패스와 충돌 없음)
	m_pd3dcbDebug = ::CreateBufferResource(
		pd3dDevice, pd3dCommandList, NULL, ncbElementSize
		, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);
	m_pd3dcbDebug->Map(0, NULL, (void**)&m_pcbMappedDebug);
}


// [추가] 셰이더 상수 갱신 - 월드 행렬을 b0 슬롯에 업로드

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pcbMappedGameObject)
	{
		XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World,
			XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

		m_pcbMappedGameObject->m_xmf4Color = m_xmf4Color;
	}

	// b0 슬롯에 GPU 가상 주소 바인딩
	if (m_pcbMappedGameObject)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbGameObject->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(0	// b0 슬롯에 월드 행렬 CBV 바인딩
			, d3dGpuVirtualAddress); 
	}
}


void CGameObject::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
		m_pd3dcbGameObject = NULL;
	}
	m_pcbMappedGameObject = NULL;

	if (m_pd3dcbDebug)
	{
		m_pd3dcbDebug->Unmap(0, NULL);
		m_pd3dcbDebug->Release();
		m_pd3dcbDebug = NULL;
	}
	m_pcbMappedDebug = NULL;
}

// [변경]
// [변경] Render — 기존 GDI 코드 80여 줄을 D3D12 3줄로 압축
//
// 기존 GDI 버전의 작업:
//   - 정점마다 Project / ScreenTransform 호출
//   - 삼각형마다 외적 Z 부호로 후면 제거
//   - 법선·광원 내적으로 Diffuse 조명 계산
//   - SetDCBrushColor / SetDCPenColor / Polygon 호출
//
// D3D12 변환 후:
//   - PSO 바인딩 (PSO에 모든 파이프라인 상태가 묶여 있음)
//   - 월드 행렬을 상수 버퍼(b0)에 업로드
//   - DrawIndexedInstanced 1회 호출 (메시 내부에서)
//
// 모든 변환 / 컬링 / 클리핑 / 조명은 GPU가 자동 처리한다.

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (!m_pMesh) return;

	// 1. PSO 바인딩
	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);

	// 2. 월드 행렬을 b0 슬롯에 업로드
	UpdateShaderVariables(pd3dCommandList);

	// 3. 메시 렌더링 (메시 내부에서 DrawIndexedInstanced 호출)
	m_pMesh->Render(pd3dCommandList);



}

// [추가] OOBB 와이어프레임 디버그 렌더
// - OOBB(Center/Extents/Orientation)로부터 World = Scale*Rot*Trans 구성
// - 디버그 전용 CB(m_pd3dcbDebug)에 업로드 후 단위 큐브(±1) 메쉬를 와이어 PSO로 렌더
void CGameObject::RenderDebugBox(ID3D12GraphicsCommandList* pd3dCommandList,
	CMesh* pWireMesh, const XMFLOAT4& xmf4Color)
{
	if (!pWireMesh || !m_pcbMappedDebug) return;

	// OOBB → World 행렬
	XMVECTOR vExtents = XMLoadFloat3(&m_xmOOBB.Extents);
	XMVECTOR vCenter = XMLoadFloat3(&m_xmOOBB.Center);
	XMVECTOR vQuat = XMLoadFloat4(&m_xmOOBB.Orientation);

	XMMATRIX mtxScale = XMMatrixScalingFromVector(vExtents);
	XMMATRIX mtxRot = XMMatrixRotationQuaternion(vQuat);
	XMMATRIX mtxTrans = XMMatrixTranslationFromVector(vCenter);
	XMMATRIX mtxWorld = mtxScale * mtxRot * mtxTrans;

	// HLSL은 열우선 → 전치하여 업로드
	XMStoreFloat4x4(&m_pcbMappedDebug->m_xmf4x4World, XMMatrixTranspose(mtxWorld));
	m_pcbMappedDebug->m_xmf4Color = xmf4Color;

	pd3dCommandList->SetGraphicsRootConstantBufferView(0,
		m_pd3dcbDebug->GetGPUVirtualAddress());

	pWireMesh->Render(pd3dCommandList);
}
