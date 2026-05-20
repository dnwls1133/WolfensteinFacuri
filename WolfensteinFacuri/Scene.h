#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Player.h"
#include "Shader.h"
#include "Instancebuffer.h"


typedef unsigned long long ULONGLONG;

class CColliderManager;

class CScene
{
public:

	CScene(CColliderManager* pCollider, CCamera* pCamera);
	virtual ~CScene();

protected:
	int				m_nObjects{ 0 }; // 씬에 존재하는 객체의 개수
	std::vector<CGameObject*> m_vpObjects; // 씬에 존재하는 객체들의 포인터 배열
	

	CMutantMesh* m_pEnemyMesh{ NULL };
	CPlayerMesh* m_pPlayerMesh{ NULL };
	CMissileMesh* m_pMissileMesh{ NULL };
	CPlaneMesh* m_pFloorMesh{ NULL };
	CWallMesh* m_pWallMesh{ NULL };
	std::vector<C3DTextMesh*> m_vpTextMeshes; 



	CMesh*		  m_pParticleMesh{ NULL };

	float endTimer = 0.0f;

	CPlayer* m_pPlayer{ NULL }; // 씬에 존재하는 플레이어의 포인터
	CCamera* m_pCamera{ NULL }; // 씬에 존재하는 카메라의 포인터


	int m_nBulletObjects{ 0 };
	int m_nEffectObjects{ 0 };


	CColliderManager* m_pColliderManager{ NULL }; // 충돌 관리자를 씬에 포함시킵니다.



	// [D3D12 추가] Root Signature + 셰이더

	ID3D12RootSignature* m_pd3dGrahpicsRootSignature = NULL;
	CDiffusedShader* m_pShader = NULL;
	ID3D12Device* m_pd3dDevice = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;


	// 파티클 인스턴싱
	CInstancedShader* m_pInstancedShader = NULL;
	CInstancebuffer   m_particleInstanceBuffer;


protected:
	virtual void BuildSharedResources();
	virtual void BuildSceneObjects() = 0;
	virtual void UpdateCamera(float fElapsedTime);


public:

	CGameObject* GetPlayer() const { return m_pPlayer; }

	CMissileMesh* GetMissileMesh() const { return m_pMissileMesh; }
	CMesh* GetParticleMesh() const { return m_pParticleMesh; }

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void AddObject(CGameObject* pObject); // 씬에 객체를 추가하는 함수
	void ReleaseObjects();
	void ClearGarbageCollection(); // 씬에서 객체를 제거하는 함수 (메모리 해제 포함)
	
	XMFLOAT3 GetLightDirection() const { return lightDir; }


	XMFLOAT3 lightDir = XMFLOAT3(-1.0f, -1.0f, 1.0f);

	virtual void Animate(float fElapsedTime);
	
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// [D3D12 추가] Root Signature 생성
	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	ID3D12RootSignature* GetGraphicsRootSignature() const { return m_pd3dGrahpicsRootSignature; }
	
	// 입력처리
	virtual void ProcessInput(float fElapsedTime); // 입력 처리 함수 (예: 키보드, 마우스)


	CPlayer* GetPlayer() { return m_pPlayer; }
};

