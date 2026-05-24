// Shaders.hlsl
// [Phase 4 신규 파일]
// [Phase 9.1 추가] VSInstanced            : 인스턴스 렌더링
// 기존 CGraphicsPipeline의 CPU 측 변환(Project/ScreenTransform)을 GPU 셰이더로 이전.
//
// 상수 버퍼 슬롯:
//   b0 = 게임 오브젝트별 World 행렬 (32BitConstants 16개 = 64바이트)
//   b1 = 카메라별 View + Projection 행렬 (32BitConstants 32개 = 128바이트)
//

// ─────────────────────────────────────────────────────────
// 상수 버퍼 ? 게임 오브젝트 정보 (b0)
// ─────────────────────────────────────────────────────────


cbuffer cbGameObjectInfo : register(b0)
{
    matrix gmtxWorld;
    float4 gObjectColor; 
};

cbuffer cbCameraInfo : register(b1)
{
    matrix gmtxView;
    matrix gmtxProjection;
};

cbuffer cbLightInfo : register(b2)
{
    float3 gLightDir;
    float gAmbient;
    float3 gLightColor;
    float gPad;
}

//struct cbGameObjectInfo
//{
//    matrix mtxWorld;
//    float4 gObjectColor;
//};

//struct cbCameraInfo
//{
//    matrix gmtxView;
//    matrix gmtxProjection;
//};

//ConstantBuffer<cbGameObjectInfo> gcbGameObject : register(b0);
//ConstantBuffer<cbCameraInfo> gcbCamera : register(b1);


// ─────────────────────────────────────────────────────────
// 정점 셰이더 입출력 구조체
// ─────────────────────────────────────────────────────────
struct VS_INPUT
{
    float3 position : POSITION; // CDiffusedVertex의 m_xmf3Position
    float4 color : COLOR;       // CDiffusedVertex의 m_xmf4Diffuse
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normalW : NORMAL;
};

// ─────────────────────────────────────────────────────────
// 정점 셰이더 ? VSDiffused
// 기존 CGraphicsPipeline::Project()의 GPU 대응:
//   ModelPos → World → View → Projection (원근 나눗셈은 래스터라이저가 자동 수행)
// ─────────────────────────────────────────────────────────
VS_OUTPUT VSDiffused(VS_INPUT input)
{
    VS_OUTPUT output;
    
    output.position = mul(mul(
            mul(float4(input.position, 1.0f), gmtxWorld),
            gmtxView), gmtxProjection);
    output.color = input.color * gObjectColor;
    output.normalW = mul(float4(input.normal, 0.0f), gmtxWorld).xyz;
    return output;
}

// 인스터싱 정점 데이터
// 입력:
//   슬롯 0 (per-vertex)   : Position + Color  (메시 정점 데이터, 모든 인스턴스 공유)
//   슬롯 1 (per-instance) : World 행렬(4×float4) + 색상(float4)  (인스턴스마다 다름)

struct VS_INPUT_INSTANCED
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    
    float4 instWorldR0 : INSTWORLD0;
    float4 instWorldR1 : INSTWORLD1;
    float4 instWorldR2 : INSTWORLD2;
    float4 instWorldR3 : INSTWORLD3;
    float4 instColor : INSTCOLOR;
};


VS_OUTPUT VSInstanced(VS_INPUT_INSTANCED input)
{
    VS_OUTPUT output;
    
    row_major float4x4 instWorld = float4x4(
        input.instWorldR0,
        input.instWorldR1,
        input.instWorldR2,
        input.instWorldR3);
    
    
    float4 modelPos = float4(input.position, 1.0f);
    float4 worldPos = mul(modelPos, instWorld);
    
    output.position = mul(mul(worldPos, gmtxView), gmtxProjection);
    
    output.color = input.color * input.instColor;
    
    //output.color *= gObjectColor; // 인스턴스 색상과 게임 오브젝트 색상 모두 적용
    output.normalW = mul(float4(input.normal, 0.0f), instWorld).xyz;
    return output;

}


// ─────────────────────────────────────────────────────────
// 픽셀 셰이더 ? PSDiffused
// ─────────────────────────────────────────────────────────
float4 PSDiffused(VS_OUTPUT input) : SV_TARGET
{
    float3 N = normalize(input.normalW);
    float3 L = normalize(-gLightDir); // 빛이 오는 방향
    
    float NdotL = max(dot(N, L), 0.0f);
    float3 diffuse = gLightColor * NdotL;
    float3 ambient = float3(gAmbient, gAmbient, gAmbient);
    float3 lit = saturate(diffuse + ambient); // [0,1] 클램프
    
    return float4(input.color.rgb * lit, input.color.a);
}
    

struct VS_INPUT_2D
{
    float2 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT_2D
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT_2D VSCrosshair(VS_INPUT_2D input)
{
    VS_OUTPUT_2D output;
    output.position = float4(input.position, 0.0f, 1.0f);
    
    output.color = input.color;
    return output;
}

float4 PSCrosshair(VS_OUTPUT_2D input) : SV_TARGET
{
    return input.color;
}