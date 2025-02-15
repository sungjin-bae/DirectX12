#include "ObjectBase.h"

ObjectBase::ObjectBase(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale)
    : m_position(in_pos), m_rotation(in_rot), m_scale(m_scale)
{ }

ObjectBase::~ObjectBase() { }