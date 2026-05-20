#pragma once
#include "GameObject.h"


class CMissile : public CGameObject
{
public:
	CMissile() {};
	virtual ~CMissile() {};

	void setDirection(const XMFLOAT3& direction)
	{
		dir = direction;
	}

	virtual void Update() override;
	virtual void Animate(float fElapsedTime);

	virtual void StartCollision(CGameObject* pOther) override;
	virtual void OnCollision(CGameObject* pOther) override
	{
		
	}
	virtual void EndCollision(CGameObject* pOther) override
	{
		SetDestroyed(true); // 미사일을 파괴 상태로 설정
	}

private:
	XMFLOAT3 dir;
	float speed = 200.0f; 
	float lifeTime = 1.5f; 
	float elapsedTime = 0.0f; 


};

