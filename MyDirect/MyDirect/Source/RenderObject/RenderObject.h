#pragma once

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
    virtual void OnChanged() override;

    void SetRendererType(ERendererType in_type) { m_renderer_type = in_type; }
    ERendererType GetRendererType() const { return m_renderer_type; }

    long GetObjID() const { return m_obj_id; }

private:
    long m_obj_id;
    ERendererType m_renderer_type = ERendererType::OBJECT3D;

    // 변환 행렬들
    DirectX::XMMATRIX m_world_matrix;
    DirectX::XMMATRIX m_scale_matrix;
    DirectX::XMMATRIX m_rotation_matrix;
    DirectX::XMMATRIX m_translation_matrix;

    bool m_need_update;
};
