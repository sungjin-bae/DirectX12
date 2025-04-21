#include "RenderObject.h"
#include "RenderObjManager.h"

RenderObjManager::RenderObjManager() {}
RenderObjManager::~RenderObjManager() {}

void RenderObjManager::Insert(const RenderObjSharedPtr& in_obj)
{
    if (in_obj == nullptr)
        return;

    // 기존 값이 있다면 덮어 씌움.
    m_repo[in_obj->GetObjID()] = in_obj;
    m_repo2[in_obj->GetRendererType()].push_back(in_obj);
} 


void RenderObjManager::Delete(long in_obj_id)
{
    m_repo.erase(in_obj_id);
}

void RenderObjManager::Clear() 
{
    m_repo.clear();
}

auto RenderObjManager::GetRenderObject(long in_obj_id) const -> const RenderObjSharedPtr&
{
    auto it = m_repo.find(in_obj_id);
    if (it == m_repo.end())
        return nullptr;

    return it->second;
}

auto RenderObjManager::GetRenderObjects(ERendererType in_type) const -> const std::vector<RenderObjSharedPtr>&
{
    const static std::vector<RenderObjSharedPtr> empty_vec;

    auto it = m_repo2.find(in_type);
    if (it == m_repo2.end())
        return empty_vec;

    return it->second;
}
