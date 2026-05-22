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
	OBJ_TEXT,
	OBJ_MONSTER,
	OBJ_CHOICE,
	NONE
};

class CGameObject
{
public:
	static UINT g_NextObjectID; // ������ �����Ǵ� ��ü���� �ο��� ���� ID
	UINT m_nObjectID; // ��ü�� ���� ID
	CGameObject();
	virtual ~CGameObject();

private:
	CMesh* m_pMesh{ NULL };  // ���� ����� �޽��� ������
	CShader* m_pShader{ NULL }; // [�߰�] ������ ���̴�


	DWORD m_dwColor;		 // ��ü�� ���� (ARGB)
	XMFLOAT4 m_xmf4Color; 

	ObjectType m_ObjectType;  // ��ü�� Ÿ��
	bool m_IsActive = true;
	bool m_IsDestroyed = false;

protected:
	XMFLOAT3	m_xmf3Position; // ��ü�� ��ġ
	XMFLOAT4    m_xmf4Rotation; // ���ʹϾ� (x,y,z,w)
	XMFLOAT4X4  m_xmf4x4World;    // ���� ��ȯ ���

	// [D3D12 �߰�] Root CBV�� Upload Heap ����
	ID3D12Resource* m_pd3dcbGameObject = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;

	// [추가] 디버그 OOBB 와이어프레임 전용 CB (일반 패스 CB와 분리하여 동기화 해저드 방지)
	ID3D12Resource* m_pd3dcbDebug = NULL;
	CB_GAMEOBJECT_INFO* m_pcbMappedDebug = NULL;

public:

	BoundingOrientedBox m_xmOOBB; // ��ü�� OOBB (Oriented Bounding Box)

	void GenerateBoundingBox();

	// �޽� / ���̴� ����
	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);


	void SetColor(DWORD dwColor) { 
		m_dwColor = dwColor;
		// COLORREF (0x00BBGGRR) �� XMFLOAT4 ��ȯ
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
		// ȸ�� ���ʹϾ𿡼� Look ���͸� ���ϴ� ���
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


	//[����] Render �ñ״�ó: HDC -> ID3D12GraphicsCommandList*
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	// [추가] OOBB 와이어프레임 디버그 렌더 (m_xmOOBB → World 행렬, 와이어 PSO + 단위 큐브 메쉬 재사용)
	void RenderDebugBox(ID3D12GraphicsCommandList* pd3dCommandList,
		CMesh* pWireMesh, const XMFLOAT4& xmf4Color);

	// [�߰�] ���̴� ��� ���� ���� (���� ����� b0 ���Կ� ���ε�)
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void AddToInstanceBuffer(CInstancebuffer& buffer) {}

	virtual void StartCollision(CGameObject* pOther) { /* �浹 ���� �� �⺻ ������ �ƹ��͵� ���� ���� */ }
	virtual void OnCollision(CGameObject* pOther) { /* �浹 �� �⺻ ������ �ƹ��͵� ���� ���� */ }
	virtual void EndCollision(CGameObject* pOther) { /* �浹 ���� �� �⺻ ������ �ƹ��͵� ���� ���� */ }
};

