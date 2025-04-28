#include "ObjectBase.h"

ObjectBase::ObjectBase(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale)
    : m_position(in_pos)
    , m_rotation(in_rot)
    , m_scale(in_scale)
{ }

ObjectBase::~ObjectBase() { }

// 위치 설정
void ObjectBase::SetPosition(XMFLOAT3 in_pos)
{
    m_position = in_pos;
}

void ObjectBase::SetPosition(float in_x, float in_y, float in_z)
{
    m_position = XMFLOAT3(in_x, in_y, in_z);
}

// 회전 설정
void ObjectBase::SetRotation(XMFLOAT3 in_rot)
{
    m_rotation = in_rot;
}

void ObjectBase::SetRotation(float in_x, float in_y, float in_z)
{
    m_rotation = XMFLOAT3(in_x, in_y, in_z);
}

// 스케일 설정
void ObjectBase::SetScale(XMFLOAT3 in_scale)
{
    m_scale = in_scale;
}

void ObjectBase::SetScale(float in_x, float in_y, float in_z)
{
    m_scale = XMFLOAT3(in_x, in_y, in_z);
}