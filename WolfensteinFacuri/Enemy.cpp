#include "stdafx.h"
#include "Particle.h"
#include "WFSMap1Scene.h"
#include "Enemy.h"


CEnemy::CEnemy()
	: speed(rand() % 100 / 100.0f + 0.5f), rotationSpeed(0.1f), Axis(XMFLOAT3(rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f))
{
   
}

CEnemy::~CEnemy()
{
}

void CEnemy::Update()
{
    if (health <= 0)
    {
        SetDestroyed(true); // 체력이 0 이하가 되면 적을 파괴 상태로 설정합니다.
        CParticle* pExplosion = new CParticle();
        pExplosion->Explode(GetPosition(), 20, RGB(255, 0, 0)); // 폭발 효과 생성 (위치, 입자 수, 색상)
        SCENE_MANAGER->GetCurrentScene()->AddObject(pExplosion); // 폭발 효과를 현재 씬에 추가
		static_cast<WFSMap1Scene*>(SCENE_MANAGER->GetCurrentScene())->m_nEnemyCount--; // 적 카운트 감소
        return; // 더 이상 업데이트할 필요가 없으므로 함수를 종료합니다.
    }
    CGameObject::Update();

    // 그리드 없으면 기존 동작

    if (!m_pGridData) return;

	XMFLOAT3 playerPos = SCENE_MANAGER->GetCurrentScene()->GetPlayer()->GetPosition();
	XMFLOAT3 myPos = GetPosition();

    // 탐지 범위 체크
    XMFLOAT3 diff = Vector3::Subtract(playerPos, myPos);
	float dist = Vector3::Length(diff);
    if (dist > kDetectionRange) return;

    // 경로 갱신
    float dt = TIMER->GetTimeElapsed();
	m_fPathTimer += dt;
    if (m_fPathTimer <= 0.0f || m_vPath.empty())
    {
        FindPath(myPos, playerPos);
        m_fPathTimer = 0.0f;
    }

    // 경로 따라 이동
	if (m_vPath.empty() || m_nPathIndex >= (int)m_vPath.size()) return;

	XMFLOAT3 waypoint = m_vPath[m_nPathIndex];
	waypoint.y = myPos.y;

	XMFLOAT3 toWaypoint = Vector3::Subtract(waypoint, myPos);
    if (Vector3::Length(toWaypoint) < kWaypointRadius)
    {
        ++m_nPathIndex;
        return;
	}

    XMFLOAT3 dir = Vector3::Normalize(toWaypoint);
    float moveSpeed = speed * 5.0f;
	float dx = dir.x * moveSpeed * dt;
	float dz = dir.z * moveSpeed * dt;

    // 축 분리 벽 충돌
    float newX = myPos.x + dx;
	float newZ = myPos.z + dz;

    XMINT2 edgeX = WorldToGrid(newX + (dx >= 0 ? kEnemyRadius : -kEnemyRadius), myPos.z);
    if (!IsWallGrid(edgeX.x, edgeX.y)) myPos.x = newX;

	XMINT2 edgeZ = WorldToGrid(myPos.x, newZ + (dz >= 0 ? kEnemyRadius : -kEnemyRadius));
	if (!IsWallGrid(edgeZ.x, edgeZ.y)) myPos.z = newZ;

	SetPosition(myPos.x, myPos.y, myPos.z);





    Axis.x = 0.0f;
    Axis.z = 0.0f;

	Axis = Vector3::Normalize(Axis); // 회전 축을 정규화하여 일관된 회전을 보장합니다.


    Rotate(&Axis, rotationSpeed); // 적이 회전하도록 설정 (예시)

    if (!isSpawned)
    {
        
        isSpawned = true;
    }

    XMFLOAT3 playerPosition = SCENE_MANAGER->GetCurrentScene()->GetPlayer()->GetPosition(); // 플레이어 위치 가져오기
    XMFLOAT3 enemyPosition = GetPosition(); // 적의 현재 위치를 가져옵니다.
    directionToPlayer = Vector3::Subtract(playerPosition, enemyPosition);
    directionToPlayer = Vector3::Normalize(directionToPlayer);
   
	// 적이 플레이어를 향해 이동하도록 설정
	// 보간을 사용하여 부드러운 이동을 구현할 수 있습니다.
	XMVECTOR vCurrentPos = XMLoadFloat3(&enemyPosition);
	XMVECTOR vTargetPos = XMLoadFloat3(&playerPosition);
	float fTimeElapsed = TIMER->GetTimeElapsed();
	float t = speed * fTimeElapsed; 
	if (t > 1.0f) t = 1.0f; 
	XMVECTOR vNewPos = XMVectorLerp(vCurrentPos, vTargetPos, t); 
	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, vNewPos);
	SetPosition(newPos);

}

