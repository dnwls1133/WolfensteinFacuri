#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Missile.h"

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer();

protected:
	CCamera* m_pCamera{ NULL }; 


	int m_nHealth = 10; 
	float m_fPlayTime = 0.0f;
	bool m_playerDestroyed = false; // 플레이어가 파괴되었는지 여부를 나타내는 플래그
	bool m_playerWon = false;


public:
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* GetCamera() const { return m_pCamera; }
	bool IsDestroyed() const { return m_playerDestroyed; }
	bool IsWon() const { return m_playerWon; }
	
	void Move(DWORD dwDirection, float fDistance);
	void Rotate(float fPitch, float fYaw, float fRoll);




	//void FireMissile(); // 미사일 발사 함수

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
			//m_nHealth = 0; // 벽과 충돌 시 체력 감소
		}
		
	}
	virtual void OnCollision(CGameObject* pOther) override
	{
		
		
	}
	virtual void EndCollision(CGameObject* pOther) override
	{
		
	}
   
};

