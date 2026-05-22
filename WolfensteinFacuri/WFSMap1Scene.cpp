#include "stdafx.h"
#include "Wall.h"
#include "Aircraft.h"
#include "WFSMap1Scene.h"


WFSMap1Scene::WFSMap1Scene(CColliderManager* pCollider, CCamera* pCamera)
	: CScene(pCollider, pCamera)
{
}

WFSMap1Scene::~WFSMap1Scene()
{
	if (m_pStairMesh) m_pStairMesh->Release();
}


void WFSMap1Scene::Animate(float fElapsedTime)
{
	CScene::Animate(fElapsedTime);
}

void WFSMap1Scene::ProcessInput(const InputState& InputState, float fElapsedTime)
{
	const UCHAR* pKeyBuffer = InputState.keys;

	if (pKeyBuffer[VK_LEFT] & 0x80)
	{
		m_pAIrcraft->Rotate(0, 0, 0.01f); // 롤 회전 (좌회전)
		m_pAIrcraft->Rotate(0, -0.01f, 0); // 카메라 요 회전 (좌회전)
		m_pCamera->Rotate(0, -1.f, 0); // 카메라 요 회전 (좌회전)
		//m_pAIrcraft->Move(DIR_LEFT, 20); // 좌회전

	}
	if (pKeyBuffer[VK_RIGHT] & 0x80)
	{
		m_pAIrcraft->Rotate(0, 0, -0.01f); // 롤 회전 (우회전)
		m_pAIrcraft->Rotate(0, 0.01f, 0); // 카메라 요 회전 (우회전)
		m_pCamera->Rotate(0, 1.f, 0); // 카메라 요 회전 (우회전)
		//m_pAIrcraft->Move(DIR_RIGHT, 20); // 우회전

	}
	if (pKeyBuffer[VK_UP] & 0x80)
	{
		m_pAIrcraft->Move(DIR_FORWARD, 50); // 이동 속도
	}
	if (pKeyBuffer[VK_DOWN] & 0x80)
	{
		m_pAIrcraft->Move(DIR_BACKWARD, 50); // 이동 속도
	}
	if (pKeyBuffer[VK_CONTROL] & 0x80)
	{

		m_pAIrcraft->m_fLastFireTime += TIMER->GetTimeElapsed(); // 매 프레임마다 경과 시간을 누적
		if (m_pAIrcraft->m_fLastFireTime >= m_pAIrcraft->m_fFireCooldown) // 쿨다운 시간이 지났는지 체크
		{
			m_pAIrcraft->m_fLastFireTime = 0.0f; // 발사 후 마지막 발사 시간 초기화
			m_pAIrcraft->FireMissile(); // 미사일 발사

		}
	}

}

void WFSMap1Scene::UpdateCamera(float fElapsedTime)
{
	CScene::UpdateCamera(fElapsedTime);
}



void WFSMap1Scene::BuildSceneObjects()
{
	m_pStairMesh = new CStairMesh(m_pd3dDevice, m_pd3dCommandList, 10.0f, 5.0f, 10.0f);

	m_pStairMesh->AddRef(); // 참조 카운트 증가

	m_pAIrcraft = new CAircraft();

	m_pAIrcraft->SetCamera(m_pCamera);
	m_pAIrcraft->SetMesh(m_pPlayerMesh);
	m_pAIrcraft->SetShader(m_pShader);
	m_pAIrcraft->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

	m_pAIrcraft->SetPosition(0.0f, 0.0f, -100.0f);

	m_pAIrcraft->SetObjectType(OBJ_PLAYER);
	// 플레이어 색상 설정
	m_pAIrcraft->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	m_pAIrcraft->GenerateBoundingBox();

	m_pAIrcraft->SetActive(true);
	m_pPlayer = m_pAIrcraft; // m_pPlayer 포인터를 m_pAIrcraft로 설정



	if (!LoadMapFile(L"\\mapdata\\level1.txt"))
	{
		::OutputDebugStringA("[WFSMap1] level1.txt 로드 실패 - 빈 맵\n");
		return;
	}
	const UINT nMapCapacity = m_nMapWidth * m_nMapHeight;
	m_wallInstanceBuffer.Create(m_pd3dDevice, nMapCapacity);
	m_floorInstanceBuffer.Create(m_pd3dDevice, nMapCapacity);

	SpawnFromGrid();

	m_nWallInstances = m_wallInstanceBuffer.GetCount();
	m_nFloorInstances = m_floorInstanceBuffer.GetCount();
}


