#pragma once
#include "Camera.h"


class CGraphicsPipeline
{
private:
	static XMFLOAT4X4* m_pxmf4x4World; // 월드 변환 행렬의 포인터
	static XMFLOAT4X4* m_pxmf4x4ViewProject; // 뷰프로젝션 변환 행렬의 포인터
	static CViewport* m_pViewport; // 뷰포트의 포인터

public:
	static void SetWorldTransform(XMFLOAT4X4* pxmf4x4World) { m_pxmf4x4World = pxmf4x4World; }
	static void SetViewProjectMatrixPointer(XMFLOAT4X4* pxmf4x4ViewProject) { m_pxmf4x4ViewProject = pxmf4x4ViewProject; }
	static void SetViewportPointer(CViewport* pViewport) { m_pViewport = pViewport; }

	// 3D 로컬 정점 -> 2D 화면 좌표로 변환하는 함수
	static XMFLOAT3 Transform(XMFLOAT3& xmf3Model);

	static XMFLOAT3 Project(XMFLOAT3& xmf3Model);
	static XMFLOAT3 ScreenTransform(XMFLOAT3& xmf3Project);
private:
	
	
};

