#ifndef MYDIRECT_SOURCE_RENDER_OBJECT_OBJECT_H
#define MYDIRECT_SOURCE_RENDER_OBJECT_OBJECT_H

#include <DirectXMath.h>

class RenderObject
{
public:
    RenderObject();
    virtual ~RenderObject();

    virtual void Update();
    virtual void Render();

    long GetObjID();

protected:
    DirectX::XMFLOAT4X4 m_world;
    DirectX::XMFLOAT4X4 m_view;
    DirectX::XMFLOAT4X4 m_proj;

    long m_obj_id = 0;
};

#endif  // MYDIRECT_SOURCE_RENDER_OBJECT_OBJECT_H



