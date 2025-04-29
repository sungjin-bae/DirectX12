#include "../Common/MathHelper.h"

#include "RenderObject.h"

RenderObject::RenderObject(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale)
    : ObjectBase(in_pos, in_rot, in_rot)
    , m_world_matrix(DirectX::XMMatrixIdentity())
    , m_scale_matrix(DirectX::XMMatrixIdentity())
    , m_rotation_matrix(DirectX::XMMatrixIdentity())
    , m_translation_matrix(DirectX::XMMatrixIdentity())
    , m_need_update(true)
    , m_vertices({
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
        Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
        Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
        Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
        Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
    })
    , m_indices({
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    })
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