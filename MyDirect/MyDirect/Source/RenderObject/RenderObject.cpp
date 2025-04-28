#include "../Common/MathHelper.h"

#include "RenderObject.h"

RenderObject::RenderObject(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale)
    : ObjectBase(in_pos, in_rot, in_rot)
    , m_world_matrix(DirectX::XMMatrixIdentity())
    , m_scale_matrix(DirectX::XMMatrixIdentity())
    , m_rotation_matrix(DirectX::XMMatrixIdentity())
    , m_translation_matrix(DirectX::XMMatrixIdentity())
    , m_need_update(true)
{
    static long id = 1;

    m_obj_id = id++;
}

RenderObject::~RenderObject()
{

}

void RenderObject::Update()
{
    if (m_need_update == false)
        return;

    // 스케일 변환
    m_scale_matrix = DirectX::XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    
    // 회전 변환 (라디안으로 변환)
    m_rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(m_rotation.x),
        DirectX::XMConvertToRadians(m_rotation.y),
        DirectX::XMConvertToRadians(m_rotation.z)
    );
    
    // 위치 변환
    m_translation_matrix = DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    
    // 변환 행렬들을 결합 (스케일 -> 회전 -> 이동 순서)
    m_world_matrix = m_scale_matrix * m_rotation_matrix * m_translation_matrix;
}

void RenderObject::Render() 
{

}


void RenderObject::OnChanged()
{
    m_need_update = true;
}