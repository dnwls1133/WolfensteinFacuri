#pragma once
#include "GameObject.h"

class CWall : public CGameObject
{
public:
	CWall() {};
	virtual ~CWall() {};

	virtual void StartCollision(CGameObject* pOther) override;
	virtual void OnCollision(CGameObject* pOther) override
	{
		
	}
	virtual void EndCollision(CGameObject* pOther) override
	{
		
	}
};

