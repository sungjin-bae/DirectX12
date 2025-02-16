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
} 


void RenderObjManager::Delete(long in_obj_id)
{
    m_repo.erase(in_obj_id);
}

void RenderObjManager::Clear() 
{
    m_repo.clear();
}