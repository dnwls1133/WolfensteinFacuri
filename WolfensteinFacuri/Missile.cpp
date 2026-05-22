#include "stdafx.h"
#include "Missile.h"
#include "Particle.h"


void CMissile::Update()
{
	Animate(TIMER->GetTimeElapsed());
}

void CMissile::Animate(float fElapsedTime)
{
	CGameObject::Update(); // 월드 행렬 갱신
	XMFLOAT3 ZAxis = Vector3::ZAxis(); // Z축을 기준으로 회전하도록 설정
	Rotate(&ZAxis, fElapsedTime * speed); // 미사일이 회전하도록 설정 (예시)
	// 1. 미사일 이동
	Move(dir, speed);
	// 2. 생존 시간 감소
	elapsedTime += fElapsedTime;
	if (elapsedTime >= lifeTime)
	{
		SetDestroyed(true); // 생존 시간이 다 되면 미사일을 파괴 상태로 설정
	}
	
}

void CMissile::StartCollision(CGameObject* pOther)
{
	
	// 적과 충돌 시 폭발 효과 생성
	CParticle* pExplosion = new CParticle();
	pExplosion->Explode(GetPosition(), 100, RGB(255, 255, 0)); // 폭발 효과 생성 (위치, 입자 수, 색상)
	SCENE_MANAGER->GetCurrentScene()->AddObject(pExplosion); // 폭발 효과를 현재 씬에 추가
		
	
	
}
