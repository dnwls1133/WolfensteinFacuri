
// 초기화: GDI 백버퍼 -> DXGI 스왑체인 + RTV/DSV 힙 
// 매 프래임 : GDI BitBlt -> CommandList 기록 -> ExecuteCommandList -> Present + 펜스 동기화

#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include "ColliderManager.h"
#include "GalagaScene.h"
#include "TitleScene.h"
#include "CGameFramework.h"


GameFramework::GameFramework()
{
	// [D3D12 추가] 스왑체인 버퍼 / 팬스 배열 초기화
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
	{
		m_ppd3dSwapChainBackBuffers[i] = nullptr;
		m_nFenceValues[i] = 0;
	}

	_tcscpy_s(m_pszFrameRate, _T("WolfensteinFacuri"));
}

GameFramework::~GameFramework()
{
}

void GameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{

	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	

	// 클라이언트 영역 크기 갱신
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nClientWidth = rcClient.right - rcClient.left;
	m_nClientHeight = rcClient.bottom - rcClient.top;

	// [D3D12 추가] Direct3D 디바이스 / 명령큐 / 스왑체인 / 디스크립터 힙 / DSV 

	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();
	CreateDepthStencilView();
	
	// 메니저 초기화
	m_pColliderManager = new CColliderManager();
	m_pColliderManager->InitCollisionMatrix();


	BuildObjects(); // 추가: 플레이어와 씬 초

	POINT ptCenter = { m_nClientWidth / 2, m_nClientHeight / 2 };
	ClientToScreen(m_hWnd, &ptCenter);
	m_ptCenterCursorPos = ptCenter;

	POINT mousePos{};
	::GetCursorPos(&mousePos);
	::ScreenToClient(m_hWnd, &mousePos);
	m_ptPrevCursorPos = mousePos;

}

void GameFramework::OnDestroy()
{
	// [D3D12 추가] GPU가 마지막 프레임 완료를 기다린다
	WaitForGpuComplete();

	ReleaseObjects();

	// [D3D12 추가] Direct3D 펜스 이벤트 핸들 해제
	if (m_hFenceEvent) ::CloseHandle(m_hFenceEvent);

	// [D3D12 추가] 스왑체인 백버퍼 / 디스크립터 힙 / 명령 객체 해제
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue)		m_pd3dCommandQueue->Release();
	if (m_pd3dCommandList)		m_pd3dCommandList->Release();
	if (m_pd3dFence)			m_pd3dFence->Release();

	if (m_pdxgiSwapChain) m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL); // 전체 화면 모드에서 창 모드로 전환
	if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
	if (m_pd3dDevice) m_pd3dDevice->Release();
	if (m_pDXGIFactory) m_pDXGIFactory->Release();


	
}


// [D3D12 추가] 디바이스 생성
void GameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;

	// 1. 디버그 레이어 활성화
	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	{
		ID3D12Debug* pd3dDebugController = nullptr;
		hResult = ::D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
		if (pd3dDebugController)
		{
			pd3dDebugController->EnableDebugLayer();
			pd3dDebugController->Release();
		}
		nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

#endif

	// 2. DXGI 팩토리 생성
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pDXGIFactory);

	// 3. 하드웨어 어댑터 검색 후 D3D12 디바이스 생성
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pDXGIFactory->EnumAdapters1(i,
		&pd3dAdapter); ++i)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0,
			__uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
	}

	// 어댑터 검색 실패 시 WARP 디바이스로 대체
	if (!pd3dAdapter)
	{
		m_pDXGIFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter,D3D_FEATURE_LEVEL_11_0,
			__uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}
	if (pd3dAdapter) pd3dAdapter->Release();

	// 4. 펜스 생성
	m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		__uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

// [D3D12 추가] 명령 큐 / 할당자 / 리스트 생성

void GameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, 
		__uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);
	m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, NULL, 
		__uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);

	// 명령 리스트 닫아 주세요

	m_pd3dCommandList->Close();

}