void CEnemy::StartCollision(CGameObject* pOther)
{
    if (pOther->GetObjectType() == OBJ_PLAYER) // 충돌한 객체가 플레이어인 경우
    {
        SetDestroyed(true); // 체력이 0 이하가 되면 적을 파괴 상태로 설정합니다.
        CParticle* pExplosion = new CParticle();
		XMFLOAT3 position = GetPosition();
        position.z += 5.0f;
        pExplosion->Explode(position, 100, RGB(255, 0, 0)); // 폭발 효과 생성 (위치, 입자 수, 색상)
        SCENE_MANAGER->GetCurrentScene()->AddObject(pExplosion); // 폭발 효과를 현재 씬에 추가
	}
    if (pOther->GetObjectType() == OBJ_BULLET) // 충돌한 객체가 총알인 경우
    {
        health -= 1; // 체력을 감소시킵니다. (예: 총알 한 발당 1의 피해)
		SetColor(RGB(255, 0, 0)); // 적의 색상을 빨간색으로 변경하여 피해를 입었음을 시각적으로 표시 (예시)
        
	}
}

void CEnemy::SetGridMap(const std::vector<std::string>* pGrid, int nW, int nH, float fTileSize, float fOffX, float fOffZ)
{
    m_pGridData = pGrid;
    m_nMapWidth = nW;
    m_nMapHeight = nH;
    m_fTileSize = fTileSize;
    m_fOffsetX = fOffX;
	m_fOffsetZ = fOffZ;
}

void CEnemy::FindPath(XMFLOAT3 start, XMFLOAT3 end)
{
    m_vPath.clear();
    m_nPathIndex = 0;
    if (!m_pGridData) return;

    XMINT2 sg = WorldToGrid(start.x, start.z);
    XMINT2 eg = WorldToGrid(end.x, end.z);
    if (IsWallGrid(sg.x, sg.y) || IsWallGrid(eg.x, eg.y)) return;
    if (sg.x == eg.x && sg.y == eg.y) return;

    // g, f 비용 / 부모기록
    std::vector<std::vector<float>> gCost(m_nMapHeight,
        std::vector<float>(m_nMapWidth, FLT_MAX));
    std::vector<std::vector<bool>> closed(m_nMapHeight,
		std::vector<bool>(m_nMapWidth, false));
	std::vector<std::vector<XMINT2>> parent(m_nMapHeight,
		std::vector<XMINT2>(m_nMapWidth, XMINT2(-1, -1)));

    auto heuristic = [&](int c, int r) {
        return static_cast<float>(abs(c - eg.x) + abs(r - eg.y));
        };

    // open set: (f, col, row)
    using PQNode = std::tuple<float, int, int>;
    std::priority_queue<PQNode, std::vector<PQNode>,
        std::greater<PQNode>> open;

	gCost[sg.y][sg.x] = 0.0f;
	open.push({ heuristic(sg.x, sg.y), sg.x, sg.y });

	const std::vector<int> dCol = { 0, 1, 0, -1 };
	const std::vector<int> dRow = { -1, 0, 1, 0 };
    bool found = false;

    while (!open.empty())
    {
        auto [f, col, row] = open.top(); open.pop();
		if (closed[row][col]) continue;
        closed[row][col] = true;
        if (col == eg.x && row == eg.y) { found = true; break; }

        for (int i = 0; i < 4; ++i)
        {
			int nCol = col + dCol[i], nRow = row + dRow[i];
            if (IsWallGrid(nCol, nRow) || closed[nRow][nCol]) continue;
			float ng = gCost[row][col] + 1.0f; // 인접한 칸으로 이동하는 비용은 1로 가정
            if (ng < gCost[nRow][nCol])
            {
                gCost[nRow][nCol] = ng;
                parent[nRow][nCol] = XMINT2(col, row);
				open.push({ ng + heuristic(nCol, nRow), nCol, nRow });
            }
        }

    }

    if (!found) return;

	// 경로 역추적
    std::vector<XMFLOAT3> rev;
    int c = eg.x, r = eg.y;
    while (!(c == sg.x && r == sg.y))
    {
        rev.push_back(GridToWorld(c, r));
		XMINT2 p = parent[r][c];
        c = p.x; r = p.y;
    }
    for (int i = (int)rev.size() - 1; i >= 0; --i)
        m_vPath.push_back(rev[i]);





}

bool CEnemy::IsWallGrid(int col, int row) const
{
    if (!m_pGridData) return true; 
	if (col < 0 || col >= m_nMapWidth || row < 0 || row >= m_nMapHeight) return true;
	return (*m_pGridData)[row][col] == '#';
}

XMINT2 CEnemy::WorldToGrid(float wx, float wz) const
{
    int col = static_cast<int>((wx - m_fOffsetX) / m_fTileSize + 0.5f);
	int row = static_cast<int>((wz - m_fOffsetZ) / m_fTileSize + 0.5f);
    return XMINT2(col, row);
}

XMFLOAT3 CEnemy::GridToWorld(int col, int row) const
{
   return XMFLOAT3(m_fOffsetX + col * m_fTileSize, 0.0f,
	   m_fOffsetZ + row * m_fTileSize);
}
