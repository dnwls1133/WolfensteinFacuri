#pragma once
#include "Scene.h"
class GalagaScene :
    public CScene
{
private:
	std::vector<CGameObject*> m_vpFloorObjects;
	std::vector<CGameObject*> m_vpWallObjects;
	std::vector<CGameObject*> m_vpEnemyObjects;
	XMFLOAT3 m_xmf3Wallposition;
	XMFLOAT3 m_xmf3Enemyspawnposition;
public:
    GalagaScene(CColliderManager* pCollider, CCamera* pCamera);
	~GalagaScene() override = default;
	virtual void Animate(float fElapsedTime) override;
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime) override;
protected:
	void BuildSceneObjects() override;
};
