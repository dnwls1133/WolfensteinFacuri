#include "stdafx.h"
#include "Wall.h"
#include "Enemy.h"
#include "JointPlayer.h"
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
	if(m_nEnemyCount <= 0)
	{
		// 모든 적이 제거되었을 때 승리 처리
		if (m_pPlayer) m_pPlayer->SetWon();
	}

	if (m_pJointPlayer) {
		XMFLOAT3 pos = m_pJointPlayer->GetPosition();
		m_pJointPlayer->SetFloorY(GetFloorY(pos.x, pos.z));
	}

	m_fLightTime += fElapsedTime;

	float a = m_fLightTime * 0.1f;

	m_pcbMappedLight->m_xmf3Direction = XMFLOAT3(sinf(a), -cosf(a), 0.5f);

	float t = ((cosf(a) + 1.0f) * 0.5f);

	m_pcbMappedLight->m_xmf3Color = XMFLOAT3(
		1.0f,
		0.6f + 0.4f * t,
		0.3f + 0.7f * t
	);


	CScene::Animate(fElapsedTime);

}

void WFSMap1Scene::ProcessInput(const InputState& InputState, float fElapsedTime)
{
	if (!m_pJointPlayer) return;
	const UCHAR* pKeyBuffer = InputState.keys;

	if (pKeyBuffer[VK_LEFT] & 0x80)
	{
		m_pJointPlayer->Rotate(0, -0.05f, 0);
		//m_pCamera->Rotate(0, -1.f, 0);
		m_pJointPlayer->m_bIsMoving = true; // 걷기 애니메이션 활성화

	}
	if (pKeyBuffer[VK_RIGHT] & 0x80)
	{
		m_pJointPlayer->Rotate(0, 0.05f, 0);
		//m_pCamera->Rotate(0, 1.f, 0);
		m_pJointPlayer->m_bIsMoving = true; // 걷기 애니메이션 활성화
	}

	// 이동 + 벽 충돌 슬라이딩
	const float kSpeed = 25.0f;
	const float kRadius = 2.0f;

	bool bMoving = false;

	if (pKeyBuffer[VK_UP] & 0x80 || pKeyBuffer[VK_DOWN] & 0x80)
	{
		// 카메라 룩 벡터를 수평 방향으로만 사용
		XMFLOAT3 look = m_pCamera->GetLookVector();
		look.y = 0;
		float len = sqrtf(look.x * look.x + look.z * look.z);
		if (len > 0.001f) { look.x /= len; look.z /= len; }

		float dir = (pKeyBuffer[VK_UP] & 0x80) ? 1.0f : -1.0f;
		float dx = look.x * kSpeed * dir * fElapsedTime;
		float dz = look.z * kSpeed * dir * fElapsedTime;

		XMFLOAT3 pos = m_pJointPlayer->GetPosition();
		float currFloor = GetFloorY(pos.x, pos.z);

		float newX = pos.x + dx;
		float edgeX = pos.x + dx + (dx >= 0 ? kRadius : -kRadius);
		bool wallX = IsWall(edgeX, pos.z - kRadius) ||
			          IsWall(edgeX, pos.z) ||
					  IsWall(edgeX, pos.z + kRadius);

		
		if (!wallX) {
			float nextFloor = GetFloorY(newX, pos.z);
			if (nextFloor - currFloor <= kMaxStepUp) {
				pos.x = newX;
				if (nextFloor > currFloor) {
					pos.y = nextFloor;
					m_pJointPlayer->SetVelocity(0.0f);
				}
			}
		}

		float newZ = pos.z + dz;
		float edgeZ = pos.z + dz + (dz >= 0 ? kRadius : -kRadius);
		bool wallZ = IsWall(pos.x - kRadius, edgeZ) ||
				 IsWall(pos.x, edgeZ) ||
			IsWall(pos.x + kRadius, edgeZ);
		if (!wallZ) {
			float nextFloor = GetFloorY(pos.x, newZ);
			if (nextFloor - currFloor <= kMaxStepUp) {
				pos.z = newZ;
				if (nextFloor > currFloor) {
					pos.y = nextFloor;
					m_pJointPlayer->SetVelocity(0.0f);
				}
			}
		}
			

		m_pJointPlayer->SetPosition(pos.x, pos.y, pos.z);
		bMoving = true;

	}

	if (pKeyBuffer[VK_SPACE] & 0x80)
	{
		m_pJointPlayer->Jump();
	}

	if (pKeyBuffer[VK_CONTROL] & 0x80)
	{
		m_pJointPlayer->m_fLastFireTime += TIMER->GetTimeElapsed(); // 매 프레임마다 경과 시간을 누적
		if (m_pJointPlayer->m_fLastFireTime >= m_pJointPlayer->m_fFireCooldown) // 쿨다운 시간이 지났는지 체크
		{
			m_pJointPlayer->m_fLastFireTime = 0.0f; // 발사 후 마지막 발사 시간 초기화
			m_pJointPlayer->FireMissile(); // 미사일 발사
		}
		
	}

	if (pKeyBuffer[VK_F2] & 0x80)
	{
		m_bThirdPersonView = true;
	}
	if (pKeyBuffer[VK_F3] & 0x80)
	{
		m_bThirdPersonView = false;
	}

	m_pJointPlayer->m_bIsMoving = bMoving; // 걷기 애니메이션 활성화 여부 설정
	//if (!(pKeyBuffer[VK_LEFT] & 0x80) && !(pKeyBuffer[VK_RIGHT] & 0x80) &&
	//	!(pKeyBuffer[VK_UP] & 0x80) && !(pKeyBuffer[VK_DOWN] & 0x80))
	//{
	//	m_pJointPlayer->m_bIsMoving = false; // 걷기 애니메이션 비활성화
	//}

}

