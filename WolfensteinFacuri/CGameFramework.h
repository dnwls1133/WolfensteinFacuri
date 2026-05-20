#pragma once
#include "InputState.h"


// - GDI 더블 버퍼링(HDC/HBITMAP) -> D3D12 스왑체인 + 펜스 동기화로 교체
// - 게임 로직 관련 멤버(씬, 플레이어, 승리/패배 타이머)는 그대로 유지


class CScene;
class CPlayer;
class CCamera;
class CColliderManager;

class GameFramework
{
private:
	HINSTANCE			m_hInstance = NULL;
	HWND				m_hWnd = NULL;
	int					m_nClientWidth = FRAMEBUFFER_WIDTH;
	int					m_nClientHeight = FRAMEBUFFER_HEIGHT;





	// [D3D12 추가] DXGI / 디바이스 / 스왑체인
	IDXGIFactory4*		m_pDXGIFactory = NULL;
	IDXGISwapChain3*	m_pdxgiSwapChain = NULL;
	ID3D12Device*		m_pd3dDevice = NULL;

	// MSAA 다중 샘플링 설정 (비할성화)
	bool 				m_bMsaa4xEnable = false;
	UINT 				m_nMsaa4xQualityLevels = 0;

	// [D3D12 추가] 스왑체인 버퍼들
	static const UINT m_nSwapChainBuffers = 2; // 더블 버퍼링
	UINT			  m_nSwapChainBufferIndex = 0; // 현재 백 버퍼 인덱스
	ID3D12Resource*   m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers]; // 백 버퍼 리소스 배열

	// [D3D12 추가] RTV/DSV 힙
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
	UINT				  m_nRtvDescriptorIncrementSize = 0;

	ID3D12Resource*		  m_pd3dDepthStencilBuffer = NULL;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
	UINT				  m_nDsvDescriptorIncrementSize = 0;

	// [D3D12 추가] 명령 큐 / 명령 할당자 / 명령 리스트
	ID3D12CommandQueue*			m_pd3dCommandQueue = NULL;
	ID3D12CommandAllocator*		m_pd3dCommandAllocator = NULL;
	ID3D12GraphicsCommandList*	m_pd3dCommandList = NULL;

	// [D3D12 추가] CPU/GPU 동기화에 필요한 펜스
	ID3D12Fence*	m_pd3dFence = NULL;
	UINT64			m_nFenceValues[m_nSwapChainBuffers] = { 0,0 };
	HANDLE 			m_hFenceEvent = NULL;

	// 메니저
	CColliderManager* m_pColliderManager{ NULL };

public:

	// 게임에 필요한 객체들
	CScene* m_pScene{ NULL };
	//CPlayer* m_pPlayer{ NULL };
	CCamera* m_pCamera{ NULL };

private:
	// 입력 제어
	POINT m_ptCenterCursorPos;
	POINT m_ptPrevCursorPos{ 0,0 };
	bool m_bIsMousedLocked = false;

	InputState m_inputState;

	_TCHAR						m_pszFrameRate[50];
	float endTimer = 0.0f;

public:
	GameFramework();
	~GameFramework();

	void OnCreate(HINSTANCE hInstance, HWND hMainWnd); // 초기화
	void OnDestroy();                                  // 종료 시 메모리 해제

	// [D3D12 추가] 초기화 함수
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	void CreateSwapChain();
	void CreateRenderTargetViews();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateDepthStencilView();

	// 오브젝트 빌드 및 해제
	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();                             // 사용자 입력 처리
	void FrameAdvance();                               // 매 프레임마다 실행될 게임루프
	
	// [D3D12 추가] GPU 동기화
	void WaitForGpuComplete();
	void MoveToNextFrame();


	// 메니저 관련 함수
	CColliderManager& GetColliderManager() { return *m_pColliderManager; }


	// 윈도우 메시지 처리기 (마우스 클릭으로 화면 캡처)
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

