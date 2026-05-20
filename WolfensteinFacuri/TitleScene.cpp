#include "stdafx.h"
#include "Text.h"
#include "TitleScene.h"
#include "GalagaScene.h"



TitleScene::TitleScene(CColliderManager* pCollider, CCamera* pCamera)
	: CScene(pCollider, pCamera)
{
}

void TitleScene::Animate(float fElapsedTime)
{
	CScene::Animate(fElapsedTime);

	for (CGameObject* pObject : m_vpObjects)
	{
		if (pObject->IsActive() && !pObject->IsDestroyed())
		{
			if (pObject->GetObjectType() == OBJ_TEXT)
			{
				// 텍스트 객체는 애니메이션 효과를 위해 회전
				XMFLOAT3 rotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f); // Y축을 중심으로 회전
				pObject->Rotate(&rotationAxis,0.01f); // Y축을 중심으로 회전
			}
			pObject->Update();
		}
	}
}

void TitleScene::ProcessInput(const InputState& InputState, float fElapsedTime)
{
	const UCHAR* pKeyBuffer = InputState.keys;
	
	if (pKeyBuffer[VK_LEFT] & 0x80)
	{
		m_pPlayer->Rotate(0, 0, 0.01f); // 롤 회전 (좌회전)
		m_pPlayer->Move(DIR_LEFT, 20); // 좌회전

	}
	if (pKeyBuffer[VK_RIGHT] & 0x80)
	{
		m_pPlayer->Rotate(0, 0, -0.01f); // 롤 회전 (우회전)
		m_pPlayer->Move(DIR_RIGHT, 20); // 우회전

	}
	if (pKeyBuffer[VK_UP] & 0x80)
	{
		m_pPlayer->Move(DIR_FORWARD, 50); // 이동 속도
	}
	if (pKeyBuffer[VK_DOWN] & 0x80)
	{
		m_pPlayer->Move(DIR_BACKWARD, 50); // 이동 속도
	}
	if (pKeyBuffer[VK_CONTROL] & 0x80)
	{

		m_pPlayer->m_fLastFireTime += TIMER->GetTimeElapsed(); // 매 프레임마다 경과 시간을 누적
		if (m_pPlayer->m_fLastFireTime >= m_pPlayer->m_fFireCooldown) // 쿨다운 시간이 지났는지 체크
		{
			m_pPlayer->m_fLastFireTime = 0.0f; // 발사 후 마지막 발사 시간 초기화
			m_pPlayer->FireMissile(); // 미사일 발사

		}
	}
	
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
			if (pSelectedObject && dynamic_cast<CText*>(pSelectedObject)->GetTextType() == TEXT_PRESSSTART)
			{
				SCENE_MANAGER->ChangeScene(new GalagaScene(m_pColliderManager, m_pCamera),m_pd3dDevice, m_pd3dCommandList);
			}
		

		}
		else
		{
			bLeftClick = false; // 클릭이 끝났을 때 플래그 초기화
		}


	}
	
}

void TitleScene::BuildSceneObjects()
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
	
	
	CText* pTitleText = new CText(TEXT_TITLE);
	pTitleText->SetPosition(0.0f, 20.0f, -50.0f); // 텍스트 위치 설정
	pTitleText->SetObjectType(OBJ_TEXT);
	pTitleText->SetMesh(m_vpTextMeshes[0]);
	pTitleText->SetShader(m_pShader);
	pTitleText->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pTitleText->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f)); // 텍스트 색상 설정
	AddObject(pTitleText); // 씬에 텍스트 객체 추가

	CText* pStartText = new CText(TEXT_PRESSSTART);
	pStartText->SetPosition(0.0f, -20.0f, -50.0f); // 텍스트 위치 설정
	pStartText->SetObjectType(OBJ_TEXT);
	pStartText->SetMesh(m_vpTextMeshes[1]);
	pStartText->SetShader(m_pShader);
	pStartText->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
	pStartText->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f)); // 텍스트 색상 설정
	AddObject(pStartText); // 씬에 텍스트 객체 추가


}

void TitleScene::UpdateCamera(float fElapsedTime)
{
	if (!m_pCamera) return;

	m_pCamera->SetPosition(0.0f, 0.0f, -150.0f);
	m_pCamera->Update();
}
