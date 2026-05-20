#include "stdafx.h"
#include "Enemy.h"
#include "Particle.h"

CEnemy::CEnemy()
	: speed(rand() % 100 / 100.0f + 0.5f), rotationSpeed(0.1f), Axis(XMFLOAT3(rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f))
{
   
}

CEnemy::~CEnemy()
{
}

void CEnemy::Update()
{
    if (health <= 0)
    {
        SetDestroyed(true); // 체력이 0 이하가 되면 적을 파괴 상태로 설정합니다.
        CParticle* pExplosion = new CParticle();
        pExplosion->Explode(GetPosition(), 20, RGB(255, 0, 0)); // 폭발 효과 생성 (위치, 입자 수, 색상)
        SCENE_MANAGER->GetCurrentScene()->AddObject(pExplosion); // 폭발 효과를 현재 씬에 추가
        return; // 더 이상 업데이트할 필요가 없으므로 함수를 종료합니다.
    }
    CGameObject::Update();


    Rotate(&Axis, rotationSpeed); // 적이 회전하도록 설정 (예시)

    if (!isSpawned)
    {
        
        isSpawned = true;
    }

    XMFLOAT3 playerPosition = SCENE_MANAGER->GetCurrentScene()->GetPlayer()->GetPosition(); // 플레이어 위치 가져오기
    XMFLOAT3 enemyPosition = GetPosition(); // 적의 현재 위치를 가져옵니다.
    directionToPlayer = Vector3::Subtract(playerPosition, enemyPosition);
    directionToPlayer = Vector3::Normalize(directionToPlayer);
   
	// 적이 플레이어를 향해 이동하도록 설정
	// 보간을 사용하여 부드러운 이동을 구현할 수 있습니다.
	XMVECTOR vCurrentPos = XMLoadFloat3(&enemyPosition);
	XMVECTOR vTargetPos = XMLoadFloat3(&playerPosition);
	float fTimeElapsed = TIMER->GetTimeElapsed();
	float t = speed * fTimeElapsed; 
	if (t > 1.0f) t = 1.0f; 
	XMVECTOR vNewPos = XMVectorLerp(vCurrentPos, vTargetPos, t); 
	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, vNewPos);
	SetPosition(newPos);

}

void CEnemy::StartCollision(CGameObject* pOther)
{
    if (pOther->GetObjectType() == OBJ_PLAYER) // 충돌한 객체가 플레이어인 경우
    {
        SetDestroyed(true); // 체력이 0 이하가 되면 적을 파괴 상태로 설정합니다.
        CParticle* pExplosion = new CParticle();
		XMFLOAT3 position = GetPosition();
        position.z += 5.0f;
        pExplosion->Explode(position, 100, RGB(255, 0, 0)); // 폭발 효과 생성 (위치, 입자 수, 색상)
        SCENE_MANAGER->GetCurrentScene()->AddObject(pExplosion); // 폭발 효과를 현재 씬에 추가
	}
    if (pOther->GetObjectType() == OBJ_BULLET) // 충돌한 객체가 총알인 경우
    {
        health -= 1; // 체력을 감소시킵니다. (예: 총알 한 발당 1의 피해)
		SetColor(RGB(255, 0, 0)); // 적의 색상을 빨간색으로 변경하여 피해를 입었음을 시각적으로 표시 (예시)
        
	}
}
