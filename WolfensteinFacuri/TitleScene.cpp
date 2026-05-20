#include "stdafx.h"
#include "Text.h"
#include "TitleScene.h"

static const std::array<int, 11> textIndices = { 22,14,11,5,4,13,18,19,4,8,13 };

TitleScene::TitleScene(CColliderManager* pCollider, CCamera* pCamera)
	: CScene(pCollider, pCamera)
{
}

void TitleScene::Animate(float fElapsedTime)
{
	CScene::Animate(fElapsedTime);
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

	for (int i = 0; i < 11; ++i)
	{
		CText* pText = new CText();
		pText->SetPosition(-50.0f + i * 10.0f, 20.0f, -50.0f); // 텍스트 위치 설정
		pText->SetMesh(m_vpTextMeshes[textIndices[i]]);
		pText->SetShader(m_pShader);
		pText->CreateShaderVariables(m_pd3dDevice, m_pd3dCommandList);
		pText->SetColor(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.f)); // 텍스트 색상 설정
		AddObject(pText); // 씬에 텍스트 객체 추가
	}


}

void TitleScene::UpdateCamera(float fElapsedTime)
{
	if (!m_pCamera) return;

	m_pCamera->SetPosition(0.0f, 0.0f, -100.0f);
	m_pCamera->Update();
}
