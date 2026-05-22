#include "stdafx.h"
#include "Enemy.h"
#include "Wall.h"
#include "GalagaScene.h"

GalagaScene::GalagaScene(CColliderManager* pCollider, CCamera* pCamera)
	: CScene(pCollider, pCamera)
{

}

void GalagaScene::Animate(float fElapsedTime)
{
	XMFLOAT3 floorPosition = m_vpFloorObjects[450]->GetPosition();

	XMFLOAT3 playerPosition = m_pPlayer->GetPosition();

	float Fdistance = Vector3::Distance(floorPosition, playerPosition);
	float Wdistance = Vector3::Distance(m_xmf3Wallposition, playerPosition);
	float Edistance = Vector3::Distance(m_xmf3Enemyspawnposition, playerPosition);

	if (Fdistance > 50.0f)
	{
		int nMapWidth = 30; // 맵의 가로 크기 (바닥 조각 수)
		int nMapDepth = 50; // 맵의 세로 크기 (바닥 조각 수)
		float fChunkSize = 10.0f; // 각 바닥 조각의 크기

		// 플레이어의 위치를 가장 가까운 바닥 조각의 중심으로 스냅
		float snappedX = round(playerPosition.x / fChunkSize) * fChunkSize;
		float snappedZ = round(playerPosition.z / fChunkSize) * fChunkSize;

		for (int i = 0; i < m_vpFloorObjects.size(); i++)
		{
			int gridX = i % nMapWidth; // X 좌표 계산
			int gridZ = i / nMapWidth; // Z 좌표 계산

			float newX = snappedX + (gridX - nMapWidth / 2.0f) * fChunkSize; // 새로운 X 좌표 계산
			float newZ = snappedZ + (gridZ - nMapDepth / 2.0f) * fChunkSize; // 새로운 Z 좌표 계산

			float currentY = m_vpFloorObjects[i]->GetPosition().y; // 현재 Y 좌표 유지

			m_vpFloorObjects[i]->SetPosition(newX, currentY, newZ); // 바닥 조각의 위치 업데이트

		}

	}


	if (Wdistance > 300.0f)
	{
		playerPosition.z += 300.0f;

		m_xmf3Wallposition = playerPosition;


		const int nWallCount = static_cast<int>(m_vpWallObjects.size());
		const int nWallsPerSide = nWallCount / 2;
		const float wallHalfOffsetX = 50.0f;  // 양옆 거리
		const float wallSpacingZ = 12.0f;      // 벽 간격(Z축)

		float startZ = m_xmf3Wallposition.z - ((nWallsPerSide - 1) * wallSpacingZ * 0.5f);

		for (int i = 0; i < nWallCount; ++i)
		{
			bool bLeftSide = (i < nWallsPerSide);
			int indexOnSide = bLeftSide ? i : (i - nWallsPerSide);

			float fPosX = m_xmf3Wallposition.x + (bLeftSide ? -wallHalfOffsetX : wallHalfOffsetX);
			float fPosZ = startZ + (indexOnSide * wallSpacingZ);

			m_vpWallObjects[i]->SetPosition(fPosX, -3.0f, fPosZ); // Y 좌표는 0으로 설정하여 바닥 위에 위치하도록 함
		}
	}

	if (Edistance > 200.0f)
	{
		playerPosition.z += 50.0f;
		m_xmf3Enemyspawnposition = playerPosition;

		int nEnemyCount = std::count_if(m_vpObjects.begin(), m_vpObjects.end(),
			[](CGameObject* pObj) { return pObj->GetObjectType() == OBJ_ENEMY; });

		int enemyIndex = 0;
		for (int i = 0; i < (int)m_vpObjects.size(); i++)
		{
			if (m_vpObjects[i]->GetObjectType() == OBJ_ENEMY)
			{
				float fPosX = m_xmf3Enemyspawnposition.x + (rand() / (float)RAND_MAX - 0.5f) * 100.0f;
				float fPosY = m_xmf3Enemyspawnposition.y + (rand() / (float)RAND_MAX - 0.5f) * 10.0f;
				float fPosZ = m_xmf3Enemyspawnposition.z + enemyIndex * 20.0f;
				m_vpObjects[i]->SetPosition(fPosX, fPosY, fPosZ);
				++enemyIndex;
			}
		}

		if (nEnemyCount < 20)
		{
			for (int i = 0; i < 20 - nEnemyCount; i++)
			{
				CEnemy* pEnemy = new CEnemy();
				pEnemy->SetMesh(m_pEnemyMesh);
				float fPosX = m_xmf3Enemyspawnposition.x + (rand() / (float)RAND_MAX - 0.5f) * 100.0f;
				float fPosY = m_xmf3Enemyspawnposition.y + (rand() / (float)RAND_MAX - 0.5f) * 10.0f;
				float fPosZ = m_xmf3Enemyspawnposition.z + i * 20.0f;
				pEnemy->SetPosition(fPosX, fPosY, fPosZ);
				pEnemy->GenerateBoundingBox();
				pEnemy->SetColor(RGB(rand() % 150 + 100, rand() % 150 + 100, rand() % 150 + 100));
				pEnemy->SetObjectType(OBJ_ENEMY);
				AddObject(pEnemy);
			}
		}



	}
	CScene::Animate(fElapsedTime);
}

