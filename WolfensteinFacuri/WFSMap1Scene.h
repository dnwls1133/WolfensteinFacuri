#pragma once
#include "Scene.h"
class WFSMap1Scene :
    public CScene
{
public:
    WFSMap1Scene(CColliderManager* pCollider, CCamera* pCamera);
	virtual ~WFSMap1Scene() = default;

	virtual void Animate(float fElapsedTime) override;
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime) override;

protected:
	virtual void BuildSceneObjects() override;
	virtual void UpdateCamera(float fElapsedTime) override;

};

