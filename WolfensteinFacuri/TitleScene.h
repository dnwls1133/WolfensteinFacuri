#pragma once
#include "Scene.h"
class TitleScene :
    public CScene
{
public:
    TitleScene(CColliderManager* pCollider, CCamera* pCamera);
	~TitleScene() override = default;

	virtual void Animate(float fElapsedTime) override;
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime) override;

protected:
	void BuildSceneObjects() override;
	void UpdateCamera(float fElapsedTime) override;
};

