#include "stdafx.h"
#include "GraphicsPipeline.h"
#include "Enemy.h"
#include "Wall.h"
#include "Scene.h"
#include "CGameFramework.h"
#include "ColliderManager.h"



static const std::vector<std::string> textMeshNames = {
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
	"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
};

CScene::CScene(CColliderManager* pCollider, CCamera* pCamera)
	: m_pColliderManager(pCollider), m_pCamera(pCamera)
{

	
}

CScene::~CScene()
{
	ReleaseObjects();

	//[D3d12 추가] 셰이더 및 루트 시그니처 해제
	if (m_pShader) m_pShader->Release();
	if (m_pd3dGrahpicsRootSignature) m_pd3dGrahpicsRootSignature->Release();

	if (m_pMissileMesh) m_pMissileMesh->Release();
	if (m_pPlayerMesh) m_pPlayerMesh->Release();
	if (m_pFloorMesh) m_pFloorMesh->Release();
	if (m_pWallMesh) m_pWallMesh->Release();
	if (m_pEnemyMesh) m_pEnemyMesh->Release();
	if (m_pParticleMesh) m_pParticleMesh->Release();
	for (auto pTextMesh : m_vpTextMeshes) {
		if (pTextMesh) pTextMesh->Release();
	}
	m_vpTextMeshes.clear();	

	if (m_pInstancedShader) m_pInstancedShader->Release();
	m_particleInstanceBuffer.Release();
}

void CScene::ProcessInput(const InputState& InputState, float fElapsedTime)
{
	
}



// [D3D12 추가] Root Signature 생성
// Root Parameter 0 : b0
// Root Parameter 1 : b1 

void CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	D3D12_ROOT_PARAMETER pd3dRootParameters[2];
	::ZeroMemory(pd3dRootParameters, sizeof(pd3dRootParameters));

	// b0 : CB_GAMEOBJECT_INFO (게임 객체별 상수 버퍼)
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 0; // b0
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// b1 : CB_SCENE_INFO (씬 전체 상수 버퍼)
	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[1].Descriptor.ShaderRegister = 1; // b1
	pd3dRootParameters[1].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// IA 활성화 플래그
	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(d3dRootSignatureDesc));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = nullptr;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);

	pd3dDevice->CreateRootSignature(0,
		pd3dSignatureBlob->GetBufferPointer(),
		pd3dSignatureBlob->GetBufferSize(),
		__uuidof(ID3D12RootSignature),
		(void**)&m_pd3dGrahpicsRootSignature);

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();
}



void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;
	m_pd3dCommandList = pd3dCommandList;

	CreateGraphicsRootSignature(pd3dDevice);
	BuildSharedResources();
	BuildSceneObjects();


	
}

void CScene::BuildSharedResources()
{
	m_pShader = new CDiffusedShader();
	m_pShader->CreateShader(m_pd3dDevice, m_pd3dGrahpicsRootSignature);
	m_pShader->AddRef();

	m_pEnemyMesh = new CMutantMesh(m_pd3dDevice, m_pd3dCommandList, 1.0f);
	m_pEnemyMesh->AddRef();

	m_pPlayerMesh = new CPlayerMesh(m_pd3dDevice, m_pd3dCommandList, 1.0f);
	m_pPlayerMesh->AddRef();

	m_pFloorMesh = new CPlaneMesh(m_pd3dDevice, m_pd3dCommandList, 10.0f, 10.0f);
	m_pFloorMesh->AddRef();

	m_pWallMesh = new CWallMesh(m_pd3dDevice, m_pd3dCommandList, 10.0f, 10.0f, 10.0f);
	m_pWallMesh->AddRef();

	m_pMissileMesh = new CMissileMesh(m_pd3dDevice, m_pd3dCommandList, 0.5f);
	m_pMissileMesh->AddRef();

	m_pParticleMesh = new CCubeMesh(m_pd3dDevice, m_pd3dCommandList, 0.2f);
	m_pParticleMesh->AddRef();

	for(int i = 0; i < 24; i++)
	{
		C3DTextMesh* pTextMesh = new C3DTextMesh(m_pd3dDevice, m_pd3dCommandList, textMeshNames[i], 1.0f, RANDOM_COLOR);
		pTextMesh->AddRef();
		m_vpTextMeshes.push_back(pTextMesh);
	}

	m_pInstancedShader = new CInstancedShader();
	m_pInstancedShader->CreateShader(m_pd3dDevice, m_pd3dGrahpicsRootSignature);
	m_pInstancedShader->AddRef();

	m_particleInstanceBuffer.Create(m_pd3dDevice, 2048);

}



