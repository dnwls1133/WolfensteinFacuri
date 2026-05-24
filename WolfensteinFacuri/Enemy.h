#pragma once
#include "GameObject.h"
#include "define.h"
class CEnemy :
    public CGameObject
{
public:
    CEnemy();
    virtual ~CEnemy();
    virtual void Update() override;
	void setowncolor(DWORD c) { color = c; }
  
    virtual void StartCollision(CGameObject* pOther) override;

    virtual void OnCollision(CGameObject* pOther) override
    {
       
    }
    virtual void EndCollision(CGameObject* pOther) override
    {
        SetColor(color);
    }

	void SetSpeed(float fSpeed) { speed = fSpeed; }

    void SetGridMap(const std::vector<std::string>* pGrid,
        int nW, int nH, float fTileSize,
        float fOffX, float fOffZ);
private:
    int health = 5;
    DWORD color;
    float speed;
	float rotationSpeed;
	bool isSpawned = false; 
    XMFLOAT3 Axis;
	XMFLOAT3 directionToPlayer;


    const std::vector<std::string>* m_pGridData = nullptr;
    int m_nMapWidth = 0;
    int m_nMapHeight = 0;
    float m_fTileSize = 10.0f;
    float m_fOffsetX = 0.0f;
    float m_fOffsetZ = 0.0f;

    std::vector<XMFLOAT3> m_vPath; // A* 경로
    int m_nPathIndex = 0;
    float m_fPathTimer = 0.0f;

    static constexpr float kDetectionRange = 100.0f; // 10 타일
    static constexpr float kPathUpdateInterval = 0.5f; // 경로 재계산
    static constexpr float kWaypointRadius = 1.5f; // 웨이포인트 도달 판정 거리
    static constexpr float kEnemyRadius = 2.0f;

	void        FindPath(XMFLOAT3 start, XMFLOAT3 end);
    bool 	    IsWallGrid(int col, int row) const;
	XMINT2      WorldToGrid(float wx, float wz) const;
	XMFLOAT3    GridToWorld(int col, int row) const;

};

