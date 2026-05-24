#pragma once
#include "Scene.h"

class CStairMesh;
class CJointPlayer;

class WFSMap1Scene :
    public CScene
{
public:
    WFSMap1Scene(CColliderManager* pCollider, CCamera* pCamera);
	virtual ~WFSMap1Scene();

	virtual void Animate(float fElapsedTime) override;
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime) override;

	bool IsWall(float wx, float wz) const; 

	float GetFloorY(float wx, float wz) const;

	int m_nEnemyCount = 0; // 현재 맵에 존재하는 적의 수

protected:
	virtual void BuildSceneObjects() override;
	virtual void UpdateCamera(float fElapsedTime) override;

private:
	// -- 맵 데이터 ----
	int m_nMapWidth = 0;
	int m_nMapHeight = 0;
	std::vector<std::string> m_vGridData;
	static constexpr float kTileSize = 10.0f;

	static constexpr float kStepHeight = 0.5f;
	static constexpr float kMaxStepUp = 2.5f;

	CJointPlayer* m_pJointPlayer{ nullptr };

	CStairMesh* m_pStairMesh{ nullptr };

	bool LoadMapFile(const wchar_t* relPath);
	void SpawnFromGrid();

	void SpawnWall (float wx, float wz);
	void SpawnFloor(float wx, float wz, float fHeight = 0.0f);
	void SpawnStair(float wx, float wz);
	void SpawnEnemy(float wx, float wz);
	void PlacePlayer(float wx, float wz);

};

