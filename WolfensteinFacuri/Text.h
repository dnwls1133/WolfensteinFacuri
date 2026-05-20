#pragma once
#include "GameObject.h"



class CText :
    public CGameObject
{
public:
	CText() = default;
	CText(TextType textType) : m_TextType(textType) {};
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

	TextType GetTextType() const { return m_TextType; }

	void SetTextType(TextType textType) { m_TextType = textType; }

private:
	TextType m_TextType;
};