// [D3D12 추가] RTV/DSV 힙 생성

void GameFramework::CreateSwapChain()
{
	// RTV 힙 생성 (백 버퍼 수 만큼)
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// DSV 힙 1개
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

}

// [D3D12 추가] 스왑체인 생성

void GameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	m_pDXGIFactory->CreateSwapChain(m_pd3dCommandQueue,
		&dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiSwapChain);

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();
}

// [D3D12 추가] RTV 생성

void GameFramework::CreateRenderTargetViews()
{
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; ++i)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource),
			(void**)&m_ppd3dSwapChainBackBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i]
			, NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}
}
// [D3D12 추가] DSV 생성
// 페인터 알고리즘 대채 - Z-Buffer
void GameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC d3dResourceDesc;
	::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nClientWidth;
	d3dResourceDesc.Height = m_nClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties,D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		__uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUDescriptorHandle);

}



void GameFramework::BuildObjects()
{
	// 명령리스트를 다시 Open
	m_pd3dCommandAllocator->Reset();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	// 1. 카메라 생성 및 투영 행렬 설정
	m_pCamera = new CCamera();
	m_pCamera->SetViewport(0, 0, m_nClientWidth, m_nClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, 60.0f);
	m_pCamera->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	
	// 2. 플레이어 생성
	

	
	
	// 3. 씬 생성
	SCENE_MANAGER->ChangeScene(new TitleScene(m_pColliderManager, m_pCamera), m_pd3dDevice, m_pd3dCommandList);
	m_pScene = SCENE_MANAGER->GetCurrentScene();

	// [D3D12 추가] 빌드 단계의 GPU 명령들을 큐에 제출하고 완료 대기
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	WaitForGpuComplete();

	// 업로드 버퍼 해제 (Phase 3에서 CMesh::ReleaseUploadBuffers 호출 추가 예정)
   // if (m_pScene) m_pScene->ReleaseUploadBuffers();
}

void GameFramework::ReleaseObjects()
{
	SCENE_MANAGER->RelaseCurrentScene();
	m_pScene = NULL;
	m_pCamera = NULL;
}
void GameFramework::ProcessInput()
{
	if (!m_pScene) return;

	::ZeroMemory(&m_inputState.keys, sizeof(m_inputState.keys));
	::GetKeyboardState(m_inputState.keys);

	POINT mousePos{};
	::GetCursorPos(&mousePos);
	::ScreenToClient(m_hWnd, &mousePos);

	m_inputState.mousePosition = mousePos;
	m_inputState.mouseDelta.x = mousePos.x - m_ptPrevCursorPos.x;
	m_inputState.mouseDelta.y = mousePos.y - m_ptPrevCursorPos.y;
	m_inputState.isMouseLocked = m_bIsMousedLocked;
	
	m_ptPrevCursorPos = mousePos;

	m_pScene->ProcessInput(m_inputState,TIMER->GetTimeElapsed());

	
}

