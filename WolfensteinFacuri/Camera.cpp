#include "stdafx.h"
#include "Camera.h"
#include "GraphicsHelpers.h"

CCamera::CCamera()
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf4x4View = Matrix4x4::Identity();
	m_xmf4x4Project = Matrix4x4::Identity();
	m_xmf4x4ViewProject = Matrix4x4::Identity();
	
	// [D3D12 추가] 뷰포트와 시저 렉트 초기화
	::ZeroMemory(&m_d3dViewport, sizeof(D3D12_VIEWPORT));
	::ZeroMemory(&m_d3dScissorRect, sizeof(D3D12_RECT));
}

CCamera::~CCamera()
{
	ReleaseShaderVariables();
}

void CCamera::GenerateViewMatrix()
{
	if(Vector3::Length(m_xmf3Look) == 0.0f)
		m_xmf3Look = Vector3::ZAxis();

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	XMFLOAT3 xmf3Cross = Vector3::CrossProduct(m_xmf3Right, m_xmf3Look);
	XMFLOAT3 right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look);
	if (Vector3::Length(right) < EPSILON)
	{
		XMFLOAT3 YAxis = Vector3::YAxis();
		right = Vector3::CrossProduct(YAxis, m_xmf3Look);
	}
	m_xmf3Right = Vector3::Normalize(right);
	xmf3Cross = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right);
	m_xmf3Up = Vector3::Normalize(xmf3Cross);

	XMFLOAT3 xmf3Target = Vector3::Add(m_xmf3Position, m_xmf3Look);
	m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3Target, m_xmf3Up);

}

void CCamera::GenerateProjectionMatrix(float fNearPlane,float fFarPlane, float fFOVAngle)
{
	float fAspectRatio = (float)m_Viewport.m_nWidth / (float)m_Viewport.m_nHeight;
	// PerspectiveFovLH 함수 사용 (원근 투영 행렬 생성)
	m_xmf4x4Project = Matrix4x4::PerspectiveFovLH(fFOVAngle, fAspectRatio, fNearPlane, fFarPlane);

}

void CCamera::SetViewport(int nLeft, int nTop, int nWidth, int nHeight)
{
	m_Viewport.SetViewport(nLeft, nTop, nWidth, nHeight);

	// [D3D12 추가] 래스터라이저 단계용 뷰포트
	m_d3dViewport.TopLeftX = (float)nLeft;
	m_d3dViewport.TopLeftY = (float)nTop;
	m_d3dViewport.Width = (float)nWidth;
	m_d3dViewport.Height = (float)nHeight;
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	// [D3D12 추가] 시저 렉터 (뷰포트 전체)
	m_d3dScissorRect.left = nLeft;
	m_d3dScissorRect.top = nTop;
	m_d3dScissorRect.right = nLeft + nWidth;
	m_d3dScissorRect.bottom = nTop + nHeight;

}

void CCamera::Move(XMFLOAT3& xmf3Shift)
{
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
}

void CCamera::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMVECTOR qRotation = XMQuaternionIdentity();

	if (fPitch != 0.0f)
	{
		
		XMVECTOR qPitch = XMQuaternionRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
		qRotation = XMQuaternionMultiply(qRotation, qPitch);
	}

	if (fYaw != 0.0f)
	{
	
		XMVECTOR qYaw = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(fYaw));
		qRotation = XMQuaternionMultiply(qRotation, qYaw);
	}

	if (fRoll != 0.0f)
	{
		XMVECTOR qRoll = XMQuaternionRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
		qRotation = XMQuaternionMultiply(qRotation, qRoll);
	}

	// 최종 회전 적용
	XMStoreFloat3(&m_xmf3Look, XMVector3Rotate(XMLoadFloat3(&m_xmf3Look), qRotation));
	XMStoreFloat3(&m_xmf3Right, XMVector3Rotate(XMLoadFloat3(&m_xmf3Right), qRotation));
	XMStoreFloat3(&m_xmf3Up, XMVector3Rotate(XMLoadFloat3(&m_xmf3Up), qRotation));
}


void CCamera::Update()
{
	GenerateViewMatrix();
	
	// ViewProjection 행렬을 구해놓으면 렌더링할 때 매번 곱셈을 하지 않아도 됩니다.
	m_xmf4x4ViewProject = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4Project);
	CalulateFrustumPlanes();
}