void CScene::AddObject(CGameObject* pObject)
{
	if (!pObject) return; 
	const int nMaxBullets = 240;
	const int nMaxEffects = 24;

	ObjectType type = pObject->GetObjectType();


	if (type == OBJ_BULLET && m_nBulletObjects >= nMaxBullets)
	{
		delete pObject;
		return;
	}
	if (type == OBJ_EFFECT && m_nEffectObjects >= nMaxEffects)
	{
		delete pObject;
		return;
	}

	if (pObject && m_pd3dDevice)
	{
		pObject->SetShader(m_pShader);
		pObject->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	}

	m_vpObjects.push_back(pObject);
	++m_nObjects;

	if (type == OBJ_BULLET) ++m_nBulletObjects;
	else if (type == OBJ_EFFECT) ++m_nEffectObjects;
}

void CScene::ReleaseObjects()
{
	if (!m_vpObjects.empty())
	{
		for (int i = 0; i < m_nObjects; i++) delete m_vpObjects[i];
		m_vpObjects.clear();
	}
	if (m_pPlayer)
	{
		delete m_pPlayer;
		m_pPlayer = nullptr;
	}
}

void CScene::ClearGarbageCollection()
{
	for (int i =0; i < m_vpObjects.size();)
	{
		if (m_vpObjects[i]->IsDestroyed())
		{
			ObjectType type = m_vpObjects[i]->GetObjectType();
			delete m_vpObjects[i];

			m_vpObjects[i] = m_vpObjects.back();
			m_vpObjects.pop_back();
			--m_nObjects;

			if (type == OBJ_BULLET && m_nBulletObjects > 0) --m_nBulletObjects;
			else if (type == OBJ_EFFECT && m_nEffectObjects > 0) --m_nEffectObjects;
		}
		else
		{
			i++; // 삭제하지 않은 경우에만 인덱스 증가
		}

	}


}



void CScene::Animate(float fElapsedTime)
{

	lightDir = Vector3::Normalize(lightDir); // 광원 방향 벡터를 정규화하여 사용
	lightDir = Vector3::TransformNormal(lightDir, XMMatrixRotationY(fElapsedTime * 0.1f)); // 광원 방향을 시간에 따라 회전시킴
	for (auto& object : m_vpObjects)
	{
		if (!object->IsActive()) continue; // 비활성화된 객체는 업데이트 안 함
		object->Update(); // 월드 행렬 갱신

	}
	if (m_pPlayer) m_pPlayer->Update();

	//m_mapCollisionGroups.clear(); // 매 프레임마다 충돌 그룹을 새로 만들어야 하므로 맵 초기화

	UpdateCamera(fElapsedTime);

	m_pColliderManager->clearCollisionGroups(); // 충돌 그룹 초기화


	if (m_pPlayer) m_pColliderManager->AddToCollisionGroup(m_pPlayer); // 플레이어는 항상 충돌 그룹에 추가

	for (int i = 0; i < m_nObjects; i++)
	{
		if(m_vpObjects[i]->IsActive() && !m_vpObjects[i]->IsDestroyed())
			m_pColliderManager->AddToCollisionGroup(m_vpObjects[i]); // 각 객체는 자신의 타입 그룹에 추가
		
	}

	m_pColliderManager->CheckAllCollisions(); // 매 프레임마다 충돌 검사
}

