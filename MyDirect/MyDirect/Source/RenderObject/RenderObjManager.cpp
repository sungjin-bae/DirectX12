#include "RenderObject.h"
#include "RenderObjManager.h"

RenderObjManager::RenderObjManager() {}
RenderObjManager::~RenderObjManager() {}

bool RenderObjManager::Insert(const std::shared_ptr<RenderObject>& in_obj)
{
    if (in_obj == nullptr)
        return false;

    auto ret = m_repo.insert(std::make_pair(in_obj->GetObjID(), in_obj));
    return ret.second;
} 


void RenderObjManager::Delete(long in_obj_id)
{
    m_repo.erase(in_obj_id);
}

void RenderObjManager::Clear() 
{
    m_repo.clear();
}