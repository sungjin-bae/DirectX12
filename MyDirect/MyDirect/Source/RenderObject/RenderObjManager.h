#ifndef MYDIRECT_SOURCE_RENDER_OBJ_MANAGER_H
#define MYDIRECT_SOURCE_RENDER_OBJ_MANAGER_H

#include <memory>
#include <unordered_map>

#include "../Common/Singleton.h"


class RenderObject;

class RenderObjManager : public Singleton<RenderObjManager>
{
public:
    RenderObjManager();
    ~RenderObjManager();

    void Insert(const RenderObjSharedPtr& in_obj);
    void Delete(long in_obj_id);
    void Clear();

private:
    std::unordered_map<long, RenderObjSharedPtr> m_repo;
};


#endif  // MYDIRECT_SOURCE_RENDER_OBJ_MANAGER_H