void GalagaScene::ProcessInput(const InputState& InputState, float fElapsedTime)
{
	const UCHAR* pKeyBuffer = InputState.keys;
	
	if (pKeyBuffer[VK_LEFT] & 0x80)
	{
		m_pPlayer->Rotate(0, 0, 0.01f); // 롤 회전 (좌회전)
		m_pPlayer->Move(DIR_LEFT, 20); // 좌회전

	}
	if (pKeyBuffer[VK_RIGHT] & 0x80)
	{
		m_pPlayer->Rotate(0, 0, -0.01f); // 롤 회전 (우회전)
		m_pPlayer->Move(DIR_RIGHT, 20); // 우회전

	}
	if (pKeyBuffer[VK_UP] & 0x80)
	{
		m_pPlayer->Move(DIR_FORWARD, 50); // 이동 속도
	}
	if (pKeyBuffer[VK_DOWN] & 0x80)
	{
		m_pPlayer->Move(DIR_BACKWARD, 50); // 이동 속도
	}
	if (pKeyBuffer[VK_CONTROL] & 0x80)
	{

		m_pPlayer->m_fLastFireTime += TIMER->GetTimeElapsed(); // 매 프레임마다 경과 시간을 누적
		if (m_pPlayer->m_fLastFireTime >= m_pPlayer->m_fFireCooldown) // 쿨다운 시간이 지났는지 체크
		{
			m_pPlayer->m_fLastFireTime = 0.0f; // 발사 후 마지막 발사 시간 초기화
			m_pPlayer->FireMissile(); // 미사일 발사

		}
	}
	
}

void GalagaScene::BuildSceneObjects()
{
	m_pPlayer = new CPlayer();
	m_pPlayer->SetCamera(m_pCamera);
	m_pPlayer->SetMesh(m_pPlayerMesh);
	m_pPlayer->SetShader(m_pShader);
	m_pPlayer->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	
	m_pPlayer->SetPosition(0.0f, 0.0f, -100.0f);

	m_pPlayer->SetObjectType(OBJ_PLAYER);
	// 플레이어 색상 설정
	m_pPlayer->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	m_pPlayer->GenerateBoundingBox();


	// ─────────────────────────────────────────────────────
	// 4. 적 20개 생성 (기존 로직 동일 — Z축 방향으로 일렬 배치)
	// ─────────────────────────────────────────────────────
	m_xmf3Enemyspawnposition = XMFLOAT3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 20; i++)
	{
		CEnemy* pEnemy = new CEnemy();
		pEnemy->SetMesh(m_pEnemyMesh);
		pEnemy->SetShader(m_pShader);
		pEnemy->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

		float X = m_xmf3Enemyspawnposition.x + (rand() / (float)RAND_MAX - 0.5f) * 100.0f;
		float Y = m_xmf3Enemyspawnposition.y + (rand() / (float)RAND_MAX - 0.5f) * 10.0f;
		float Z = m_xmf3Enemyspawnposition.z + i * 20.0f;
		pEnemy->SetPosition(X, Y, Z);
		pEnemy->GenerateBoundingBox();

		pEnemy->SetColor(RGB(rand() % 150 + 100, rand() % 150 + 100, rand() % 150 + 100));
		pEnemy->SetObjectType(OBJ_ENEMY);

		m_vpObjects.push_back(pEnemy);
		++m_nObjects;
	}

	// ─────────────────────────────────────────────────────
	// 5. 바닥 타일 30x50 = 1500개 (기존 로직 동일)
	// ─────────────────────────────────────────────────────
	int nMapWidth = 30;
	int nMapDepth = 50;

	float fChunkSize = 10.0f; // 타일 간격

	for (int Z = 0; Z < nMapDepth; Z++)
	{
		for (int X = 0; X < nMapWidth; X++)
		{
			CGameObject* pFloor = new CGameObject();
			pFloor->SetMesh(m_pFloorMesh);
			pFloor->SetShader(m_pShader);
			pFloor->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

			float fPosX = (X - nMapWidth / 2.0f) * fChunkSize;
			float fPosZ = (Z - nMapDepth / 2.0f) * fChunkSize;
			pFloor->SetPosition(fPosX, -5.0f, fPosZ);
			pFloor->GenerateBoundingBox();

			pFloor->SetColor(RGB(100, 100, 20));
			pFloor->SetObjectType(OBJ_FLOOR);

			m_vpObjects.push_back(pFloor);
			m_vpFloorObjects.push_back(pFloor);
			++m_nObjects;
		}
	}

	// ─────────────────────────────────────────────────────
	// 6. 벽 500개 좌우 배치 (기존 로직 동일)
	// ─────────────────────────────────────────────────────
	m_xmf3Wallposition = XMFLOAT3(0.0f, -4.0f, 0.0f);

	const int   nWallCount = 500;
	const int   nWallsPerSide = nWallCount / 2;
	const float wallHalfOffsetX = 50.0f;
	const float wallSpacingZ = 12.0f;

	for (int i = 0; i < nWallCount; ++i)
	{
		CWall* pWall = new CWall();
		pWall->SetMesh(m_pWallMesh);
		pWall->SetShader(m_pShader);
		pWall->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

		bool bLeftSide = (i < nWallsPerSide);
		int  indexOnSide = bLeftSide ? i : (i - nWallsPerSide);

		float startZ = m_xmf3Wallposition.z - ((nWallsPerSide - 1) * wallSpacingZ * 0.5f);
		float fPosX = m_xmf3Wallposition.x + (bLeftSide ? -wallHalfOffsetX : wallHalfOffsetX);
		float fPosZ = startZ + (indexOnSide * wallSpacingZ);

		pWall->SetPosition(fPosX, -4.0f, fPosZ);
		pWall->SetColor(RGB(rand() % 50 + 100, rand() % 50 + 100, rand() % 50 + 100));
		pWall->GenerateBoundingBox();
		pWall->SetObjectType(OBJ_WALL);

		m_vpObjects.push_back(pWall);
		m_vpWallObjects.push_back(pWall);
		++m_nObjects;
	}



}