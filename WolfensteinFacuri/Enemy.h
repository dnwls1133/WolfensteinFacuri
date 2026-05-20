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
private:
    int health = 5;
    DWORD color;
    float speed;
	float rotationSpeed;
	bool isSpawned = false; 
    XMFLOAT3 Axis;
	XMFLOAT3 directionToPlayer;
};

