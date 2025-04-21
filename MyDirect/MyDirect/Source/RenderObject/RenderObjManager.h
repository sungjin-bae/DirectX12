#ifndef MYDIRECT_SOURCE_RENDER_OBJ_MANAGER_H
#define MYDIRECT_SOURCE_RENDER_OBJ_MANAGER_H

#include <memory>
#include <unordered_map>

#include "../Common/Singleton.h"


class RenderObject;
typedef std::shared_ptr<RenderObject> RenderObjSharedPtr;

class RenderObjManager : public Singleton<RenderObjManager>
{
public:
    RenderObjManager();
    virtual ~RenderObjManager();

    void Insert(const RenderObjSharedPtr& in_obj);
    void Delete(long in_obj_id);
    void Clear();

    auto GetRenderObject(long in_obj_id) const -> const RenderObjSharedPtr&;
    auto GetRenderObjects(ERendererType in_type) const -> const std::vector<RenderObjSharedPtr>&;

private:
    std::unordered_map<long, RenderObjSharedPtr> m_repo;
    std::unordered_map<ERendererType, std::vector<RenderObjSharedPtr>> m_repo2;
};


#endif  // MYDIRECT_SOURCE_RENDER_OBJ_MANAGER_H
