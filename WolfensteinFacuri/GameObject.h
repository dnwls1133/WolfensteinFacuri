#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"

class CInstancebuffer;	

enum ObjectType
{
	OBJ_PLAYER,
	OBJ_ENEMY,
	OBJ_BULLET,
	OBJ_SCENE,
	OBJ_FLOOR,
	OBJ_WALL,
	OBJ_ITEM,
	OBJ_EFFECT,
	OBJ_MONSTER,
	NONE
};

class CGameObject
{
public:
	static UINT g_NextObjectID; // 다음에 생성되는 객체에게 부여할 고유 ID
	UINT m_nObjectID; // 객체의 고유 ID
	CGameObject();
	virtual ~CGameObject();

private:
	CMesh* m_pMesh{ NULL };  // 내가 사용할 메쉬의 포인터
	CShader* m_pShader{ NULL }; // [추가] 렌더링 셰이더


	DWORD m_dwColor;		 // 객체의 색상 (ARGB)
	XMFLOAT4 m_xmf4Color; 

	ObjectType m_ObjectType;  // 객체의 타입
	bool m_IsActive = true;
	bool m_IsDestroyed = false;

protected:
	XMFLOAT3	m_xmf3Position; // 객체의 위치
	XMFLOAT4    m_xmf4Rotation; // 쿼터니언 (x,y,z,w)
	XMFLOAT4X4  m_xmf4x4World;    // 월드 변환 행렬

	// [D3D12 추가] Root CBV용 Upload Heap 버퍼
	ID3D12Resource* m_pd3dcbGameObject = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;

public:

	BoundingOrientedBox m_xmOOBB; // 객체의 OOBB (Oriented Bounding Box)

	void GenerateBoundingBox();

	// 메시 / 셰이더 설정
	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);


	void SetColor(DWORD dwColor) { 
		m_dwColor = dwColor;
		// COLORREF (0x00BBGGRR) → XMFLOAT4 변환
		m_xmf4Color = XMFLOAT4(
			GetRValue(dwColor) / 255.0f,
			GetGValue(dwColor) / 255.0f,
			GetBValue(dwColor) / 255.0f,
			1.0f);
	}
	void SetColor(XMFLOAT4 xmf4Color) { m_xmf4Color = xmf4Color; }
	void SetActive(bool isActive) { m_IsActive = isActive; }
	void SetDestroyed(bool isDestroyed) { m_IsDestroyed = isDestroyed; }
	bool IsActive() const { return m_IsActive; }
	bool IsDestroyed() const { return m_IsDestroyed; }

	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& xmf3Position) { m_xmf3Position = xmf3Position; }
	void SetObjectType(ObjectType type) { m_ObjectType = type; }
	void SetRotation(const XMFLOAT4& xmf4Rotation) { m_xmf4Rotation = xmf4Rotation; }
	XMFLOAT3 GetPosition() const { return m_xmf3Position; }
	XMFLOAT4 GetRotation() const { return m_xmf4Rotation; }
	DWORD GetColor() const { return m_dwColor; }
	XMFLOAT4 GetColorF() const { return m_xmf4Color; }
	XMFLOAT3 GetDirection() const
	{
		// 회전 쿼터니언에서 Look 벡터를 구하는 방법
		XMVECTOR qRotation = XMLoadFloat4(&m_xmf4Rotation);
		XMFLOAT3 zAxis = Vector3::ZAxis();
		XMVECTOR vLook = XMVector3Rotate(XMLoadFloat3(&zAxis), qRotation); 
		XMFLOAT3 xmf3Direction;
		XMStoreFloat3(&xmf3Direction, vLook);
		return xmf3Direction;
	}
	
	ObjectType GetObjectType() const { return m_ObjectType; }
	

	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Move(XMFLOAT3& dir, float distance);
	
	virtual void Update();


	//[변경] Render 시그니처: HDC -> ID3D12GraphicsCommandList*
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// [추가] 셰이더 상수 변수 갱신 (월드 행렬을 b0 슬롯에 업로드)
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void AddToInstanceBuffer(CInstancebuffer& buffer) {}

	virtual void StartCollision(CGameObject* pOther) { /* 충돌 시작 시 기본 동작은 아무것도 하지 않음 */ }
	virtual void OnCollision(CGameObject* pOther) { /* 충돌 시 기본 동작은 아무것도 하지 않음 */ }
	virtual void EndCollision(CGameObject* pOther) { /* 충돌 종료 시 기본 동작은 아무것도 하지 않음 */ }
};

