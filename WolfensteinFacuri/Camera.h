#pragma once

// - View+Projection 행렬을 Upload Heap CBV로 관리 (영구 매핑)
// - Root Signature의 b1 슬롯에 Root CBV로 바인딩
// - 기존 카메라 좌표계, 절두체 컬링, 추적 카메라 로직 100% 보존
//

// [D3D12 추가] 카메라 상수 버퍼 구조체
// Shaders.hlsl의 cbCamerInfo와 정확히 일치하는 메모리 레이아웃

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4 m_xmf4x4View;		// gmtxView
	XMFLOAT4X4 m_xmf4x4Projection;  // gmtxProjection
};

class CViewport
{
public:
	int m_nLeft{ 0 };
	int m_nTop{ 0 };
	int m_nWidth{ 0 };
	int m_nHeight{ 0 };

	void SetViewport(int nLeft, int nTop, int nWidth, int nHeight)
	{
		m_nLeft = nLeft;
		m_nTop = nTop;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}
};

class CCamera
{
public:
	CCamera();
	virtual ~CCamera();

private:
	// 카메라의 로컬 축
	XMFLOAT3 m_xmf3Position; 
	XMFLOAT3 m_xmf3Right;    
	XMFLOAT3 m_xmf3Up;       
	XMFLOAT3 m_xmf3Look;     

	// 변환 행렬들
	XMFLOAT4X4 m_xmf4x4View;
	XMFLOAT4X4 m_xmf4x4Project;
	XMFLOAT4X4 m_xmf4x4ViewProject; // 뷰 행렬과 투영 행렬의 곱

	CViewport m_Viewport;

	// 카메라의 시야를 구성하는 6개의 평면
	XMFLOAT4 m_xmf4FrustumPlanes[6]; // 왼쪽, 오른쪽, 위, 아래, 앞, 뒤 평면

	// [D3D12 추가] 뷰포트와 시저 렉트
	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT	   m_d3dScissorRect;

	// [D3D12 추가] Root CBV용 Upload Heap 버퍼
	// - m_pd3dCamera       : GPU 리소스 (Upload Heap)
	// - m_pCameraInfo      : Map()으로 얻은 CPU 측 영구 매핑 포인터
	//                        매 프레임 memcpy로 갱신만 하면 됨

	ID3D12Resource*		m_pd3dcbCamera = NULL;
	VS_CB_CAMERA_INFO*	m_pcbMappedCamera = NULL;

public:
	// 행렬 생성
	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float fNearPlane,float fFarPlane, float fFOVAngle);

	// 변경 D3D12: 뷰포트와 시저 렉트 설정 함수 추가
	void SetViewport(int nLeft, int nTop, int nWidth, int nHeight);


	void SetPosition(float x, float y, float z) { m_xmf3Position = XMFLOAT3(x, y, z); }
	void SetPosition(XMFLOAT3& xmf3Position)	{ m_xmf3Position = xmf3Position; }
	CViewport* GetViewport()					{ return &m_Viewport; }
	XMFLOAT3 GetLookVector() const				{ return m_xmf3Look; }
	XMFLOAT3 GetRightVector() const				{ return m_xmf3Right; }
	XMFLOAT4X4* GetViewMatrix()					{ return &m_xmf4x4View; }
	XMFLOAT4X4* GetViewProjectMatrix()			{ return &m_xmf4x4ViewProject; }
	XMFLOAT3 GetPosition() const				{ return m_xmf3Position; }

	// 절두체 컬링
	void CalulateFrustumPlanes();
	bool IsInFrustum(XMFLOAT3& xmf3Center, float fRadius);


	// 카메라 이동 및 회전
	void Move(XMFLOAT3& xmf3Shift);
	void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll=0.0f);
	void Update();


	// [D3D12  추가] 셰이더 상수 변수 관리
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice
		, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	// [D3D12 추가] RS 단계에 뷰포트 / 시저 렉트 바인딩
	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList);
};