void CCamera::CalulateFrustumPlanes()
{
	XMFLOAT4X4& M = m_xmf4x4ViewProject;

	// 1. 왼쪽 평면
	m_xmf4FrustumPlanes[0] = XMFLOAT4(
		M._14 + M._11,
		M._24 + M._21,
		M._34 + M._31,
		M._44 + M._41
	);
	{
		// 2. 오른쪽 평면 
		m_xmf4FrustumPlanes[1] = XMFLOAT4(
			M._14 - M._11,
			M._24 - M._21,
			M._34 - M._31,
			M._44 - M._41
		);
		// 3. 위 평면
		m_xmf4FrustumPlanes[2] = XMFLOAT4(
			M._14 - M._12,
			M._24 - M._22,
			M._34 - M._32,
			M._44 - M._42
		);
		// 4. 아래 평면
		m_xmf4FrustumPlanes[3] = XMFLOAT4(
			M._14 + M._12,
			M._24 + M._22,
			M._34 + M._32,
			M._44 + M._42
		);
		// 5. 근 평면 (z' >= 0 : DirectX 기준)
		m_xmf4FrustumPlanes[4] = XMFLOAT4(
			M._13,
			M._23,
			M._33,
			M._43
		);
		// 6. 원 평면
		m_xmf4FrustumPlanes[5] = XMFLOAT4(
			M._14 - M._13,
			M._24 - M._23,
			M._34 - M._33,
			M._44 - M._43
		);
	}
	
	for (int i = 0; i < 6; i++)
	{
		XMVECTOR plane = XMLoadFloat4(&m_xmf4FrustumPlanes[i]);
		plane = XMVector4Normalize(plane);
		XMStoreFloat4(&m_xmf4FrustumPlanes[i], plane);
	}

}

bool CCamera::IsInFrustum(XMFLOAT3& xmf3Center, float fRadius)
{
	XMVECTOR vCenter = XMLoadFloat3(&xmf3Center);
	vCenter = XMVectorSetW(vCenter, 1.0f);

	for (int i = 0; i < 6; i++)
	{
		XMVECTOR plane = XMLoadFloat4(&m_xmf4FrustumPlanes[i]);

		// 내적 연산 
		XMVECTOR vDistance = XMVector4Dot(plane, vCenter);
		float fDistance = XMVectorGetX(vDistance);

		// 객체가 평면의 바깥쪽에 완전히 위치한 경우 
		if (fDistance < -fRadius)
		{
			return false; // 프러스텀 밖에 있음
		}
	}
	return true; // 프러스텀 안에 있거나 평면과 교차함

}


// [D3D12  추가] 셰이더 상수 변수 생성
//
// 1. sizeof(VS_CB_CAMERA_INFO) = 128 bytes
//  -> 256바이트 정렬 필수
// 2. Upload Heap에 생성 -> CPU에서 매 프레임 쓰기 가능
// 3. Map()으로 영구 매핑 - Unmap은 소멸 시 1회만

void CCamera::CreateShaderVariables(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList)
{
	// 256바이트 단위로 올림
	UINT ncbElementSize = ((sizeof(VS_CB_CAMERA_INFO) + 255) & ~255);

	// Upload Heap에 CBV 리소스 생성 (CPU에서 직접 쓰기 가능)
	m_pd3dcbCamera = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
		NULL, ncbElementSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, NULL);

	m_pd3dcbCamera->Map(0, NULL, (void**)&m_pcbMappedCamera);
}

// ═════════════════════════════════════════════════════════
// [D3D12 추가] View + Projection 행렬을 b1 슬롯에 업로드
//
// HLSL은 column-major이므로 CPU에서 전치(Transpose) 필요.
// ═════════════════════════════════════════════════════════

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pcbMappedCamera)
	{
		// 영구 매핑된 CBV 메모리에 직접 기록 (Map/Unmap 불필요)
		XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4View
			, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
		XMStoreFloat4x4(&m_pcbMappedCamera->m_xmf4x4Projection
			, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Project)));
	}

	// b1 슬롯에 CBV 뷰포트 정보 바인딩
	if (m_pd3dcbCamera)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dCameraGPUAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(
		1,							// b1 슬롯
		d3dCameraGPUAddress);
	}
}

// ═════════════════════════════════════════════════════════
// [D3D12 추가] 셰이더 변수 정리
// 32BitConstants 방식이라 정리할 자원 없음.
// ═════════════════════════════════════════════════════════
void CCamera::ReleaseShaderVariables()
{
	if (m_pd3dcbCamera)
	{
		m_pd3dcbCamera->Unmap(0, NULL);
		m_pd3dcbCamera->Release();
		m_pd3dcbCamera = NULL;
	}
	m_pd3dcbCamera = NULL;
}


// ═════════════════════════════════════════════════════════
// [D3D12 추가] RS 단계 뷰포트/가위 사각형 바인딩
// ═════════════════════════════════════════════════════════
void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList)
{
	pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}