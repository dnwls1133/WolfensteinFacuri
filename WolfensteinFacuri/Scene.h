#pragma once
#include "InputState.h"
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
	int				m_nObjects{ 0 }; // ���� �����ϴ� ��ü�� ����
	std::vector<CGameObject*> m_vpObjects; // ���� �����ϴ� ��ü���� ������ �迭
	
	CCubeMesh* m_pCubeMesh{NULL};
	CMutantMesh* m_pEnemyMesh{ NULL };
	CPlayerMesh* m_pPlayerMesh{ NULL };
	CMissileMesh* m_pMissileMesh{ NULL };
	CPlaneMesh* m_pFloorMesh{ NULL };
	CWallMesh* m_pWallMesh{ NULL };
	std::vector<C3DTextMesh*> m_vpTextMeshes; 



	CMesh*		  m_pParticleMesh{ NULL };

	float endTimer = 0.0f;

	CPlayer* m_pPlayer{ NULL }; // ���� �����ϴ� �÷��̾��� ������
	CCamera* m_pCamera{ NULL }; // ���� �����ϴ� ī�޶��� ������


	int m_nBulletObjects{ 0 };
	int m_nEffectObjects{ 0 };


	CColliderManager* m_pColliderManager{ NULL }; // �浹 �����ڸ� ���� ���Խ�ŵ�ϴ�.



	// [D3D12 �߰�] Root Signature + ���̴�

	ID3D12RootSignature* m_pd3dGrahpicsRootSignature = NULL;
	CDiffusedShader* m_pShader = NULL;
	ID3D12Device* m_pd3dDevice = NULL;
	ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;


	// ��ƼŬ �ν��Ͻ�
	CInstancedShader* m_pInstancedShader = NULL;
	CInstancebuffer   m_particleInstanceBuffer;

	// [추가] OOBB 와이어프레임 디버그 자원
	CWireframeShader* m_pWireShader = NULL;
	CCubeMesh*        m_pWireCubeMesh = NULL;   // 단위 큐브 (fWHD=2.0 → ±1)


protected:
	virtual void BuildSharedResources();
	virtual void BuildSceneObjects() = 0;
	virtual void UpdateCamera(float fElapsedTime);


public:

	CGameObject* GetPlayer() const { return m_pPlayer; }
	CCamera* GetCamera() const { return m_pCamera; }


	CMissileMesh* GetMissileMesh() const { return m_pMissileMesh; }
	CMesh* GetParticleMesh() const { return m_pParticleMesh; }

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void AddObject(CGameObject* pObject); // ���� ��ü�� �߰��ϴ� �Լ�
	void ReleaseObjects();
	void ClearGarbageCollection(); // ������ ��ü�� �����ϴ� �Լ� (�޸� ���� ����)
	
	XMFLOAT3 GetLightDirection() const { return lightDir; }


	XMFLOAT3 lightDir = XMFLOAT3(-1.0f, -1.0f, 1.0f);

	virtual void Animate(float fElapsedTime);
	
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// [추가] F1 토글 — CGameFramework가 켜고 끔. 모든 Scene 인스턴스에 공통 적용.
	static bool s_bDebugWireframe;
	void RenderDebugBoxes(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// [D3D12 �߰�] Root Signature ����
	void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	ID3D12RootSignature* GetGraphicsRootSignature() const { return m_pd3dGrahpicsRootSignature; }
	
	// �Է�ó��
	virtual void ProcessInput(const InputState& InputState, float fElapsedTime); // �Է� ó�� �Լ� (��: Ű����, ���콺)


	CPlayer* GetPlayer() { return m_pPlayer; }
};

