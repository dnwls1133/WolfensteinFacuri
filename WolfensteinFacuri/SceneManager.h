#pragma once
#include "Scene.h"
class CScene;
class ID3D12Device;
class ID3D12GraphicsCommandList;


class CSceneManager
{
public:
	static CSceneManager* GetInstance()
	{
		static CSceneManager instance;
		return &instance;
	}

	CSceneManager(const CSceneManager&) = delete; 
	CSceneManager& operator=(const CSceneManager&) = delete;

	CSceneManager() {};
	virtual ~CSceneManager() {};

private:
	CScene* m_pCurrentScene{ nullptr }; // 현재 활성화된 씬의 포인터

public:
	void SetCurrentScene(CScene* pScene) { m_pCurrentScene = pScene; }
	CScene* GetCurrentScene() const { return m_pCurrentScene; }

	void ChangeScene(CScene* pNewScene, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	{
		if (m_pCurrentScene == pNewScene) return;

		if (m_pCurrentScene)
		{
			delete m_pCurrentScene;
			m_pCurrentScene = nullptr;
		}

		m_pCurrentScene = pNewScene;

		if (m_pCurrentScene && pd3dDevice && pd3dCommandList)
		{
			m_pCurrentScene->BuildObjects(pd3dDevice, pd3dCommandList);
		}
	}
	
	void RelaseCurrentScene()
	{
		if (m_pCurrentScene)
		{
			delete m_pCurrentScene;
			m_pCurrentScene = nullptr;
		}
	}

};



