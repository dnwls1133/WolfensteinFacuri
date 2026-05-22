#include "stdafx.h"
#include "ChoiceCube.h"
#include "Text.h"
#include "WFSMap1Scene.h"
#include "GalagaScene.h"
#include "MapChoiceScene.h"

MapChoiceScene::MapChoiceScene(CColliderManager* pCollider, CCamera* pCamera)
	: CScene(pCollider, pCamera)
{
}

void MapChoiceScene::Animate(float fElapsedTime)
{
	CScene::Animate(fElapsedTime);
}

void MapChoiceScene::ProcessInput(const InputState& InputState, float fElapsedTime)
{
	const UCHAR* pKeyBuffer = InputState.keys;

	static bool bLeftClick = false;
	if (pKeyBuffer[VK_LBUTTON] & 0x80)
	{
		if (!bLeftClick)
		{
			bLeftClick = true;

			float mouseX = static_cast<float>(InputState.mousePosition.x);
			float mouseY = static_cast<float>(InputState.mousePosition.y);

			XMFLOAT3 vNear = m_pCamera->ScreenToWorld(mouseX, mouseY, 0.0f); // 근 평면 좌표
			XMFLOAT3 vFar = m_pCamera->ScreenToWorld(mouseX, mouseY, 1.0f); // 원 평면 좌표

			// Ray의 시작점과 Direction 계산
			XMVECTOR vRayOrigin = XMLoadFloat3(&vNear);
			XMVECTOR vRayFar = XMLoadFloat3(&vFar);
			XMVECTOR vRayDirection = XMVector3Normalize(XMVectorSubtract(vRayFar, vRayOrigin));

			// Ray와 충돌하는 객체 OBB 검사
			float fMinDist = FLT_MAX;
			CGameObject* pSelectedObject = nullptr;

			for (CGameObject* pObject : m_vpObjects)
			{
				if (!pObject->IsActive() || pObject->IsDestroyed()) continue;

				float fDist = 0.0f;

				if (pObject->m_xmOOBB.Intersects(vRayOrigin, vRayDirection, fDist))
				{
					if (fDist < fMinDist)
					{
						fMinDist = fDist;
						pSelectedObject = pObject;
					}
				}


			}

			// 선택된 객체가 있으면 처리
			if (pSelectedObject)
			{
				SCENE_MANAGER->RequestSceneChange(new WFSMap1Scene(m_pColliderManager, m_pCamera));
			}


		}
		else
		{
			bLeftClick = false; // 클릭이 끝났을 때 플래그 초기화
		}


	}
}

void MapChoiceScene::BuildSceneObjects()
{
	m_pPlayer = new CPlayer();
	m_pPlayer->SetCamera(m_pCamera);
	m_pPlayer->SetPosition(0.0f, 0.0f, -100.0f);
	m_pPlayer->SetObjectType(OBJ_PLAYER);
	// 플레이어 색상 설정
	m_pPlayer->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f));

	m_pPlayer->SetMesh(m_pPlayerMesh);
	m_pPlayer->SetShader(m_pShader);
	m_pPlayer->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);

	CChoiceCube* pChoiceCube = new CChoiceCube();
	pChoiceCube->SetPosition(-25.0f, 0.0f, -50.0f);
	pChoiceCube->SetObjectType(OBJ_CHOICE);
	pChoiceCube->SetMesh(m_pCubeMesh);
	pChoiceCube->SetShader(m_pShader);
	pChoiceCube->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pChoiceCube->SetColor(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.f)); // 빨간색으로 설정
	pChoiceCube->GenerateBoundingBox(); // OOBB 생성
	AddObject(pChoiceCube);

	CText* pChoiceText = new CText(TEXT_CHOICE);
	pChoiceText->SetPosition(-25.0f, 0.0f, -75.0f); // 텍스트 위치 설정
	pChoiceText->SetObjectType(OBJ_TEXT);
	pChoiceText->SetMesh(m_vpTextMeshes[2]);
	pChoiceText->SetShader(m_pShader);
	pChoiceText->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pChoiceText->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f)); // 텍스트 색상 설정
	pChoiceText->GenerateBoundingBox(); // 텍스트 객체의 OOBB 생성
	AddObject(pChoiceText); // 씬에 텍스트 객체 추가

	// 다른 선택 큐브 추가 (예시)
	CChoiceCube* pChoiceCube2 = new CChoiceCube();
	pChoiceCube2->SetPosition(25.0f, 0.0f, -50.0f);
	pChoiceCube2->SetObjectType(OBJ_CHOICE);
	pChoiceCube2->SetMesh(m_pCubeMesh);
	pChoiceCube2->SetShader(m_pShader);
	pChoiceCube2->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pChoiceCube2->SetColor(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.f)); // 파란색으로 설정
	pChoiceCube2->GenerateBoundingBox(); // OOBB 생성
	AddObject(pChoiceCube2);

	CText* pChoiceText2 = new CText(TEXT_CHOICE);
	pChoiceText2->SetPosition(25.0f, 0.0f, -75.0f); // 텍스트 위치 설정
	pChoiceText2->SetObjectType(OBJ_TEXT);
	pChoiceText2->SetMesh(m_vpTextMeshes[3]);
	pChoiceText2->SetShader(m_pShader);
	pChoiceText2->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pChoiceText2->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f)); // 텍스트 색상 설정
	pChoiceText2->GenerateBoundingBox(); // 텍스트 객체의 OOBB 생성
	AddObject(pChoiceText2); // 씬에 텍스트 객체 추가

}
void MapChoiceScene::UpdateCamera(float fElapsedTime)
{
	if (!m_pCamera) return;

	m_pCamera->SetPosition(0.0f, 0.0f, -150.0f);
	m_pCamera->Update();
}
