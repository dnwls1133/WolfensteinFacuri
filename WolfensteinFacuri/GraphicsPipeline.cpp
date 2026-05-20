#include "stdafx.h"
#include "GraphicsPipeline.h"

// static 멤버 변수 초기화
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4World = NULL;
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4ViewProject = NULL;
CViewport* CGraphicsPipeline::m_pViewport = NULL;

// 1. [투영 변환] 
XMFLOAT3 CGraphicsPipeline::Project(XMFLOAT3& xmf3Model)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Multiply(*m_pxmf4x4World, *m_pxmf4x4ViewProject);
	// TransformCoord 함수 사용 행렬곱셉 및 동촤 좌표(w) 나누기 수행
	XMFLOAT3 xmf3Project = Vector3::TransformCoord(xmf3Model, xmf4x4Transform);

	return xmf3Project;
}

// 2. 뷰포트 맵핑 단계 
XMFLOAT3 CGraphicsPipeline::ScreenTransform(XMFLOAT3& xmf3Project)
{
	XMFLOAT3 xmf3Screen;
	xmf3Screen.x = m_pViewport->m_nLeft + (xmf3Project.x + 1.0f) * 0.5f * m_pViewport->m_nWidth;
	xmf3Screen.y = m_pViewport->m_nTop + (-xmf3Project.y + 1.0f) * 0.5f * m_pViewport->m_nHeight;
	xmf3Screen.z = xmf3Project.z; 
	return xmf3Screen;
	
}

// 3. 최종변환 위 두단계를 한번에 실행하자
XMFLOAT3 CGraphicsPipeline::Transform(XMFLOAT3& xmf3Model)
{
	XMFLOAT3 xmf3Project = Project(xmf3Model);
	XMFLOAT3 xmf3Screen = ScreenTransform(xmf3Project);
	return xmf3Screen;

}