#pragma once
#include "GameObject.h"

class CInstancebuffer;


struct Particle
{
	XMFLOAT3 position; // 입자의 위치
	XMFLOAT3 velocity; // 입자의 속도
	float lifeTime;   // 입자의 생존 시간
	COLORREF color;     // 입자의 색상
};


class CParticle :
    public CGameObject
{
private:
	std::vector<Particle> m_particles;
public:
	CParticle();
	virtual ~CParticle();

	void Explode(XMFLOAT3 startPos, int count, COLORREF color, float lT = 2.0f);

	virtual void Update() override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override {};

	virtual void AddToInstanceBuffer(CInstancebuffer& buffer) override;

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override {};



};

