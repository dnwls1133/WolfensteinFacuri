#pragma once
#include "GameObject.h"
class CText :
    public CGameObject
{
public:
    CText() {};
    virtual ~CText() {};

	virtual void StartCollision(CGameObject* pOther) override
	{

	}
	virtual void OnCollision(CGameObject* pOther) override
	{

	}
	virtual void EndCollision(CGameObject* pOther) override
	{

	}
};

