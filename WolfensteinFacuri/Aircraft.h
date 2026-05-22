#pragma once
#include "Player.h"
class CAircraft :
    public CPlayer
{
public:
    CAircraft();
    virtual ~CAircraft() = default;

public:
    float m_fFireCooldown = 0.1f;
    float m_fLastFireTime = 0.0f;
public:

    void FireMissile(); // 미사일 발사 함수

	virtual void Update() override;

public:
	virtual void StartCollision(CGameObject* pOther) override
	{
		if (pOther->GetObjectType() == OBJ_ENEMY)
		{
			--m_nHealth; // 적과 충돌 시 체력 감소
		}
		if (pOther->GetObjectType() == OBJ_WALL || pOther->GetObjectType() == OBJ_MONSTER)
		{
			m_nHealth = 0; // 벽과 충돌 시 체력 감소
		}

	}
	virtual void OnCollision(CGameObject* pOther) override
	{


	}
	virtual void EndCollision(CGameObject* pOther) override
	{

	}
};

