#include "stdafx.h"
#include "GameObject.h"
#include "ColliderManager.h"













void CColliderManager::clearCollisionGroups()
{
	m_mapCollisionGroups.clear();
}




void CColliderManager::InitCollisionMatrix()
{
	m_setCollisionMatrix.insert({ OBJ_PLAYER, OBJ_MONSTER });
	m_setCollisionMatrix.insert({ OBJ_PLAYER, OBJ_ENEMY });
	m_setCollisionMatrix.insert({ OBJ_ENEMY, OBJ_BULLET });
	m_setCollisionMatrix.insert({ OBJ_PLAYER, OBJ_WALL });
}

void CColliderManager::CheckAllCollisions()
{
	std::unordered_set<ULONGLONG> setCurrentCollisions;

	for (const auto& pair : m_setCollisionMatrix)
	{
		int typeA = pair.first;
		int typeB = pair.second;

		CheckCollisionGroup(m_mapCollisionGroups[typeA], m_mapCollisionGroups[typeB], setCurrentCollisions);
	
	}

	for (const auto& key : m_setPreviousCollisions)
	{
		if (setCurrentCollisions.find(key) == setCurrentCollisions.end())
		{
			UINT keyA = (UINT)(key >> 32); // 상위 32비트에서 객체 A의 ID 추출
			UINT keyB = (UINT)(key & 0xFFFFFFFF); // 하위 32비트에서 객체 B의 ID 추출

			CGameObject* pObjA = FindGameObject(keyA);
			CGameObject* pObjB = FindGameObject(keyB);

			if (!pObjA || !pObjB) continue; // 객체가 이미 삭제된 경우 무시

			pObjA->EndCollision(pObjB);
			pObjB->EndCollision(pObjA);
		}
	}

	m_setPreviousCollisions = std::move(setCurrentCollisions);
}

void CColliderManager::CheckCollisionGroup(std::vector<CGameObject*>& groupA, std::vector<CGameObject*>& groupB, std::unordered_set<ULONGLONG>& setCurrentCollisions)
{
	for (auto& pObjA : groupA)
	{
		for (auto& pObjB : groupB)
		{
			if (pObjA == pObjB) continue; // 같은 객체끼리는 충돌 검사 안 함
			if(pObjA->m_xmOOBB.Intersects(pObjB->m_xmOOBB))
			{
				ULONGLONG key = GetCollisionKey(pObjA, pObjB);
				setCurrentCollisions.insert(key); // 이번 프레임에 충돌하는 쌍으로 추가
				if (m_setPreviousCollisions.find(key) == m_setPreviousCollisions.end())
				{
					pObjA->StartCollision(pObjB);
					pObjB->StartCollision(pObjA);
				}
				else
				{
					pObjA->OnCollision(pObjB);
					pObjB->OnCollision(pObjA);
				}
			}
		}
	}
}

void CColliderManager::AddToCollisionGroup(CGameObject* pObject)
{
	m_mapCollisionGroups[pObject->GetObjectType()].push_back(pObject);
}

CGameObject* CColliderManager::FindGameObject(UINT nID)
{
	for (auto& pair : m_mapCollisionGroups)
	{
		for (CGameObject* pObj : pair.second)
		{
			if (pObj && pObj->m_nObjectID == nID)
			{
				return pObj;
			}
		}
	}

	return nullptr;
}