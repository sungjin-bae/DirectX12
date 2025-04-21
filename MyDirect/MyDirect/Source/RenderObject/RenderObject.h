#ifndef MYDIRECT_SOURCE_RENDER_OBJECT_OBJECT_H
#define MYDIRECT_SOURCE_RENDER_OBJECT_OBJECT_H

#include <DirectXMath.h>

#include "RendererType.h"
#include "../Common/ObjectBase.h"

class RenderObject;
typedef std::shared_ptr<RenderObject> RenderObjSharedPtr;

class RenderObject : public ObjectBase
{
public:
    RenderObject(XMFLOAT3 in_pos, XMFLOAT3 in_rot, XMFLOAT3 in_scale);
    virtual ~RenderObject();

    virtual void Update();
    virtual void Render();

    long GetObjID() const { return const_cast<const long&>(m_obj_id); }
    auto GetRendererType() const -> const ERendererType { return const_cast<const ERendererType&>(m_renderer_type); }
protected:
    long m_obj_id = 0;

    ERendererType m_renderer_type = ERendererType::OBJECT3D;
};

#endif  // MYDIRECT_SOURCE_RENDER_OBJECT_OBJECT_H



