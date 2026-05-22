#include "stdafx.h"
#include "Player.h"
#include "Mesh.h"
#include "Scene.h"
#include "Particle.h"
#include "AIrcraft.h"

CPlayer::CPlayer()
{
	
}

CPlayer::~CPlayer()
{
	//if (m_pCamera) delete m_pCamera;
	
}

void CPlayer::Move(DWORD dwDirection, float fDistance)
{
	if (!m_pCamera) return;
	
   	XMFLOAT3 xmf3Shift = XMFLOAT3(0.0f, 0.0f, 0.0f);

	
	switch (dwDirection)
	{
		case DIR_FORWARD:
		{
			// ���� ���Ϳ� �Ÿ� ������ ���� �� ���ϱ�
			XMFLOAT3 look{ m_pCamera->GetLookVector() };
			XMFLOAT3 scaledLook = Vector3::ScalarProduct(look, fDistance, true);
			xmf3Shift = Vector3::Add(xmf3Shift, scaledLook);
		}
		break;
		case DIR_BACKWARD:
		{
			// ���� ���Ϳ� �Ÿ� ������ ���� �� ����
			XMFLOAT3 look{ m_pCamera->GetLookVector() };
			XMFLOAT3 backward = Vector3::ScalarProduct(look, -fDistance, true);
			xmf3Shift = Vector3::Add(xmf3Shift, backward);
		}
			break;
		case DIR_LEFT:
		{
			// ���� ���Ϳ� �Ÿ� ������ ���� �� �������� �̵�
			XMFLOAT3 right{ m_pCamera->GetRightVector() };
			XMFLOAT3 left = Vector3::ScalarProduct(right, -fDistance, true);
			xmf3Shift = Vector3::Add(xmf3Shift, left);
		}
			
			break;
		case DIR_RIGHT:
		{
			// ���� ���Ϳ� �Ÿ� ������ ���� �� ���������� �̵�
			XMFLOAT3 right{ m_pCamera->GetRightVector() };
			XMFLOAT3 rightShift = Vector3::ScalarProduct(right, fDistance, true);
			xmf3Shift = Vector3::Add(xmf3Shift, rightShift);
		}
			break;
	}
	CGameObject::Move(xmf3Shift, 1.0f); // CGameObject�� Move �Լ� ȣ���Ͽ� ��ġ ������Ʈ
	
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
	if(m_pCamera)
	{
		//m_pCamera->Rotate(fPitch, fYaw, fRoll); // 카메라 ȸ��
	}

	XMFLOAT3 axisX = Vector3::XAxis(); // (1,0,0)
	XMFLOAT3 axisY = Vector3::YAxis(); // (0,1,0)
	XMFLOAT3 axisZ = Vector3::ZAxis(); // (0,0,1)

	CGameObject::Rotate(&axisX, fPitch);
	CGameObject::Rotate(&axisY, fYaw);
	CGameObject::Rotate(&axisZ, fRoll); 
}

//
//void CPlayer::FireMissile()
//{
//	CMissile* pMissile = new CMissile();
//	pMissile->SetPosition(GetPosition());
//	pMissile->SetObjectType(OBJ_BULLET); 
//	
//	XMFLOAT3 look = GetDirection(); // �÷��̾��� ���� ���� ���͸� �����ɴϴ�.
//	XMFLOAT3 missileDirection = Vector3::Normalize(look); // ���� ���͸� ����ȭ�Ͽ� ���� ���ͷ� ����ϴ�.
//	pMissile->setDirection(missileDirection); // �̻����� ���� ����
//
//	pMissile->SetMesh(SCENE_MANAGER->GetCurrentScene()->GetMissileMesh()); // �̻����� �޽� ���� (��: ���� ť��)
//	pMissile->SetColor(RGB(255, 255, 0)); // �̻����� ���� ���� (��: �����)
//
//	// �̻����� ���� ���忡 �߰�
//	SCENE_MANAGER->GetCurrentScene()->AddObject(pMissile); 
//
//
//
//}

void CPlayer::Update()
{
	//if (m_fPlayTime >= 60.0f)
	//{
	//	if (!m_playerWon)
	//	{
	//		CParticle* pWinEffect1 = new CParticle();
	//		CParticle* pWinEffect2 = new CParticle();
	//		CParticle* pWinEffect3 = new CParticle();
	//		CParticle* pWinEffect4 = new CParticle();
	//		XMFLOAT3 position = GetPosition();
	//		position.z += 5.0f;
	//		position.y += 2.0f;
	//		position.x += 2.0f;
	//		pWinEffect1->Explode(position, 100, RGB(0, 255, 0)); 
	//		position.z += 10.0f;
	//		pWinEffect3->Explode(position, 100, RGB(0, 255, 0)); 
	//		position.z -= 5.0f;
	//		position.x -= 4.0f;
	//		pWinEffect2->Explode(position, 100, RGB(0, 255, 0)); 
	//		pWinEffect4->Explode(position, 100, RGB(0, 255, 0)); 
	//		SCENE_MANAGER->GetCurrentScene()->AddObject(pWinEffect1);
	//		SCENE_MANAGER->GetCurrentScene()->AddObject(pWinEffect2);
	//		SCENE_MANAGER->GetCurrentScene()->AddObject(pWinEffect3);
	//		SCENE_MANAGER->GetCurrentScene()->AddObject(pWinEffect4);
	//		
	//		
	//		
	//		
	//	}
	//	

	//	
	//	m_playerWon = true;
	//}
	//if(m_nHealth <= 0 )
	//{
	//	if (!m_playerDestroyed) // �÷��̾ ���� �ı����� ���� ��쿡�� ���� ȿ�� ����
	//	{
	//		CParticle* pExplosion = new CParticle();
	//		XMFLOAT3 position = GetPosition();
	//		position.z += 5.0f;
	//		pExplosion->Explode(position, 100, RGB(255, 0, 0)); // ���� ȿ�� ���� (��ġ, ���� ��, ����)
	//		SCENE_MANAGER->GetCurrentScene()->AddObject(pExplosion); // ���� ȿ���� ���� ���� �߰�
	//	}
	//	
	//	m_playerDestroyed = true; // ü���� 0 ���ϰ� �Ǹ� �÷��̾ �ı� ���·� �����մϴ�.
	//	return; // �� �̻� ������Ʈ�� �ʿ䰡 �����Ƿ� �Լ��� �����մϴ�.
	//}
	
	CGameObject::Update(); // �÷��̾��� ���� ��� ����

	//XMFLOAT4 currentRotation = GetRotation(); // �÷��̾��� ȸ�� ���� ��������
	//XMFLOAT4 targetRotation = {0.0f, currentRotation.y, 0.0f, currentRotation.w}; // ��ǥ ȸ�� (��ġ�� ���� 0���� ����)
	//float fRotationSpeed = 5.0f; 
	//XMVECTOR vCurrent = XMLoadFloat4(&currentRotation);
	//XMVECTOR vTarget = XMLoadFloat4(&targetRotation);
	//float t0 = fRotationSpeed * TIMER->GetTimeElapsed(); 
	//if (t0 > 1.0f) t0 = 1.0f;
	//XMVECTOR vLerp = XMQuaternionSlerp(vCurrent, vTarget, t0); 
	//XMFLOAT4 lerpRotation;
	//XMStoreFloat4(&lerpRotation, vLerp);
	//SetRotation(lerpRotation); // �÷��̾��� ȸ���� ������ ȸ������ ������Ʈ

	
	
	m_fPlayTime += TIMER->GetTimeElapsed(); // �÷��̾ ����ִ� �ð� ����
	
	
}

