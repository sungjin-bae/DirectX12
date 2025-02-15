#ifndef MYDIRECT_SOURCE_COMMON_OBJECT_BASE_H
#define MYDIRECT_SOURCE_COMMON_OBJECT_BASE_H

#include "../Common/d3dUtil.h"

using namespace DirectX;

class ObjectBase
{
public:
    ObjectBase(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale);
    virtual ~ObjectBase();

protected:
    XMFLOAT3 m_position;
    XMFLOAT3 m_rotation;
    XMFLOAT3 m_scale;

};

#endif  // MYDIRECT_SOURCE_COMMON_OBJECT_BASE_H
