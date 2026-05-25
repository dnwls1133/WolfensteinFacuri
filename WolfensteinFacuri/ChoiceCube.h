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

	int m_nChoiceID{ 0 }; // 선택지 ID (1, 2, 3 등)

};

