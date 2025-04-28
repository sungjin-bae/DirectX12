#pragma once

#include "../Common/d3dUtil.h"

using namespace DirectX;

class ObjectBase
{
public:
    ObjectBase(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale);
    virtual ~ObjectBase();

    // 위치 설정
    void SetPosition(XMFLOAT3 in_pos);
    void SetPosition(float in_x, float in_y, float in_z);
    
    // 회전 설정
    void SetRotation(XMFLOAT3 in_rot);
    void SetRotation(float in_x, float in_y, float in_z);
    
    // 스케일 설정
    void SetScale(XMFLOAT3 in_scale);
    void SetScale(float in_x, float in_y, float in_z);

protected:
    virtual void OnChanged() {}

protected:
    XMFLOAT3 m_position;
    XMFLOAT3 m_rotation;
    XMFLOAT3 m_scale;

};
