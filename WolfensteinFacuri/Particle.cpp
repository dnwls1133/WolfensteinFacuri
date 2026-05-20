#include "stdafx.h"
#include "Particle.h"
#include "Scene.h"
#include "Instancebuffer.h"

CParticle::CParticle()
{
	SetObjectType(OBJ_EFFECT);

	if (SCENE_MANAGER && SCENE_MANAGER->GetCurrentScene())
	{
		SetMesh(SCENE_MANAGER->GetCurrentScene()->GetParticleMesh());
	}
}

CParticle::~CParticle()
{
}

void CParticle::Explode(XMFLOAT3 startPos, int count, COLORREF color, float lT )
{
	SetPosition(startPos);

	for (int i = 0; i < count; i++)
	{
		Particle particle;
		particle.position = startPos;
		particle.velocity = XMFLOAT3(
			(rand() % 100 - 50) / 10.0f,
			(rand() % 100 - 50) / 10.0f,
			(rand() % 100 - 50) / 10.0f
		);
		particle.lifeTime = lT; // 입자의 생존 시간 (초)
		particle.color = color;
		m_particles.push_back(particle);
	}
}

void CParticle::Update()
{
	CGameObject::Update(); // 부모 클래스의 Update()를 호출하여 월드 행렬 갱신
	float deltaTime = TIMER->GetTimeElapsed();

	for (int i = 0; i < (int)m_particles.size();) {
		m_particles[i].lifeTime -= deltaTime;
		if (m_particles[i].lifeTime > 0)
		{
			m_particles[i].position = Vector3::Add(m_particles[i].position, m_particles[i].velocity, deltaTime);
			++i;
		}
		else
		{
			m_particles[i] = m_particles.back();
			m_particles.pop_back();
		}
	}

	if(m_particles.empty())
	{
		SetDestroyed(true); // 모든 입자가 사라지면 이 파티클 객체도 제거 대상으로 표시
	}
}

// ═════════════════════════════════════════════════════════
// [Phase 9.2 추가] 인스턴스 버퍼에 자기 파티클들 추가
//
// 각 파티클을 하나의 인스턴스로 등록:
//   - World 행렬: Scale(0.3) × Translate(position)
//   - 색상      : COLORREF → XMFLOAT4 변환
//
// 호출자(CScene::Render)가 모든 파티클 객체에 대해 이 함수를 호출하여
// 인스턴스 버퍼를 채운 뒤 단 한 번의 DrawIndexedInstanced로 일괄 렌더.
//
// row_major 매트릭스 처리:
//   인스턴스드 셰이더는 row-major로 받음 → CPU에서 Transpose 불필요
//   XMStoreFloat4x4로 그대로 저장
// ═════════════════════════════════════════════════════════

void CParticle::AddToInstanceBuffer(CInstancebuffer& buffer)
{
	if (m_particles.empty()) return; 
	for (const auto& particle : m_particles)
	{
		XMMATRIX mtxSclae = XMMatrixScaling(0.3f, 0.3f, 0.3f);
		XMMATRIX mtxTranslate = XMMatrixTranslation(particle.position.x, particle.position.y, particle.position.z);
		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, XMMatrixMultiply(mtxSclae, mtxTranslate));

		XMFLOAT4 color(
			GetRValue(particle.color) / 255.0f,
			GetGValue(particle.color) / 255.0f,
			GetBValue(particle.color) / 255.0f,
			1.0f);

		buffer.AddInstance(world, color);
	}
}