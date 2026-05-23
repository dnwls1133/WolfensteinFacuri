#pragma once
#include "Scene.h"


class CStairMesh;
class CAircraft;

class WFSMap2Scene :
	public CScene
{
public:
	WFSMap2Scene(CColliderManager* pCollider, CCamera* pCamera);
	virtual ~WFSMap2Scene();

	virtual void Animate(float fElapsedTime) override;
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime) override;

protected:
	virtual void BuildSceneObjects() override;
	virtual void UpdateCamera(float fElapsedTime) override;

private:
	// -- ∏  µ•¿Ã≈Õ ----
	int m_nMapWidth = 0;
	int m_nMapHeight = 0;
	std::vector<std::string> m_vGridData;
	static constexpr float kTileSize = 10.0f;

	CAircraft* m_pAIrcraft{ nullptr };

	CStairMesh* m_pStairMesh{ nullptr };

	bool LoadMapFile(const wchar_t* relPath);
	void SpawnFromGrid();

	void SpawnWall(float wx, float wz);
	void SpawnFloor(float wx, float wz);
	void SpawnStair(float wx, float wz);
	void PlacePlayer(float wx, float wz);

};