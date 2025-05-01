#pragma once

#include <DirectXMath.h>

#include "RendererType.h"
#include "../Common/ObjectBase.h"
#include "../Common/d3dUtil.h"

struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

class RenderObject;
typedef std::shared_ptr<RenderObject> RenderObjSharedPtr;

class RenderObject : public ObjectBase
{
public:
    RenderObject(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale);
    virtual ~RenderObject();

    virtual void Update();
    virtual void Render();
    virtual void OnChanged() override;

    void SetRendererType(ERendererType in_type) { m_renderer_type = in_type; }
    ERendererType GetRendererType() const { return m_renderer_type; }

    long GetObjID() const { return m_obj_id; }

    // PSO 관련 함수
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetPSODesc() const { return m_pso_desc; }

private:
    // 셰이더와 입력 레이아웃을 위한 정적 멤버 변수
    static Microsoft::WRL::ComPtr<ID3DBlob> m_vs_byte_code;
    static Microsoft::WRL::ComPtr<ID3DBlob> m_ps_byte_code;
    static std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_layout;

    // 정점과 인덱스 데이터를 위한 상수
    static constexpr size_t NUM_VERTICES = 8;
    static constexpr size_t NUM_INDICES = 36;

    // 지오메트리 생성 함수
    void CreateBoxGeometry();

    // PSO 설정 함수
    void BuildPSODesc();

    long m_obj_id;
    ERendererType m_renderer_type = ERendererType::OBJECT3D;

    // 변환 행렬들
    DirectX::XMMATRIX m_world_matrix;
    DirectX::XMMATRIX m_scale_matrix;
    DirectX::XMMATRIX m_rotation_matrix;
    DirectX::XMMATRIX m_translation_matrix;

    bool m_need_update;

    // 메시 지오메트리
    std::unique_ptr<MeshGeometry> m_box_geometry;

    // PSO 설정
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_pso_desc;
};