void GameFramework::FrameAdvance()
{
	TIMER->Tick(60.0f); // 프레임 타이머 갱신

	if (SCENE_MANAGER->HasPendingScene()) {
		m_pd3dCommandAllocator->Reset();
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

		CScene* pNextScene = SCENE_MANAGER->TakePendingScene();
		SCENE_MANAGER->ChangeScene(pNextScene, m_pd3dDevice, m_pd3dCommandList);
		m_pScene = SCENE_MANAGER->GetCurrentScene();

		m_pd3dCommandList->Close();
		ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
		m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
		WaitForGpuComplete();

	}


	if (SCENE_MANAGER->GetCurrentScene()->GetPlayer()->IsWon())
	{
		endTimer += TIMER->GetTimeElapsed();

		if (endTimer > 0.5f)
		{
			MessageBox(m_hWnd, _T("Congratulations! You Win!"), _T("3D Shooting Game"), MB_OK | MB_ICONINFORMATION);
			PostQuitMessage(0); // 게임 종료
			return; // 더 이상 프레임 진행할 필요 없음
		}
	}

	if (SCENE_MANAGER->GetCurrentScene()->GetPlayer()->IsDestroyed() && !SCENE_MANAGER->GetCurrentScene()->GetPlayer()->IsWon())
	{
		endTimer += TIMER->GetTimeElapsed();

		if (endTimer > 1.5f)
		{
			MessageBox(m_hWnd, _T("Game Over!"), _T("3D Shooting Game"), MB_OK | MB_ICONEXCLAMATION);
			PostQuitMessage(0); // 게임 종료
			return; // 더 이상 프레임 진행할 필요 없음
		}
		// 플레이어가 파괴되었을 때 게임 오버 처리 (예: 메시지 박스 표시)
		
	}
	// 2. 델타 타임 가져오기
	float fTimeElapsed = TIMER->GetTimeElapsed();

	// 1. 입력 처리
	ProcessInput();

	// 1.5 가비지 컬렉션 (충돌이 끝난 객체들 제거)
	m_pScene->ClearGarbageCollection(); // 씬에서 제거된 객체들을 메모리에서 해제

	// 2. 게임 로직 업데이트 (회전 행렬 갱신 등)
	if (m_pScene) m_pScene->Animate(fTimeElapsed);

	// ═════════════════════════════════════════════════════
   // [D3D12 교체] 렌더링 단계
   // 기존 GDI: ClearFrameBuffer + Scene->Render(hDC) + PresentFrameBuffer
   //   ↓
   // 변경 후: 명령 리스트 Reset → 배리어(Present→RT) → ClearRTV/DSV
   //                          → Scene->Render(commandList) → 배리어(RT→Present)
   //                          → Close → ExecuteCommandLists → Present → Fence
   // ═════════════════════════════════════════════════════

	// 1. 명령 할당자 / 리스트 리셋
	m_pd3dCommandAllocator->Reset();
	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

	// 2. 리소스 배리어 설정 (백 버퍼: Present → Render Target)
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;	
	d3dResourceBarrier.Transition.pResource = 
		m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// 3. RTV/DSV 클리어
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUHandle.ptr += m_nRtvDescriptorIncrementSize * m_nSwapChainBufferIndex;

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUHandle, FALSE, &d3dDsvCPUHandle);

	// 4. 백버퍼/ 깊이버퍼 클리어
	float pfClearColor[4] = { 40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUHandle, pfClearColor, 0, NULL);
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUHandle
		, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// 5. 씬 렌더링
	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

	// 6. 리소스 배리어 설정 (백 버퍼: Render Target → Present)
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	// 7. 명령 리스트 클로즈 + 큐에 제출
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);

	// 8. GPU 동기화 후 프리젠트
	WaitForGpuComplete();
	m_pdxgiSwapChain->Present(0, 0);

	// 9. 다음 백버퍼로 전환
	MoveToNextFrame();

	// 6. 프레임 레이트 계산 및 출력
	unsigned long fps = TIMER->GetFrameRate();
	_stprintf_s(m_pszFrameRate, _T("3Dshootinggame (%lu FPS)"), fps);
	::SetWindowText(m_hWnd, m_pszFrameRate);

	
}

// [D3D12 추가] GPU 동기화 - CPU가 GPU 완료를 기다림

void GameFramework::WaitForGpuComplete()
{
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

// [D3D12 추가] 다음 백버퍼로 전환

void GameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}


// 윈도우 프로시저에서 게임 프레임워크로 메시지를 넘겨받는 함수
LRESULT CALLBACK GameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	
		
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) // ESC 누르면 마우스 고정 풀기
		{
			m_bIsMousedLocked = false;
			ReleaseCapture();
			ShowCursor(TRUE);
		}
		else if (wParam == VK_F1) // [추가] F1: OOBB 와이어프레임 디버그 토글
		{
			CScene::s_bDebugWireframe = !CScene::s_bDebugWireframe;
		}
		break;
	}
	return 0;
}