void CScene::UpdateCamera(float fElapsedTime)
{
	if (!m_pPlayer || !m_pCamera) return;

	XMFLOAT3 playerPos = m_pPlayer->GetPosition();
	XMFLOAT3 playerLook = m_pPlayer->GetDirection();
	playerLook = Vector3::Normalize(playerLook);

	XMFLOAT3 offset = Vector3::ScalarProduct(playerLook, -5.0f, true);
	XMFLOAT3 targetCameraPos = Vector3::Add(playerPos, offset);
	targetCameraPos.y += 2.0f;

	XMFLOAT3 currentCameraPos = m_pCamera->GetPosition();
	XMVECTOR vCurrent = XMLoadFloat3(&currentCameraPos);
	XMVECTOR vTarget = XMLoadFloat3(&targetCameraPos);

	const float fFollowSpeed = 5.0f;
	float t = fFollowSpeed * TIMER->GetTimeElapsed();
	if (t > 1.0f) t = 1.0f;

	XMVECTOR vLerp = XMVectorLerp(vCurrent, vTarget, t);
	XMFLOAT3 lerpPos;
	XMStoreFloat3(&lerpPos, vLerp);

	m_pCamera->SetPosition(lerpPos);
	m_pCamera->Update();

}



void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)	
{
	
	// 1. Root Signature 바인딩
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGrahpicsRootSignature);

	// 2. 뷰포트/가위 + 카메라 상수 버퍼(b1) 갱신/바인딩
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	// 3. 절두체 컬링 
	std::vector<CGameObject*> vEffectObjects;
	vEffectObjects.reserve(128);

	for (int i = 0; i < m_nObjects; ++i)
	{
		if (m_vpObjects[i]->IsDestroyed()) continue;

		XMFLOAT3 objPos = m_vpObjects[i]->GetPosition();
		bool isEffect = (m_vpObjects[i]->GetObjectType() == OBJ_EFFECT);
		float cullRadius = isEffect ? 15.0f : 1.0f;
		if (!pCamera->IsInFrustum(objPos, 1.0f))
		{
			m_vpObjects[i]->SetActive(false);
			continue;
		}
		m_vpObjects[i]->SetActive(true);

		// 이펙트는 따로 모았다가 마지막에
		if (isEffect)
		{
			vEffectObjects.push_back(m_vpObjects[i]);
			continue;
		}

		// 일반 오브젝트: 즉시 렌더
		m_vpObjects[i]->Render(pd3dCommandList, pCamera);
	}

	// 4) 플레이어 (절두체 안에 있으면 렌더)
	if (m_pPlayer)
	{
		XMFLOAT3 playerPos = m_pPlayer->GetPosition();
		if (pCamera->IsInFrustum(playerPos, 4.0f))
			m_pPlayer->Render(pd3dCommandList, pCamera);
	}

	// 5) 파티클 인스턴싱 렌더
	if (!vEffectObjects.empty() && m_pInstancedShader && m_pParticleMesh)
	{
		// [디버그] 진단 메시지
		static int s_frameCount = 0;
		if (++s_frameCount % 60 == 0)  // 1초에 한 번만 출력
		{
			char msg[256];
			sprintf_s(msg, "[Particle] EffectObjects=%d, Shader=%p, Mesh=%p\n",
				(int)vEffectObjects.size(), m_pInstancedShader, m_pParticleMesh);
			OutputDebugStringA(msg);
		}

		m_pInstancedShader->Render(pd3dCommandList, pCamera);
		m_particleInstanceBuffer.Begin();

		for (auto* pEffect : vEffectObjects)
			pEffect->AddToInstanceBuffer(m_particleInstanceBuffer);

		UINT nInstanceCount = m_particleInstanceBuffer.GetCount();

		// [디버그]
		if (s_frameCount % 60 == 0)
		{
			char msg[256];
			sprintf_s(msg, "[Particle] InstanceCount=%u\n", nInstanceCount);
			OutputDebugStringA(msg);
		}

		if (nInstanceCount > 0)
		{
			m_particleInstanceBuffer.Bind(pd3dCommandList, 1);
			m_pParticleMesh->RenderInstanced(pd3dCommandList, nInstanceCount);
		}
	}
	else
	{
		// [디버그] 가드에 걸린 경우
		static int s_skipCount = 0;
		if (++s_skipCount % 60 == 0)
		{
			char msg[256];
			sprintf_s(msg, "[Particle] SKIPPED: empty=%d, shader=%p, mesh=%p\n",
				vEffectObjects.empty() ? 1 : 0,
				m_pInstancedShader, m_pParticleMesh);
			OutputDebugStringA(msg);
		}
	}
}