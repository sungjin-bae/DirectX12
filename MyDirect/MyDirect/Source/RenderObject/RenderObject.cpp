#include "../Common/MathHelper.h"

#include "RenderObject.h"

RenderObject::RenderObject(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale)
    : ObjectBase(in_pos, in_rot, in_rot)
{
    static long id = 1;

    m_obj_id = id++;
}

RenderObject::~RenderObject()
{

}

void RenderObject::Update()
{

}

void RenderObject::Render() 
{

}
