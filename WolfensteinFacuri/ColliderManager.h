#pragma once

class CColliderManager
{

private:
	// 충돌 그룹을 저장하는 맵 (ObjectType을 키로 사용)
	std::unordered_map<int, std::vector<CGameObject*>> m_mapCollisionGroups;
	// 2. 충돌해야 할 "타입 쌍(Pair)"을 등록해두는 Set (충돌 매트릭스)
	std::set<std::pair<int, int>> m_setCollisionMatrix;
	// "이전 프레임"에 부딪히고 있었던 충돌 쌍들의 모임
	std::unordered_set<ULONGLONG> m_setPreviousCollisions; // ULONGLONG을 사용하여 객체 포인터 두 개를 하나의 숫자로 합쳐서 저장

	ULONGLONG GetCollisionKey(CGameObject* pObjA, CGameObject* pObjB)
	{
		// 객체 포인터 두 개를 하나의 ULONGLONG으로 합치는 방법 (예시)
		ULONGLONG keyA = pObjA->m_nObjectID;
		ULONGLONG keyB = pObjB->m_nObjectID;

		// A가 B를 치든, B가 A를 치든 같은 키가 나오도록 정렬하여 합칩니다.
		if (keyA < keyB) return (keyA << 32) | (keyB & 0xFFFFFFFF);
		else return (keyB << 32) | (keyA & 0xFFFFFFFF);

	}

	CGameObject* FindGameObject(UINT nID); // ID로 객체를 찾는 함수 (충돌 처리에서 필요할 수 있음)


public:
	void clearCollisionGroups(); // 모든 충돌 그룹을 초기화하는 함수
	void AddToCollisionGroup(CGameObject* pObject); // 객체를 충돌 그룹에 추가하는 함수
	void InitCollisionMatrix(); // 충돌 규칙을 세팅하는 함수
	void CheckAllCollisions();  // 매 프레임 자동으로 충돌을 검사하는 함수
	void CheckCollisionGroup(std::vector<CGameObject*>& groupA, std::vector<CGameObject*>& groupB, std::unordered_set<ULONGLONG>& setCurrentCollisions);

};

