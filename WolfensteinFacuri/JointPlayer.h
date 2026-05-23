#pragma once
#include "Player.h"

class CJointPartMesh;

class CJointPlayer :
    public CPlayer
{
public:
    float m_fFireCooldown = 0.1f;
    float m_fLastFireTime = 0.0f;
public:
    
    CJointPlayer() = default;
	virtual ~CJointPlayer();

    void BuildParts(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList
        , CShader* pShader );

	virtual void Update() override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    bool m_bIsMoving = false;

    void Jump();

    void FireMissile(); // 미사일 발사 함수


    bool IsOnGround() const { return m_bOnGround; }

	XMFLOAT3 GetMuzzlePosition() const { return m_xmf3MuzzlePos; }
	XMFLOAT3 GetMuzzleDirection() const { return m_xmf3MuzzleDir; }
private:
	void BuildOnePart(CGameObject*& rpPart, CJointPartMesh*& rpMesh,
		ID3D12Device* pd3dDevice,
		ID3D12GraphicsCommandList* pd3dCommandList,
		CShader* pShader,
		float fW, float fH, float fD, XMFLOAT4 color);

	void UpdatePartTransforms(float swing); // 각 파트의 월드 행렬을 업데이트하는 함수

    // ── 파트 메시 (소유) ──────────────────────────────
    CJointPartMesh* m_pBodyMesh = nullptr;
    CJointPartMesh* m_pHeadMesh = nullptr;
    CJointPartMesh* m_pLArmMesh = nullptr;
    CJointPartMesh* m_pRArmMesh = nullptr;
    CJointPartMesh* m_pLLegMesh = nullptr;
    CJointPartMesh* m_pRLegMesh = nullptr;
	CGunMesh* m_pGunMesh = nullptr;


    // ── 파트 오브젝트 (GPU 상수버퍼 보유) ─────────────
    CGameObject* m_pBody = nullptr;
    CGameObject* m_pHead = nullptr;
    CGameObject* m_pLeftArm = nullptr;
    CGameObject* m_pRightArm = nullptr;
    CGameObject* m_pLeftLeg = nullptr;
    CGameObject* m_pRightLeg = nullptr;
    CGameObject* m_pGun = nullptr;


    // ㅡㅡ 물리 ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ
    float m_fVelocity = 0.0f;
    bool  m_bOnGround = false;
    static constexpr float kGravity = -90.81f;
    static constexpr float kJumpSpeed = 10.0f;

    // ── 걷기 애니메이션 ──────────────────────────────
    float m_fWalkTime = 0.0f;
    float m_fWalkSpeed = 5.0f;   // 흔들기 속도
    float m_fLimbAngle = 0.4f;   // 최대 각도 (라디안, ≈23도)

	XMFLOAT3 m_xmf3MuzzlePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_xmf3MuzzleDir = { 0.0f, 0.0f, 1.0f };

};

