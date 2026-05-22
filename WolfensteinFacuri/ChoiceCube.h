#pragma once
#include "GameObject.h"
class CChoiceCube :
    public CGameObject
{
public:
	CChoiceCube() = default;
    virtual ~CChoiceCube() {};

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

