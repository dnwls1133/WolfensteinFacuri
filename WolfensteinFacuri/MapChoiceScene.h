#pragma once
#include "Scene.h"

class MapChoiceScene :
	public CScene
{
public:
	MapChoiceScene(CColliderManager* pCollider, CCamera* pCamera);
	~MapChoiceScene() override = default;

	virtual void Animate(float fElapsedTime) override;
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime) override;

protected:
	void BuildSceneObjects() override;
	void UpdateCamera(float fElapsedTime) override;
};