bool WFSMap1Scene::LoadMapFile(const wchar_t* relPath)
{
	WCHAR szFullPath[MAX_PATH];
	::GetModuleFileNameW(NULL, szFullPath, MAX_PATH);
	WCHAR* slash = wcsrchr(szFullPath, L'\\');
	if (slash) *slash = L'\0'; // 실행 파일 경로에서 마지막 '\' 제거
	wcscat_s(szFullPath, MAX_PATH, relPath);


	std::ifstream ifs{ szFullPath };
	if (!ifs.is_open()) return false;

	std::string firstLine;
	if (!std::getline(ifs, firstLine)) return false;
	std::istringstream iss(firstLine);
	iss >> m_nMapWidth >> m_nMapHeight;
	if (m_nMapWidth <= 0 || m_nMapHeight <= 0) return false;

	m_vGridData.clear();
	m_vGridData.reserve(m_nMapHeight);
	for (int i = 0; i < m_nMapHeight; ++i)
	{
		std::string row;
		if (!std::getline(ifs, row)) row.clear();

		if ((int)row.size() < m_nMapWidth)
			row.append(m_nMapWidth - row.size(), ' '); 
		else if ((int)row.size() > m_nMapWidth)
			row.resize(m_nMapWidth);

		m_vGridData.push_back(std::move(row));
	}


	return true;
}

void WFSMap1Scene::SpawnFromGrid()
{
	const float worldOffsetX = -m_nMapWidth * kTileSize * 0.5f;
	const float worldOffsetZ = -m_nMapHeight * kTileSize * 0.5f;
	
	for (int z = 0; z < m_nMapHeight; ++z) {
		for (int x = 0; x < m_nMapWidth; ++x) {
			float wx = worldOffsetX + x * kTileSize;
			float wz = worldOffsetZ + z * kTileSize;
			char c = m_vGridData[z][x];

			switch (c) {
			case '#': SpawnWall(wx, wz); break;
			case '.': SpawnFloor(wx, wz); break;
			case 'P': SpawnFloor(wx, wz); PlacePlayer(wx, wz); break;
			case 'S': SpawnFloor(wx, wz); SpawnStair(wx, wz); break;
			case 'E': SpawnFloor(wx, wz); break; // 적 스폰은 나중에 구현
			default: break;  // 공백 등은 빈 공간
			}
		}
	}

}

void WFSMap1Scene::SpawnWall(float wx, float wz)
{
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranslation(wx, 0.0f, wz));
	m_wallInstanceBuffer.AddInstance(world, XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));

	CWall* pWall = new CWall();
	pWall->SetMesh(m_pWallMesh);
	pWall->SetPosition(wx, 0.0f, wz);
	pWall->SetObjectType(OBJ_WALL);
	pWall->SetInstancedOnly(true); // 인스턴싱 전용으로 설정
	pWall->SetColor(XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f));
	pWall->GenerateBoundingBox();
	AddObject(pWall);

}

void WFSMap1Scene::SpawnFloor(float wx, float wz)
{
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranslation(wx, -0.5f, wz));
	m_floorInstanceBuffer.AddInstance(world, XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
}

void WFSMap1Scene::SpawnStair(float wx, float wz)
{
	CGameObject* pStair = new CGameObject();
	pStair->SetMesh(m_pStairMesh);
	pStair->SetShader(m_pShader);
	pStair->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pStair->SetPosition(wx, -0.5f, wz);
	pStair->SetObjectType(OBJ_FLOOR);
	pStair->SetColor(XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
	pStair->GenerateBoundingBox();
	AddObject(pStair);
}

void WFSMap1Scene::PlacePlayer(float wx, float wz)
{
	if (!m_pAIrcraft) return;
	m_pAIrcraft->SetPosition(wx, 0.0f, wz);
}