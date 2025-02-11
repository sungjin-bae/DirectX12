#include "../Common/MathHelper.h"

#include "RenderObject.h"

RenderObject::RenderObject() 
    : m_world(MathHelper::Identity4x4()) 
    , m_view(MathHelper::Identity4x4())
    , m_proj(MathHelper::Identity4x4())
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

long RenderObject::GetObjID()
{
    return m_obj_id;
}