bool WFSMap1Scene::IsWall(float wx, float wz) const
{

	float offsetX = -m_nMapWidth * kTileSize * 0.5f;
	float offsetZ = -m_nMapHeight * kTileSize * 0.5f;

	// int()cast는 버림을 하므로 올림 내림 처리를 위해 floorf() + 0.5f를 사용하여 가장 가까운 정수로 반올림한다.
	int col = (int)floorf((wx - offsetX) / kTileSize + 0.5f);
	int row = (int)floorf((wz - offsetZ) / kTileSize + 0.5f);

	if(col < 0 || col >= m_nMapWidth || row < 0 || row >= m_nMapHeight)
		return true;

	return m_vGridData[row][col] == '#';
}

float WFSMap1Scene::GetFloorY(float wx, float wz) const
{
	float offsetX = -m_nMapWidth * kTileSize * 0.5f;
	float offsetZ = -m_nMapHeight * kTileSize * 0.5f;

	int col = (int)floorf((wx - offsetX) / kTileSize + 0.5f);
	int row = (int)floorf((wz - offsetZ) / kTileSize + 0.5f);

	if (col < 0 || col >= m_nMapWidth || row < 0 || row >= m_nMapHeight)
		return 0.0f;

	char c = m_vGridData[row][col];
	if (c >= '1' && c <= '9')
		return (c - '0') * kStepHeight;

	return 0.0f;
}

void WFSMap1Scene::UpdateCamera(float fElapsedTime)
{
	CScene::UpdateCamera(fElapsedTime);
}



void WFSMap1Scene::BuildSceneObjects()
{
	m_pStairMesh = new CStairMesh(m_pd3dDevice, m_pd3dCommandList, 10.0f, 5.0f, 10.0f);

	m_pStairMesh->AddRef(); // 참조 카운트 증가

	m_pJointPlayer = new CJointPlayer();
	m_pJointPlayer->SetCamera(m_pCamera);
	m_pJointPlayer->BuildParts(m_pd3dDevice, m_pd3dCommandList, m_pShader);

	m_pJointPlayer->SetMesh(m_pPlayerMesh);
	m_pJointPlayer->SetPosition(0.0f, 0.0f, -100.0f);
	m_pJointPlayer->SetObjectType(OBJ_PLAYER);
	m_pJointPlayer->GenerateBoundingBox();
	m_pJointPlayer->SetActive(true);
	m_pPlayer = m_pJointPlayer; // m_pPlayer 포인터를 m_pAIrcraft로 설정



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
			case 'E': SpawnFloor(wx, wz); SpawnEnemy(wx, wz); break; // 적 스폰은 나중에 구현
			default:
				if (c >= '1' && c <= '9') {
					float h = (c - '0') * kStepHeight;
					SpawnFloor(wx, wz, h);
				}
				break;
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

void WFSMap1Scene::SpawnFloor(float wx, float wz, float fHeight)
{
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, XMMatrixTranslation(wx, fHeight -0.5f, wz));
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

void WFSMap1Scene::SpawnEnemy(float wx, float wz)
{
	CEnemy* pEnemy = new CEnemy();
	pEnemy->SetMesh(m_pEnemyMesh);
	pEnemy->SetShader(m_pShader);
	pEnemy->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pEnemy->SetPosition(wx, 0.0f, wz);
	pEnemy->SetObjectType(OBJ_ENEMY);
	pEnemy->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	pEnemy->GenerateBoundingBox();
	pEnemy->SetSpeed(1.0f); // 적의 이동 속도 설정

	float offsetX = -m_nMapWidth * kTileSize * 0.5f;
	float offsetZ = -m_nMapHeight * kTileSize * 0.5f;
	pEnemy->SetGridMap(&m_vGridData, m_nMapWidth, m_nMapHeight, kTileSize, offsetX, offsetZ);
	AddObject(pEnemy);

	++m_nEnemyCount;
}

void WFSMap1Scene::PlacePlayer(float wx, float wz)
{
	if (!m_pJointPlayer) return;
	m_pJointPlayer->SetPosition(wx, 0.0f, wz);
}